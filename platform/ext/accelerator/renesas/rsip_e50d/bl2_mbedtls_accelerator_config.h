/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Mbed TLS configuration for the Renesas RSIP-E50D accelerator - BL2.
 *
 * bl2/ext/mcuboot/config/mcuboot-mbedtls-cfg.h includes this LAST, as
 * MBEDTLS_ACCELERATOR_CONFIG_FILE, under CRYPTO_HW_ACCELERATOR.
 *
 * BL2 accelerates SHA-256 only. MCUboot runs two primitives per image per boot: a SHA-256
 * over the whole slot payload, and one P-256 signature verify. The hash is where the time
 * goes and it scales with image size.
 *
 * The verify is deliberately left on p256-m. BL2 routes P-256 to it through
 * MBEDTLS_PSA_P256M_DRIVER_ENABLED, so an ECDSA ALT would never be reached; replacing p256-m
 * would pull in bignum and FSP's whole ECP_ALT to speed up one verify per image, out of a
 * 64 KB BL2 budget on this part (RA8M2 has 1 MB of MRAM total - see the RA8M2 flash_layout.h
 * for why the budget is tighter here than on RA6M5).
 */

#ifndef BL2_MBEDTLS_ACCELERATOR_CONFIG_H
#define BL2_MBEDTLS_ACCELERATOR_CONFIG_H

/* Keep FSP's MCUboot flash-area helpers out of BL2.
 *
 * rsip_e50d_fsp_cfg.h includes bsp_api.h, and bsp_common.h includes the bootloader project's
 * generated bsp_linker_info.h unconditionally. Because that project carries rm_mcuboot_port,
 * the header holds FSP's own FLASH_AREA_IMAGE_PRIMARY/SECONDARY - as inline functions -
 * under "#ifdef __SYSFLASH_H__ / #ifndef __SYSFLASH_BSP_LINKER_H", meant to complete FSP's
 * sysflash.h. TF-M's bl2/ext/mcuboot/include/sysflash/sysflash.h uses the same
 * __SYSFLASH_H__ guard and defines those names as macros, so any bootutil file that includes
 * sysflash.h and then a PSA header (image_ecdsa.c does) gets both and fails to compile.
 *
 * __SYSFLASH_BSP_LINKER_H is FSP's own opt-out for that block; defining it here skips the
 * helpers and nothing else. It only matters where the BL2 crypto config is reached, and BL2
 * uses TF-M's flash map, never FSP's (DESIGN.md 4, 5). */
#ifndef __SYSFLASH_BSP_LINKER_H
#define __SYSFLASH_BSP_LINKER_H
#endif

#include "rsip_e50d_fsp_cfg.h"

#define MBEDTLS_SHA256_ALT
#define MBEDTLS_SHA256_PROCESS_ALT

#endif /* BL2_MBEDTLS_ACCELERATOR_CONFIG_H */
