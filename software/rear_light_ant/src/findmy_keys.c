/*
 * Find My accessory key schedule, see findmy_keys.h.
 *
 * Mirrors FindMy.py (findmy/util/crypto.py: x963_kdf, derive_ps_key;
 * findmy/accessory.py: _AccessoryKeyGenerator), but on the public side:
 * FindMy.py derives the private key d_i = u * d0 + v, we derive its public
 * key P_i = u * P0 + v * G directly.
 */

#include "findmy_keys.h"

#include <tinycrypt/sha256.h>

#include "ext/micro-ecc/uECC_vli.h"

/* 224 bits: 7 words of 32 bits (target) or 4 of 64 bits (host test). One
 * point is two coordinates; uECC_vli_mmod wants a double-width input. */
#define MAX_WORDS 8U

#define DIVERSIFY_HALF 36U /* bytes per scalar in the 72-byte "diversify" output */

static void store_be32(uint8_t out[4], uint32_t v)
{
    out[0] = (uint8_t)(v >> 24);
    out[1] = (uint8_t)(v >> 16);
    out[2] = (uint8_t)(v >> 8);
    out[3] = (uint8_t)v;
}

/* ANSI X9.63 KDF with SHA-256: out = H(z || 1 || info) || H(z || 2 || info) ... */
static void x963_kdf(const uint8_t *z, uint32_t z_len, const char *info, uint32_t info_len,
                     uint8_t *out, uint32_t out_len)
{
    struct tc_sha256_state_struct s;
    uint8_t block[TC_SHA256_DIGEST_SIZE];
    uint8_t counter_be[4];
    uint32_t counter = 1U;
    uint32_t i;

    while (out_len > 0U) {
        uint32_t n = (out_len < TC_SHA256_DIGEST_SIZE) ? out_len : TC_SHA256_DIGEST_SIZE;

        store_be32(counter_be, counter);
        (void)tc_sha256_init(&s);
        (void)tc_sha256_update(&s, z, z_len);
        (void)tc_sha256_update(&s, counter_be, sizeof(counter_be));
        (void)tc_sha256_update(&s, (const uint8_t *)info, info_len);
        (void)tc_sha256_final(block, &s);
        for (i = 0U; i < n; i++) {
            out[i] = block[i];
        }
        out += n;
        out_len -= n;
        counter++;
    }
}

void findmy_next_sk(const uint8_t sk[FINDMY_SK_LEN], uint8_t next[FINDMY_SK_LEN])
{
    uint8_t tmp[FINDMY_SK_LEN];
    uint32_t i;

    x963_kdf(sk, FINDMY_SK_LEN, "update", 6U, tmp, FINDMY_SK_LEN);
    for (i = 0U; i < FINDMY_SK_LEN; i++) {
        next[i] = tmp[i];
    }
}

/* scalar = 1 + (bytes mod (n - 1)), as in derive_ps_key() */
static void to_scalar(const uint8_t bytes[DIVERSIFY_HALF], uECC_word_t scalar[MAX_WORDS],
                      uECC_Curve curve)
{
    const wordcount_t nw = (wordcount_t)uECC_curve_num_n_words(curve);
    uECC_word_t wide[2U * MAX_WORDS];
    uECC_word_t n_minus_1[MAX_WORDS];
    uECC_word_t one[MAX_WORDS];

    uECC_vli_clear(wide, (wordcount_t)(2 * nw));
    uECC_vli_bytesToNative(wide, bytes, (int)DIVERSIFY_HALF);

    uECC_vli_clear(one, nw);
    one[0] = 1U;
    (void)uECC_vli_sub(n_minus_1, uECC_curve_n(curve), one, nw);

    uECC_vli_mmod(scalar, wide, n_minus_1, nw);
    (void)uECC_vli_add(scalar, scalar, one, nw); /* < n, cannot carry */
}

/* x(a + b) for affine points a != +-b */
static bool add_x(const uECC_word_t *a, const uECC_word_t *b, uECC_word_t *x, uECC_Curve curve)
{
    const wordcount_t nw = (wordcount_t)uECC_curve_num_words(curve);
    const uECC_word_t *p = uECC_curve_p(curve);
    uECC_word_t dx[MAX_WORDS];
    uECC_word_t dy[MAX_WORDS];
    uECC_word_t lambda[MAX_WORDS];

    if (uECC_vli_equal(a, b, nw) != 0U) {
        return false; /* doubling or point at infinity: not worth handling */
    }
    uECC_vli_modSub(dx, b, a, p, nw);
    uECC_vli_modInv(dx, dx, p, nw);
    uECC_vli_modSub(dy, b + nw, a + nw, p, nw);
    uECC_vli_modMult(lambda, dy, dx, p, nw);

    uECC_vli_modMult(x, lambda, lambda, p, nw);
    uECC_vli_modSub(x, x, a, p, nw);
    uECC_vli_modSub(x, x, b, p, nw);
    return true;
}

bool findmy_adv_key(const uint8_t master[FINDMY_MASTER_LEN], const uint8_t sk[FINDMY_SK_LEN],
                    uint8_t adv_key[FINDMY_ADV_KEY_LEN])
{
    const uECC_Curve curve = uECC_secp224r1();
    const wordcount_t nw = (wordcount_t)uECC_curve_num_words(curve);
    const int nb = (int)uECC_curve_num_bytes(curve);
    uint8_t uv[2U * DIVERSIFY_HALF];
    uECC_word_t u[MAX_WORDS];
    uECC_word_t v[MAX_WORDS];
    uECC_word_t p0[2U * MAX_WORDS];
    uECC_word_t up0[2U * MAX_WORDS];
    uECC_word_t vg[2U * MAX_WORDS];
    uECC_word_t x[MAX_WORDS];

    uECC_vli_bytesToNative(p0, master, nb);
    uECC_vli_bytesToNative(p0 + nw, master + nb, nb);
    if (uECC_valid_point(p0, curve) == 0) {
        return false;
    }

    x963_kdf(sk, FINDMY_SK_LEN, "diversify", 9U, uv, sizeof(uv));
    to_scalar(&uv[0], u, curve);
    to_scalar(&uv[DIVERSIFY_HALF], v, curve);

    uECC_point_mult(up0, p0, u, curve);
    uECC_point_mult(vg, uECC_curve_G(curve), v, curve);
    if (!add_x(up0, vg, x, curve)) {
        return false;
    }
    uECC_vli_nativeToBytes(adv_key, nb, x);
    return true;
}
