/*
 * Find My ("OpenHaystack") advertisement of the light, ADR-0011.
 *
 * Hardware-free: turns the 28-byte advertisement key into the BLE
 * advertising address and the 31-byte payload that nearby Apple devices
 * report to the Find My network. Host-tested (test/test_haystack_adv.c).
 */

#ifndef HAYSTACK_ADV_H
#define HAYSTACK_ADV_H

#include <stdint.h>

#define HAYSTACK_KEY_LEN     28U /* x coordinate of the P-224 public key */
#define HAYSTACK_ADDR_LEN    6U
#define HAYSTACK_PAYLOAD_LEN 31U

/* addr: static random address in over-the-air order (least significant byte
 * first). payload: advertising data for ADV_NONCONN_IND. */
void haystack_adv_build(const uint8_t key[HAYSTACK_KEY_LEN],
                        uint8_t addr[HAYSTACK_ADDR_LEN],
                        uint8_t payload[HAYSTACK_PAYLOAD_LEN]);

#endif /* HAYSTACK_ADV_H */
