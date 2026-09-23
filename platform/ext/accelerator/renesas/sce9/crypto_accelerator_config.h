/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * PSA crypto configuration adjustments for the Renesas SCE9 accelerator.
 *
 * crypto_config_*.h includes this LAST under CRYPTO_HW_ACCELERATOR, so an #undef here
 * removes an algorithm or curve from the build. With MBEDTLS_PSA_CRYPTO_CONFIG the legacy
 * MBEDTLS_ECP_DP_* / MBEDTLS_ECDSA_DETERMINISTIC settings are derived from these, so this is
 * the one place to change them.
 *
 * The rule: do not advertise what the engine cannot do correctly. FSP's ECC ALT has no
 * software fallback (mbedtls_accelerator_config.h), so anything below would otherwise build
 * and then fail - or, worse, return a wrong kind of result - at run time.
 */

#ifndef CRYPTO_ACCELERATOR_CONFIG_H
#define CRYPTO_ACCELERATOR_CONFIG_H

/* P-521: SCE9 has no procedure for it (RSIP-E51A/E50D only). */
#undef PSA_WANT_ECC_SECP_R1_521

/* Curve25519 (X25519): no SCE9 procedure; its HW table references RSIP-only functions. */
#undef PSA_WANT_ECC_MONTGOMERY_255

/* Deterministic ECDSA (RFC 6979). Not supported by FSP on the SCE - the FSP configurator does
 * not allow it - so this only brings TF-M's default configuration in line with FSP. The SCE
 * draws its own nonce (mbedtls_ecdsa_sign() does not use f_rng), and with
 * MBEDTLS_ECDSA_SIGN_ALT Mbed TLS's deterministic path is exactly that call with an HMAC-DRBG
 * as f_rng, so leaving it enabled would return a valid but randomised signature for
 * PSA_ALG_DETERMINISTIC_ECDSA. Nothing in TF-M needs it: the attestation key signs with
 * PSA_ALG_ECDSA. */
#undef PSA_WANT_ALG_DETERMINISTIC_ECDSA

#endif /* CRYPTO_ACCELERATOR_CONFIG_H */
