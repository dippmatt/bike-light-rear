/*
 * Host test of the persistent Find My key position (ADR-0012), on a RAM
 * flash that behaves like NOR flash (writes only clear bits).
 * Build and run with: make test
 */

#include <stdio.h>
#include <string.h>

#include "../src/key_store.h"

static int failures;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);           \
            failures++;                                                      \
        }                                                                    \
    } while (0)

#define PAGE  KEY_STORE_PAGE_WORDS
#define SLOTS (PAGE / 12U) /* 48-byte records */

static uint32_t flash[KEY_STORE_PAGES * PAGE];
static int writes;
static int erases;
static bool refuse; /* next operation is not accepted (SoftDevice busy) */

uint32_t key_store_flash_write(const uint32_t *dst, const uint32_t *src, uint32_t words)
{
    uint32_t *d = &flash[dst - flash];
    uint32_t i;

    if (refuse) {
        refuse = false;
        return 1U;
    }
    for (i = 0U; i < words; i++) {
        d[i] &= src[i];
    }
    writes++;
    return 0U;
}

uint32_t key_store_flash_erase(const uint32_t *page)
{
    if (refuse) {
        refuse = false;
        return 1U;
    }
    memset(&flash[page - flash], 0xFF, PAGE * 4U);
    erases++;
    return 0U;
}

static uint32_t crc32(const uint8_t *p, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFU;

    while (len-- > 0U) {
        crc ^= *p++;
        for (int b = 0; b < 8; b++) {
            crc = (crc >> 1) ^ (0xEDB88320U & (0U - (crc & 1U)));
        }
    }
    return ~crc;
}

/* Same layout as provision_airtag.py: magic, index, master[56], sk[32], sks[32], crc */
static void provision(uint32_t index, uint8_t fill)
{
    uint8_t b[132];

    memset(flash, 0xFF, sizeof(flash));
    memset(b, fill, sizeof(b));
    b[0] = 'F';
    b[1] = 'M';
    b[2] = 'P';
    b[3] = '2';
    memcpy(&b[4], &index, 4);
    memset(&b[96], fill ^ 0xFF, 32); /* sks: tell it apart from sk */
    uint32_t crc = crc32(b, 128U);
    memcpy(&b[128], &crc, 4);
    memcpy(flash, b, sizeof(b));
}

static void sk_for(uint32_t index, uint8_t sk[FINDMY_SK_LEN])
{
    memset(sk, (int)(index & 0xFFU), FINDMY_SK_LEN);
    sk[0] = (uint8_t)(index >> 8);
}

/* Reports success for every flash operation the store starts, until it stops */
static void complete_all(void)
{
    for (;;) {
        int w = writes;
        int e = erases;

        key_store_on_flash_done(true);
        if (writes == w && erases == e) {
            return;
        }
    }
}

static void save_ok(uint32_t index)
{
    uint8_t sk[FINDMY_SK_LEN];

    sk_for(index, sk);
    key_store_save(index, sk);
    complete_all();
}

int main(void)
{
    struct key_store_pos pos;
    uint8_t sk[FINDMY_SK_LEN];
    uint32_t i;

    /* The test's own CRC is the standard one */
    CHECK(crc32((const uint8_t *)"123456789", 9U) == 0xCBF43926U);

    /* Blank flash: no provision, nothing to send */
    memset(flash, 0xFF, sizeof(flash));
    pos.index = 7U;
    CHECK(!key_store_init(flash, &pos));
    CHECK(pos.index == 7U);

    /* Provision only: start there */
    provision(1000U, 0x11U);
    CHECK(key_store_init(flash, &pos));
    CHECK(pos.index == 1000U && pos.sk[0] == 0x11U && pos.master[55] == 0x11U);
    CHECK(pos.sks_index == 1000U / 96U + 1U && pos.sks[0] == 0xEEU && pos.sks[31] == 0xEEU);

    /* One rotation survives a reset */
    save_ok(1001U);
    CHECK(writes == 1);
    CHECK(key_store_init(flash, &pos));
    sk_for(1001U, sk);
    CHECK(pos.index == 1001U && memcmp(pos.sk, sk, sizeof(sk)) == 0);
    CHECK(pos.master[0] == 0x11U); /* master comes from the provision block */
    CHECK(pos.sks_index == 11U && pos.sks[0] == 0xEEU); /* SKS too: not logged */

    /* Many rotations: both log pages wrap at least once */
    erases = 0;
    for (i = 1002U; i < 1002U + 3U * SLOTS; i++) {
        save_ok(i);
    }
    CHECK(erases >= 2);
    CHECK(key_store_init(flash, &pos));
    CHECK(pos.index == 1001U + 3U * SLOTS);

    /* Save while busy: only the newest one is written next */
    provision(50U, 0x22U);
    CHECK(key_store_init(flash, &pos));
    writes = 0;
    sk_for(51U, sk);
    key_store_save(51U, sk);
    sk_for(52U, sk);
    key_store_save(52U, sk); /* first still in flight */
    CHECK(writes == 1);
    key_store_on_flash_done(true);
    CHECK(writes == 2);
    key_store_on_flash_done(true);
    CHECK(key_store_init(flash, &pos));
    CHECK(pos.index == 52U);

    /* Refused, then failed: poll retries, never into the same slot twice */
    refuse = true;
    sk_for(53U, sk);
    key_store_save(53U, sk);
    CHECK(writes == 2);
    key_store_poll();
    CHECK(writes == 3);
    key_store_on_flash_done(false);
    key_store_poll();
    CHECK(writes == 4);
    key_store_on_flash_done(true);
    CHECK(key_store_init(flash, &pos));
    CHECK(pos.index == 53U);

    /* Re-provisioned light: old records (higher index, other tag) ignored */
    save_ok(54U);
    {
        uint32_t keep[2U * PAGE];

        memcpy(keep, &flash[PAGE], sizeof(keep));
        provision(20U, 0x33U);
        memcpy(&flash[PAGE], keep, sizeof(keep));
    }
    CHECK(key_store_init(flash, &pos));
    CHECK(pos.index == 20U && pos.sk[0] == 0x33U);

    /* A torn record (bad crc) is skipped, the one before it wins */
    provision(5U, 0x44U);
    CHECK(key_store_init(flash, &pos));
    save_ok(6U);
    save_ok(7U);
    flash[PAGE + 12U + 3U] ^= 0x1U; /* slot 1 = index 7: corrupt its sk */
    CHECK(key_store_init(flash, &pos));
    CHECK(pos.index == 6U);
    save_ok(7U); /* continues in slot 2, not on top of the torn slot 1 */
    CHECK(key_store_init(flash, &pos));
    CHECK(pos.index == 7U);

    if (failures != 0) {
        printf("test_key_store: %d failure(s)\n", failures);
        return 1;
    }
    printf("test_key_store: all passed\n");
    return 0;
}
