/*
 * Find My advertisement payload, see haystack_adv.h.
 *
 * Layout (same as seemoo-lab/openhaystack offline finding firmware):
 *   address  key[0] | 0xC0, key[1..5]           (static random, MSB first)
 *   payload  1E FF 4C 00   manufacturer data, Apple
 *            12 19         offline finding, 25 bytes follow
 *            00            status
 *            key[6..27]    22 bytes
 *            key[0] >> 6   the two bits the address had to give up
 *            00            hint
 */

#include "haystack_adv.h"

void haystack_adv_build(const uint8_t key[HAYSTACK_KEY_LEN],
                        uint8_t addr[HAYSTACK_ADDR_LEN],
                        uint8_t payload[HAYSTACK_PAYLOAD_LEN])
{
    uint32_t i;

    addr[5] = (uint8_t)(key[0] | 0xC0U);
    for (i = 1U; i < HAYSTACK_ADDR_LEN; i++) {
        addr[5U - i] = key[i];
    }

    payload[0] = 0x1EU;
    payload[1] = 0xFFU;
    payload[2] = 0x4CU;
    payload[3] = 0x00U;
    payload[4] = 0x12U;
    payload[5] = 0x19U;
    payload[6] = 0x00U;
    for (i = 6U; i < HAYSTACK_KEY_LEN; i++) {
        payload[1U + i] = key[i];
    }
    payload[29] = (uint8_t)(key[0] >> 6);
    payload[30] = 0x00U;
}
