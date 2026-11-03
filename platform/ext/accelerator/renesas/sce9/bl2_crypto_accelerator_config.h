/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * PSA crypto configuration adjustments for BL2: none.
 *
 * bl2/ext/mcuboot/config/mcuboot_crypto_config.h includes this whenever CRYPTO_HW_ACCELERATOR
 * is defined, as MBEDTLS_ACCELERATOR_PSA_CRYPTO_CONFIG_FILE. MBEDTLS_SHA256_ALT
 * (bl2_mbedtls_accelerator_config.h) accelerates SHA-256 beneath the PSA built-in driver, so
 * the PSA algorithm set (SHA-256, and ECDSA P-256 via p256-m) is unchanged.
 */

#ifndef BL2_CRYPTO_ACCELERATOR_CONFIG_H
#define BL2_CRYPTO_ACCELERATOR_CONFIG_H

#endif /* BL2_CRYPTO_ACCELERATOR_CONFIG_H */
