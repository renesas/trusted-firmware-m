/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Mbed TLS configuration for the Renesas RSIP-E50D accelerator - BL2.
 *
 * bl2/ext/mcuboot/config/mcuboot-mbedtls-cfg.h includes this last, as
 * MBEDTLS_ACCELERATOR_CONFIG_FILE, under CRYPTO_HW_ACCELERATOR.
 *
 * BL2 accelerates SHA-256 only. Per image per boot, MCUboot runs one SHA-256 over the whole
 * slot payload, which takes most of the time and scales with image size, and one P-256
 * signature verify.
 *
 * The verify deliberately stays on p256-m. BL2 routes P-256 to p256-m through
 * MBEDTLS_PSA_P256M_DRIVER_ENABLED, so an ECDSA ALT would never be reached. Replacing p256-m
 * would pull bignum and FSP's whole ECP_ALT into the 64 KB BL2 budget to speed up
 * one verify per image. RA8M2 has 1 MB of MRAM in total; the RA8M2 flash_layout.h explains why
 * the budget is tighter than on RA6M5.
 */

#ifndef BL2_MBEDTLS_ACCELERATOR_CONFIG_H
#define BL2_MBEDTLS_ACCELERATOR_CONFIG_H

/* Keep FSP's MCUboot flash-area helpers out of BL2.
 *
 * rsip_e50d_fsp_cfg.h includes bsp_api.h, and bsp_common.h includes the bootloader project's
 * generated bsp_linker_info.h unconditionally. Because that project carries rm_mcuboot_port,
 * the header defines FSP's FLASH_AREA_IMAGE_PRIMARY/SECONDARY as inline functions under
 * "#ifdef __SYSFLASH_H__ / #ifndef __SYSFLASH_BSP_LINKER_H". TF-M's
 * bl2/ext/mcuboot/include/sysflash/sysflash.h uses the same __SYSFLASH_H__ guard and defines
 * those names as macros. A bootutil file that includes sysflash.h and then a PSA header
 * (image_ecdsa.c does) therefore gets both and fails to compile.
 *
 * __SYSFLASH_BSP_LINKER_H is FSP's opt-out for that block; defining it skips the helpers and
 * nothing else. BL2 uses TF-M's flash map, never FSP's (DESIGN.md 4, 5). */
#ifndef __SYSFLASH_BSP_LINKER_H
#define __SYSFLASH_BSP_LINKER_H
#endif

#include "rsip_e50d_fsp_cfg.h"

#define MBEDTLS_SHA256_ALT
#define MBEDTLS_SHA256_PROCESS_ALT

#endif /* BL2_MBEDTLS_ACCELERATOR_CONFIG_H */
