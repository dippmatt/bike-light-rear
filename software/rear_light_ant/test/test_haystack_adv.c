/*
 * Host test of the Find My advertisement payload (ADR-0011).
 * Build and run with: make test
 */

#include <stdio.h>

#include "../src/haystack_adv.h"

static int failures;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);           \
            failures++;                                                      \
        }                                                                    \
    } while (0)

int main(void)
{
    uint8_t key[HAYSTACK_KEY_LEN];
    uint8_t addr[HAYSTACK_ADDR_LEN];
    uint8_t payload[HAYSTACK_PAYLOAD_LEN];
    uint32_t i;

    /* key[0] = 0xAB (top bits 10), key[i] = i otherwise */
    key[0] = 0xABU;
    for (i = 1U; i < HAYSTACK_KEY_LEN; i++) {
        key[i] = (uint8_t)i;
    }
    haystack_adv_build(key, addr, payload);

    /* Over-the-air order: key[5] first, key[0] | 0xC0 last */
    const uint8_t want_addr[HAYSTACK_ADDR_LEN] = { 5, 4, 3, 2, 1, 0xEB };
    for (i = 0U; i < HAYSTACK_ADDR_LEN; i++) {
        CHECK(addr[i] == want_addr[i]);
    }

    const uint8_t want_head[7] = { 0x1E, 0xFF, 0x4C, 0x00, 0x12, 0x19, 0x00 };
    for (i = 0U; i < 7U; i++) {
        CHECK(payload[i] == want_head[i]);
    }
    for (i = 6U; i < HAYSTACK_KEY_LEN; i++) {
        CHECK(payload[1U + i] == (uint8_t)i); /* key[6..27] at payload[7..28] */
    }
    CHECK(payload[29] == 2U); /* 0xAB >> 6 */
    CHECK(payload[30] == 0U);

    /* The length byte covers everything after it */
    CHECK(payload[0] == HAYSTACK_PAYLOAD_LEN - 1U);

    /* The address always has the two top bits set (static random address) */
    key[0] = 0x00U;
    haystack_adv_build(key, addr, payload);
    CHECK((addr[5] & 0xC0U) == 0xC0U);
    CHECK(payload[29] == 0U);
    key[0] = 0xFFU;
    haystack_adv_build(key, addr, payload);
    CHECK(addr[5] == 0xFFU);
    CHECK(payload[29] == 3U);

    if (failures == 0) {
        printf("test_haystack_adv: all passed\n");
    }
    return failures == 0 ? 0 : 1;
}
