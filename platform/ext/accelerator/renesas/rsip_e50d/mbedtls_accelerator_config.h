/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Mbed TLS configuration for the Renesas RSIP-E50D accelerator (secure crypto partition).
 *
 * tfm_mbedcrypto_config_*.h includes this last under CRYPTO_HW_ACCELERATOR, so anything
 * defined here overrides the profile.
 *
 * Model: legacy *_ALT. FSP's rm_psa_crypto ships RSIP-E50D forks of Mbed TLS library modules
 * (aes_alt.c for aes.c, gcm_alt.c for gcm.c, ...), compiled against FSP's own Mbed TLS. The
 * port builds the crypto library from FSP's tree rather than upstream, because the ALT sources
 * depend on FSP's PSA core (ra8m2/cmake/fsp_mbedtls.cmake). Mbed TLS 3.6 only: TF-M 2.3 moves
 * to TF-PSA-Crypto, which has no *_ALT mechanism, and this becomes a PSA transparent driver
 * (PROJECT_PLAN.md P7, DECISIONS D035).
 *
 * Scope: plaintext keys only: *_fsp_cfg.h sets every PSA_CRYPTO_CFG_*_FORMAT to plaintext, and
 * FSP's wrapped-key "vendor" driver (vendor.c, aes_vendor.c) is not built here.
 */

#ifndef MBEDTLS_ACCELERATOR_CONFIG_H
#define MBEDTLS_ACCELERATOR_CONFIG_H

/* FSP's rm_psa_crypto configuration (plaintext keys only), bsp_api.h and BYTES_TO_WORDS -
 * shared with BL2. */
#include "rsip_e50d_fsp_cfg.h"

/* ----------------------------------------------------------------------------------------
 * Accelerated modules: FSP's E50D set, held to the SCE9 module list for bring-up.
 *
 * The E50D pack also ships sha512_alt, sha3_alt, chacha20_alt, chachapoly_alt, mlkem_alt and
 * mldsa_alt, which SCE9 lacks. All are off, as in the e2 solution's own configuration, so the
 * first RA8M2 bring-up differs from the validated RA6M5 one by the engine alone. Enabling one
 * means adding its MBEDTLS_*_ALT here and its source pair in the CMakeLists (DECISIONS D051).
 *
 * MBEDTLS_CIPHER_ALT is part of the set, not an extra: the AES ALT is written against the
 * block chunking and session close in FSP's cipher_alt.c (see CMakeLists.txt, E50D_ALT_SOURCES).
 * -------------------------------------------------------------------------------------- */
#define MBEDTLS_CIPHER_ALT

#define MBEDTLS_AES_ALT
#define MBEDTLS_AES_SETKEY_ENC_ALT
#define MBEDTLS_AES_SETKEY_DEC_ALT
#define MBEDTLS_AES_ENCRYPT_ALT
#define MBEDTLS_AES_DECRYPT_ALT

#define MBEDTLS_GCM_ALT
#define MBEDTLS_CMAC_ALT

/* MBEDTLS_CCM_ALT is deliberately not defined (DECISIONS D043, D051).
 *
 * NOTE: this diverges from FSP, whose E50D configuration enables MBEDTLS_CCM_ALT. FSP's SCE9
 * CCM accepts at most 110 B of associated data, which Protected Storage exceeds; see
 * sce9/mbedtls_accelerator_config.h, at MBEDTLS_CCM_ALT. Whether E50D formats B-blocks into
 * the same 128 B buffer is unverified, so the ALT stays off until the limit is measured on
 * this engine.
 *
 * CCM therefore stays in software, which still reaches the E50D engine per block through
 * MBEDTLS_CIPHER_ALT/MBEDTLS_AES_ALT. */

#define MBEDTLS_SHA256_ALT
#define MBEDTLS_SHA256_PROCESS_ALT

#define MBEDTLS_RSA_ALT

/* ECC. FSP's ECDSA ALT reads its patched group struct (grp->vendor_ctx), so it needs
 * MBEDTLS_ECP_ALT; ecp_alt.c and ecp_curves_alt.c replace Mbed TLS's ECP module whole.
 *
 * Curves: E50D signs and verifies secp256r1, secp256k1, brainpoolP256r1, secp384r1,
 * brainpoolP384r1 and, unlike SCE9, secp521r1 and Curve25519. ecp_can_do_sce() gates the last
 * two on BSP_FEATURE_RSIP_RSIP_E50D_SUPPORTED, which is 1 on this part.
 *
 * The port advertises only the first five: crypto_accelerator_config.h removes P-521 and
 * Curve25519 for flash, as the SCE9 configuration does, and gives the cost and the plan to
 * restore them (DECISIONS D059). The engine can do P-521; the port is not configured to.
 *
 * There is no software fallback: any curve outside the engine's set returns
 * MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE. Curve448 stays: ECP scalar multiplication falls back to
 * software (ecp_mul_mxz), and nothing references hardware for it. Deterministic ECDSA: see
 * crypto_accelerator_config.h. */
#define MBEDTLS_ECP_ALT
#define MBEDTLS_ECDSA_SIGN_ALT
#define MBEDTLS_ECDSA_VERIFY_ALT

/* Deliberately not enabled, although FSP's own config sets them:
 *   MBEDTLS_ENTROPY_HARDWARE_ALT, MBEDTLS_CTR_DRBG_C_ALT - entropy is already hardware, via
 *       MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG and the platform's sce_trng.c.
 *   MBEDTLS_PLATFORM_SETUP_TEARDOWN_ALT - nothing in TF-M calls mbedtls_platform_setup();
 *       crypto_hw_accelerator_init() brings the engine up instead.
 *   MBEDTLS_PSA_CRYPTO_ACCEL_DRV_C - the wrapped-key vendor driver; plaintext keys only, so a
 *       wrapped-key request panics in crypto_hw.c. */

#endif /* MBEDTLS_ACCELERATOR_CONFIG_H */
