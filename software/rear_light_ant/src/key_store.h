/*
 * Persistent Find My key position (ADR-0012).
 *
 * Three flash pages at KEY_STORE_ADDR (config.h, kept out of the image by
 * the linker script):
 *
 *   page 0  provision block, written by tools/haystack/provision_airtag.py
 *           and never by the firmware: master public key P0, start index,
 *           SK at that index
 *   page 1  log of {index, SK}: one record per key rotation, appended
 *   page 2  the same; when one log page is full the other one is erased
 *           and the log continues there
 *
 * The newest valid record of this provision block wins; with none, the
 * provision block itself is the start. Records of an older provision block
 * (re-provisioned light) carry a different tag and are ignored.
 *
 * Hardware-free except for the two flash functions at the bottom, which the
 * caller implements (haystack.c: SoftDevice flash API; host test: RAM).
 * Host-tested (test/test_key_store.c).
 */

#ifndef KEY_STORE_H
#define KEY_STORE_H

#include <stdbool.h>
#include <stdint.h>

#include "findmy_keys.h"

#define KEY_STORE_PAGE_WORDS 1024U /* 4 kB nRF52 flash page */
#define KEY_STORE_PAGES      3U

/* Where the key schedule stands */
struct key_store_pos {
    uint32_t index;                   /* Apple key index (15-minute periods since pairing) */
    uint8_t sk[FINDMY_SK_LEN];        /* SK at that index */
    uint8_t master[FINDMY_MASTER_LEN];/* P0, constant */
};

/* Reads the store at base (KEY_STORE_PAGES pages). Returns false if there
 * is no valid provision block; pos is then unchanged. */
bool key_store_init(const uint32_t *base, struct key_store_pos *pos);

/* Remembers {index, sk} and starts writing it. Only the latest call matters:
 * a save that arrives while flash is busy replaces an older unsaved one. */
void key_store_save(uint32_t index, const uint8_t sk[FINDMY_SK_LEN]);

/* Retries a save that failed or was busy; call from the main loop */
void key_store_poll(void);

/* The flash operation started by key_store_flash_write/erase has ended */
void key_store_on_flash_done(bool ok);

/* --- Implemented by the caller -------------------------------------------
 * Both start an operation and return 0 if it was accepted; completion is
 * reported through key_store_on_flash_done(). src stays valid until then. */
uint32_t key_store_flash_write(const uint32_t *dst, const uint32_t *src, uint32_t words);
uint32_t key_store_flash_erase(const uint32_t *page);

#endif /* KEY_STORE_H */
