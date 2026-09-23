/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Mbed TLS configuration for the Renesas SCE9 accelerator (secure crypto partition).
 *
 * Included LAST by tfm_mbedcrypto_config_*.h under CRYPTO_HW_ACCELERATOR, so anything
 * defined here overrides the profile.
 *
 * MODEL: legacy *_ALT. FSP's rm_psa_crypto ships SCE9 implementations of Mbed TLS modules
 * as forks of the library sources (aes_alt.c for aes.c, gcm_alt.c for gcm.c, ...), compiled
 * against FSP's OWN Mbed TLS - the port builds the crypto library from it rather than from
 * upstream, because the ALT sources depend on FSP's PSA core (ra6m5/cmake/fsp_mbedtls.cmake).
 * This is Mbed TLS 3.6-only: TF-M 2.3 moves to TF-PSA-Crypto, which has no *_ALT mechanism,
 * and there this becomes a PSA transparent driver (PROJECT_PLAN.md P7, DECISIONS D035).
 *
 * SCOPE: plaintext keys only. FSP's wrapped-key "vendor" driver (vendor.c, aes_vendor.c,
 * MBEDTLS_PSA_CRYPTO_ACCEL_DRV_C) depends on FSP's patched Mbed TLS and is not built.
 */

#ifndef MBEDTLS_ACCELERATOR_CONFIG_H
#define MBEDTLS_ACCELERATOR_CONFIG_H

/* FSP's rm_psa_crypto configuration (plaintext keys only), bsp_api.h and BYTES_TO_WORDS -
 * shared with BL2. */
#include "sce9_fsp_cfg.h"

/* ----------------------------------------------------------------------------------------
 * Accelerated modules - FSP's SCE9 set.
 *
 * MBEDTLS_CIPHER_ALT comes with the rest, not as an extra: FSP's cipher_alt.c is where the
 * block chunking and the SCE session close live (see the CMakeLists). The AES ALT is written
 * against it.
 * -------------------------------------------------------------------------------------- */
#define MBEDTLS_CIPHER_ALT

#define MBEDTLS_AES_ALT
#define MBEDTLS_AES_SETKEY_ENC_ALT
#define MBEDTLS_AES_SETKEY_DEC_ALT
#define MBEDTLS_AES_ENCRYPT_ALT
#define MBEDTLS_AES_DECRYPT_ALT

#define MBEDTLS_GCM_ALT
#define MBEDTLS_CMAC_ALT

/* MBEDTLS_CCM_ALT is deliberately NOT defined - see DECISIONS D043.
 *
 * FSP's SCE9 CCM formats the whole B-block sequence into one 128 B hardware buffer
 * (HW_SCE_AES_CCM_B_FORMAT_BYTE_SIZE), so it accepts at most 110 B of associated data:
 * 16 (B0) + roundup16(2 + aad_len) <= 128. Beyond that it returns MBEDTLS_ERR_CCM_BAD_INPUT,
 * which the PSA layer reports as PSA_ERROR_INVALID_ARGUMENT.
 *
 * Protected Storage authenticates its object table with AES-CCM where the associated data is
 * the table itself - about 140 B at PS_NUM_ASSETS 10 - so every PS object-table write fails
 * and the partition does not initialise. CCM therefore stays in software, where it still
 * reaches the SCE9 per block through MBEDTLS_CIPHER_ALT/MBEDTLS_AES_ALT. */

#define MBEDTLS_SHA256_ALT
#define MBEDTLS_SHA256_PROCESS_ALT

#define MBEDTLS_RSA_ALT

/* ECC. FSP's ECDSA ALT reads its patched group struct (grp->vendor_ctx), so it comes with
 * MBEDTLS_ECP_ALT - ecp_alt.c / ecp_curves_alt.c replace Mbed TLS's ECP module whole.
 *
 * Curves: SCE9 signs and verifies secp256r1, secp256k1, brainpoolP256r1, secp384r1 and
 * brainpoolP384r1 (ecp_can_do_sce() in ecdsa_alt.c). There is NO software fallback - any
 * other curve returns MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE - and P-521 and Curve25519 have no
 * SCE9 procedures at all (their HW tables do not compile). crypto_accelerator_config.h
 * therefore removes those two curves from the PSA configuration. Curve448 stays: ECP scalar
 * multiplication falls back to software (ecp_mul_mxz), and nothing references hardware for
 * it. See crypto_accelerator_config.h also for deterministic ECDSA. */
#define MBEDTLS_ECP_ALT
#define MBEDTLS_ECDSA_SIGN_ALT
#define MBEDTLS_ECDSA_VERIFY_ALT

/* Deliberately NOT enabled, although FSP's own config sets them:
 *   MBEDTLS_ENTROPY_HARDWARE_ALT, MBEDTLS_CTR_DRBG_C_ALT - entropy is already hardware, via
 *       MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG and the platform's sce_trng.c.
 *   MBEDTLS_PLATFORM_SETUP_TEARDOWN_ALT - nothing in TF-M calls mbedtls_platform_setup();
 *       the engine is brought up by crypto_hw_accelerator_init() instead.
 *   MBEDTLS_PSA_CRYPTO_ACCEL_DRV_C - the wrapped-key vendor driver, see above.
 *   MBEDTLS_PSA_CRYPTO_ACCEL_DRV_C and the wrapped-key vendor driver - plaintext keys only,
 *       see above. */

#endif /* MBEDTLS_ACCELERATOR_CONFIG_H */
