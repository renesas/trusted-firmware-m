/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * PSA crypto configuration adjustments for the Renesas RSIP-E50D accelerator.
 *
 * crypto_config_*.h includes this last under CRYPTO_HW_ACCELERATOR, so an #undef here removes
 * an algorithm or curve from the build. With MBEDTLS_PSA_CRYPTO_CONFIG, the legacy
 * MBEDTLS_ECP_DP_* / MBEDTLS_ECDSA_DETERMINISTIC settings derive from these, so this is the
 * one place to change them.
 *
 * Do not advertise what the engine cannot do correctly. FSP's ECC ALT has no software fallback
 * (mbedtls_accelerator_config.h), so an unsupported setting would build and then fail, or
 * return the wrong kind of result, at run time.
 *
 * E50D also removes two curves that it supports, to save flash; see below.
 */

#ifndef CRYPTO_ACCELERATOR_CONFIG_H
#define CRYPTO_ACCELERATOR_CONFIG_H

/* P-521 and Curve25519: removed for flash, not for capability (DECISIONS D059).
 *
 * E50D has hardware procedures for both: ecp_can_do_sce() returns 1 for
 * MBEDTLS_ECP_DP_SECP521R1 and MBEDTLS_ECP_DP_CURVE25519 under
 * BSP_FEATURE_RSIP_RSIP_E50D_SUPPORTED, which bsp_feature.h defines as 1 for R7KA8M2.
 *
 * Each HW procedure is a large constant instruction table, and these two curves pull in the
 * transitive closure of their hw_sce_p_func###.o members: 22,977 bytes of tfm_s text. With
 * both curves, the HW_SCE_* tables are 127,542 bytes on E50D against 8,124 on SCE9.
 *
 * The engine keeps secp256r1, secp256k1, brainpoolP256r1, secp384r1 and brainpoolP384r1, the
 * curve set the validated RA6M5/RA6E1 ports advertise. Nothing in TF-M itself needs more.
 *
 * TODO: restore both curves after the TF-M 2.3 migration, once the layout has room. Deleting
 * these two #undefs is the whole change. */
#undef PSA_WANT_ECC_SECP_R1_521
#undef PSA_WANT_ECC_MONTGOMERY_255

/* Deterministic ECDSA (RFC 6979). FSP does not support it on the SCE (the FSP configurator
 * does not allow it), so this aligns TF-M's default configuration with FSP. The SCE draws its
 * own nonce (mbedtls_ecdsa_sign() does not use f_rng). With MBEDTLS_ECDSA_SIGN_ALT, Mbed TLS's
 * deterministic path is exactly that call with an HMAC-DRBG as f_rng, so
 * PSA_ALG_DETERMINISTIC_ECDSA would return a valid but randomised signature. Nothing in TF-M
 * needs it: the attestation key signs with PSA_ALG_ECDSA. */
#undef PSA_WANT_ALG_DETERMINISTIC_ECDSA

#endif /* CRYPTO_ACCELERATOR_CONFIG_H */
