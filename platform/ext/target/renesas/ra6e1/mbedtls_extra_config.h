/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Appended to TF-M's mbedcrypto config as MBEDTLS_USER_CONFIG_FILE, via
 * TFM_MBEDCRYPTO_PLATFORM_EXTRA_CONFIG_PATH in config.cmake.
 *
 * This is NOT FSP's mbedTLS config. FSP's ra_cfg/arm/mbedtls/config.h includes bsp_api.h,
 * carries the rm_psa_crypto module config inline, and wires PSA ITS to littlefs - which
 * collides with TFM_PARTITION_INTERNAL_TRUSTED_STORAGE. See DESIGN.md 6.
 */

#ifndef __MBEDTLS_EXTRA_CONFIG_H__
#define __MBEDTLS_EXTRA_CONFIG_H__

/* Take randomness from mbedtls_psa_external_get_random() - sce_trng.c, backed by the
 * SCE9 TRNG - instead of the entropy/DRBG path fed by the NV seed. */
#undef MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG
#define MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG 1

#endif /* __MBEDTLS_EXTRA_CONFIG_H__ */
