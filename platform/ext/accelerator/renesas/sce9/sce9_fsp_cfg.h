/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * The part of FSP's generated Mbed TLS configuration that FSP's rm_psa_crypto *_ALT sources
 * need, shared by the secure (mbedtls_accelerator_config.h) and BL2
 * (bl2_mbedtls_accelerator_config.h) configurations. FSP generates these into
 * ra_cfg/arm/mbedtls/config.h; TF-M uses its own Mbed TLS configuration, so they are
 * restated here.
 */

#ifndef SCE9_FSP_CFG_H
#define SCE9_FSP_CFG_H

/* The ALT sources test BSP_FEATURE_RSIP_* to pick the engine variant, and their headers use
 * FSP_HEADER / FSP_FOOTER. bsp_api.h provides both; the accelerator puts the FSP include
 * paths on each crypto library that includes this. */
#include "bsp_api.h"

/* rm_psa_crypto module configuration.
 *
 * Key format is PLAINTEXT only (0x01). FSP's default is 3 (plaintext + wrapped); with the
 * wrapped bit clear, every PSA_CRYPTO_IS_WRAPPED_SUPPORT_REQUIRED() branch in the ALT sources
 * compiles out, which is what keeps the wrapped-key vendor driver out of the image. */
#define PSA_CRYPTO_CFG_PLAINTEXT_KEY_SUPPORT                (0x01)
#define PSA_CRYPTO_CFG_WRAPPED_KEY_SUPPORT                  (0x02)
#define PSA_CRYPTO_CFG_WRAPPED_AND_PLAINTEXT_KEY_SUPPORT    ((PSA_CRYPTO_CFG_PLAINTEXT_KEY_SUPPORT) | \
                                                             (PSA_CRYPTO_CFG_WRAPPED_KEY_SUPPORT))

#define PSA_CRYPTO_IS_WRAPPED_SUPPORT_REQUIRED(type)        ((type) & PSA_CRYPTO_CFG_WRAPPED_KEY_SUPPORT)
#define PSA_CRYPTO_IS_PLAINTEXT_SUPPORT_REQUIRED(type)      ((type) & PSA_CRYPTO_CFG_PLAINTEXT_KEY_SUPPORT)

#define PSA_CRYPTO_CFG_AES_FORMAT                           (PSA_CRYPTO_CFG_PLAINTEXT_KEY_SUPPORT)
#define PSA_CRYPTO_CFG_ECC_FORMAT                           (PSA_CRYPTO_CFG_PLAINTEXT_KEY_SUPPORT)
#define PSA_CRYPTO_CFG_RSA_FORMAT                           (PSA_CRYPTO_CFG_PLAINTEXT_KEY_SUPPORT)

#define RM_PSA_CRYPTO_CFG_RSA3K_KEYGEN_ENABLED              0
#define RM_PSA_CRYPTO_CFG_RSA3K_SIGNING_ENABLED             0
#define RM_PSA_CRYPTO_CFG_RSA3K_VERIFICATION_ENABLED        0
#define RM_PSA_CRYPTO_CFG_RSA4K_KEYGEN_ENABLED              0
#define RM_PSA_CRYPTO_CFG_RSA4K_SIGNING_ENABLED             0
#define RM_PSA_CRYPTO_CFG_RSA4K_VERIFICATION_ENABLED        0

/* The ALT *_process.c files use this, but FSP defines it in rm_psa_crypto/inc/platform_alt.h,
 * which mbedtls/platform.h includes only under MBEDTLS_PLATFORM_SETUP_TEARDOWN_ALT - not set
 * by TF-M. Without it the call compiles as an implicit function and fails at link. Same
 * arithmetic as FSP's, with the argument parenthesised. */
#ifndef BYTES_TO_WORDS
#define BYTES_TO_WORDS(x)                                   ((((x) + 3U) >> 2))
#endif

#endif /* SCE9_FSP_CFG_H */
