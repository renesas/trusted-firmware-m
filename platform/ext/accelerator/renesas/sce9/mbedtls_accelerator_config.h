/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Mbed TLS configuration for the Renesas SCE9 accelerator (secure crypto partition).
 *
 * tfm_mbedcrypto_config_*.h includes this last under CRYPTO_HW_ACCELERATOR, so anything
 * defined here overrides the profile.
 *
 * Model: legacy *_ALT. FSP's rm_psa_crypto ships SCE9 forks of Mbed TLS library modules
 * (aes_alt.c for aes.c, gcm_alt.c for gcm.c, ...), compiled against FSP's own Mbed TLS. The
 * port builds the crypto library from FSP's tree rather than upstream, because the ALT sources
 * depend on FSP's PSA core (ra6m5/cmake/fsp_mbedtls.cmake). Mbed TLS 3.6 only: TF-M 2.3 moves
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
#include "sce9_fsp_cfg.h"

/* ----------------------------------------------------------------------------------------
 * Accelerated modules: FSP's SCE9 set.
 *
 * MBEDTLS_CIPHER_ALT is part of the set, not an extra: the AES ALT is written against the
 * block chunking and session close in FSP's cipher_alt.c (see CMakeLists.txt, SCE9_ALT_SOURCES).
 * -------------------------------------------------------------------------------------- */
#define MBEDTLS_CIPHER_ALT

#define MBEDTLS_AES_ALT
#define MBEDTLS_AES_SETKEY_ENC_ALT
#define MBEDTLS_AES_SETKEY_DEC_ALT
#define MBEDTLS_AES_ENCRYPT_ALT
#define MBEDTLS_AES_DECRYPT_ALT

#define MBEDTLS_GCM_ALT
#define MBEDTLS_CMAC_ALT

/* MBEDTLS_CCM_ALT is deliberately not defined (DECISIONS D043).
 *
 * FSP's SCE9 CCM formats the whole B-block sequence into one 128 B hardware buffer
 * (HW_SCE_AES_CCM_B_FORMAT_BYTE_SIZE), so it accepts at most 110 B of associated data:
 * 16 (B0) + roundup16(2 + aad_len) <= 128. Beyond that it returns MBEDTLS_ERR_CCM_BAD_INPUT,
 * which the PSA layer reports as PSA_ERROR_INVALID_ARGUMENT.
 *
 * Protected Storage authenticates its object table with AES-CCM, using the table itself
 * (about 140 B at PS_NUM_ASSETS 10) as associated data. With the CCM ALT, every PS object-table
 * write fails and the partition does not initialise. CCM therefore stays in software, which
 * still reaches the SCE9 per block through MBEDTLS_CIPHER_ALT/MBEDTLS_AES_ALT. */

#define MBEDTLS_SHA256_ALT
#define MBEDTLS_SHA256_PROCESS_ALT

#define MBEDTLS_RSA_ALT

/* ECC. FSP's ECDSA ALT reads its patched group struct (grp->vendor_ctx), so it needs
 * MBEDTLS_ECP_ALT; ecp_alt.c and ecp_curves_alt.c replace Mbed TLS's ECP module whole.
 *
 * Curves: SCE9 signs and verifies secp256r1, secp256k1, brainpoolP256r1, secp384r1 and
 * brainpoolP384r1 (ecp_can_do_sce() in ecdsa_alt.c). There is no software fallback: any other
 * curve returns MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE. P-521 and Curve25519 have no SCE9
 * procedures (their HW tables do not compile), so crypto_accelerator_config.h removes them
 * from the PSA configuration. Curve448 stays: ECP scalar multiplication falls back to software
 * (ecp_mul_mxz), and nothing references hardware for it. Deterministic ECDSA: see
 * crypto_accelerator_config.h. */
#define MBEDTLS_ECP_ALT
#define MBEDTLS_ECDSA_SIGN_ALT
#define MBEDTLS_ECDSA_VERIFY_ALT

/* Deliberately not enabled, although FSP's own config sets them:
 *   MBEDTLS_ENTROPY_HARDWARE_ALT, MBEDTLS_CTR_DRBG_C_ALT - entropy is already hardware, via
 *       MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG and the platform's sce_trng.c.
 *   MBEDTLS_PLATFORM_SETUP_TEARDOWN_ALT - nothing in TF-M calls mbedtls_platform_setup();
 *       crypto_hw_accelerator_init() brings the engine up instead.
 *   MBEDTLS_PSA_CRYPTO_ACCEL_DRV_C - the wrapped-key vendor driver. Without it a
 *       vendor-defined key type returns PSA_ERROR_NOT_SUPPORTED; see crypto_hw.c. */

#endif /* MBEDTLS_ACCELERATOR_CONFIG_H */
