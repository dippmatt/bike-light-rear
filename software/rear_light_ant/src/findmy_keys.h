/*
 * Find My accessory key schedule (ADR-0012), the one a real AirTag runs.
 *
 * Hardware-free, host-tested (test/test_findmy_keys.c) against FindMy.py.
 *
 *   SK_0     = SKN (shared secret from pairing)
 *   SK_i     = X9.63-KDF-SHA256(SK_{i-1}, "update", 32)
 *   u, v     = X9.63-KDF-SHA256(SK_i, "diversify", 72), 36 bytes each,
 *              each reduced to 1 + (x mod (n - 1))
 *   P_i      = u * P0 + v * G           (NIST P-224)
 *   advert   = x(P_i), 28 bytes big-endian
 *
 * P0 is the PUBLIC master key, so the light never holds a private key: a
 * dumped flash lets someone predict the light's adverts but never decrypt
 * the location reports.
 */

#ifndef FINDMY_KEYS_H
#define FINDMY_KEYS_H

#include <stdbool.h>
#include <stdint.h>

#define FINDMY_SK_LEN      32U /* SK_i */
#define FINDMY_MASTER_LEN  56U /* P0 uncompressed, x || y, big-endian */
#define FINDMY_ADV_KEY_LEN 28U /* x(P_i), what haystack_adv_build() takes */

/* next = SK_{i+1} from sk = SK_i. next and sk may be the same buffer. */
void findmy_next_sk(const uint8_t sk[FINDMY_SK_LEN], uint8_t next[FINDMY_SK_LEN]);

/* adv_key = x(P_i) for sk = SK_i. Returns false if master is not a point on
 * the curve (or for the negligible case u*P0 = +-v*G); adv_key is then
 * unchanged. Takes two P-224 point multiplications: call from the main loop,
 * never from the radio timeslot callback. */
bool findmy_adv_key(const uint8_t master[FINDMY_MASTER_LEN], const uint8_t sk[FINDMY_SK_LEN],
                    uint8_t adv_key[FINDMY_ADV_KEY_LEN]);

#endif /* FINDMY_KEYS_H */
