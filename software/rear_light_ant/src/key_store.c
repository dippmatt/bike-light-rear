/*
 * Persistent Find My key position, see key_store.h.
 *
 * Little-endian words, CRC-32 (IEEE, as Python's zlib.crc32) over all bytes
 * before the crc field. tools/haystack/provision_airtag.py writes the same
 * provision layout.
 */

#include "key_store.h"

#define PROVISION_MAGIC 0x31504D46UL /* "FMP1" */
#define RECORD_MAGIC    0x31524D46UL /* "FMR1" */

struct provision {
    uint32_t magic;
    uint32_t index;
    uint8_t master[FINDMY_MASTER_LEN];
    uint8_t sk[FINDMY_SK_LEN];
    uint32_t crc;
};

struct record {
    uint32_t magic;
    uint32_t index;
    uint8_t sk[FINDMY_SK_LEN];
    uint32_t tag; /* crc of the provision block this record continues */
    uint32_t crc;
};

_Static_assert(sizeof(struct provision) == 100U, "provision layout is shared with provision_airtag.py");
_Static_assert(sizeof(struct record) % 4U == 0U, "records are written as words");

#define RECORD_WORDS  (sizeof(struct record) / 4U)
#define SLOTS_PER_PAGE (KEY_STORE_PAGE_WORDS / RECORD_WORDS)

enum op { OP_IDLE, OP_ERASING, OP_WRITING };

static const uint32_t *store;       /* page 0 provision, pages 1 and 2 log */
static uint32_t tag;                /* crc of the provision block */
static uint32_t log_page;           /* 0 or 1: log page written to */
static uint32_t next_slot;          /* SLOTS_PER_PAGE: page full */
static enum op op;
static bool pending;
static struct record pending_rec;
static struct record inflight;      /* stays untouched until the flash operation ends */

static uint32_t crc32(const uint8_t *p, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFUL;
    uint32_t i;
    uint32_t bit;

    for (i = 0U; i < len; i++) {
        crc ^= p[i];
        for (bit = 0U; bit < 8U; bit++) {
            crc = (crc >> 1) ^ (0xEDB88320UL & (0UL - (crc & 1UL)));
        }
    }
    return ~crc;
}

static void copy_bytes(void *dst, const void *src, uint32_t len)
{
    uint8_t *d = dst;
    const uint8_t *s = src;
    uint32_t i;

    for (i = 0U; i < len; i++) {
        d[i] = s[i];
    }
}

static const uint32_t *log_base(uint32_t page)
{
    return store + (1U + page) * KEY_STORE_PAGE_WORDS;
}

static const uint32_t *slot_addr(uint32_t page, uint32_t slot)
{
    return log_base(page) + slot * RECORD_WORDS;
}

static bool slot_erased(const uint32_t *w)
{
    uint32_t i;

    for (i = 0U; i < RECORD_WORDS; i++) {
        if (w[i] != 0xFFFFFFFFUL) {
            return false;
        }
    }
    return true;
}

static uint32_t record_crc(const struct record *r)
{
    return crc32((const uint8_t *)r, (uint32_t)(sizeof(*r) - sizeof(r->crc)));
}

/* First erased slot at or after from, SLOTS_PER_PAGE if none */
static uint32_t first_free(uint32_t page, uint32_t from)
{
    uint32_t s;

    for (s = from; s < SLOTS_PER_PAGE; s++) {
        if (slot_erased(slot_addr(page, s))) {
            return s;
        }
    }
    return SLOTS_PER_PAGE;
}

bool key_store_init(const uint32_t *base, struct key_store_pos *pos)
{
    struct provision prov;
    struct record rec;
    bool found = false;
    uint32_t best_page = 0U;
    uint32_t best_slot = 0U;
    uint32_t page;
    uint32_t slot;

    store = base;
    op = OP_IDLE;
    pending = false;

    copy_bytes(&prov, base, sizeof(prov));
    if (prov.magic != PROVISION_MAGIC ||
        prov.crc != crc32((const uint8_t *)&prov, (uint32_t)(sizeof(prov) - sizeof(prov.crc)))) {
        store = 0;
        return false;
    }
    tag = prov.crc;
    pos->index = prov.index;
    copy_bytes(pos->sk, prov.sk, FINDMY_SK_LEN);
    copy_bytes(pos->master, prov.master, FINDMY_MASTER_LEN);

    for (page = 0U; page < 2U; page++) {
        for (slot = 0U; slot < SLOTS_PER_PAGE; slot++) {
            copy_bytes(&rec, slot_addr(page, slot), sizeof(rec));
            if (rec.magic != RECORD_MAGIC || rec.tag != tag || rec.crc != record_crc(&rec)) {
                continue; /* erased, torn or from an older provision */
            }
            if (rec.index > pos->index || (!found && rec.index == pos->index)) {
                found = true;
                best_page = page;
                best_slot = slot;
                pos->index = rec.index;
                copy_bytes(pos->sk, rec.sk, FINDMY_SK_LEN);
            }
        }
    }

    /* Continue after the newest record, in its page */
    log_page = best_page;
    next_slot = first_free(best_page, found ? best_slot + 1U : 0U);
    return true;
}

static void kick(void)
{
    if (store == 0 || op != OP_IDLE || !pending) {
        return;
    }
    if (next_slot >= SLOTS_PER_PAGE) {
        /* Page full: the other page holds only older records */
        if (key_store_flash_erase(log_base(1U - log_page)) == 0U) {
            op = OP_ERASING;
        }
        return;
    }
    inflight = pending_rec;
    if (key_store_flash_write(slot_addr(log_page, next_slot), (const uint32_t *)&inflight,
                              (uint32_t)RECORD_WORDS) == 0U) {
        op = OP_WRITING;
    }
}

void key_store_save(uint32_t index, const uint8_t sk[FINDMY_SK_LEN])
{
    pending_rec.magic = RECORD_MAGIC;
    pending_rec.index = index;
    copy_bytes(pending_rec.sk, sk, FINDMY_SK_LEN);
    pending_rec.tag = tag;
    pending_rec.crc = record_crc(&pending_rec);
    pending = true;
    kick();
}

void key_store_poll(void)
{
    kick();
}

void key_store_on_flash_done(bool ok)
{
    switch (op) {
    case OP_ERASING:
        if (ok) {
            log_page = 1U - log_page;
            next_slot = 0U;
        }
        break;
    case OP_WRITING:
        /* Even a failed write may have touched the slot: never reuse it */
        next_slot++;
        if (ok && pending_rec.index == inflight.index) {
            pending = false;
        }
        break;
    default:
        break;
    }
    op = OP_IDLE;
    if (ok) {
        kick(); /* a newer save may be waiting, or the write after an erase */
    }
}
