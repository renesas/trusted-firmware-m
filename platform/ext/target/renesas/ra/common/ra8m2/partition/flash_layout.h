/*
 * Copyright (c) 2017-2022 Arm Limited. All rights reserved.
 * Copyright (c) 2020 Cypress Semiconductor Corporation. All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __FLASH_LAYOUT_H__
#define __FLASH_LAYOUT_H__

/* This header file is included from linker scatter file as well, where only a
 * limited C constructs are allowed. Therefore it is not possible to include
 * here the platform_retarget.h to access flash related defines. To resolve this
 * some of the values are redefined here with different names, these are marked
 * with comment.
 */

/* All geometry values below are injected as compiler -D definitions from the
 * platform's config.cmake (via platform_region_defs / tfm_config); see
 * ek_ra8m2_r7ka8m2jf_cm85/config.cmake and check_config.cmake for the RA8M2
 * hardware facts and validation behind these values. A missing definition is
 * a build-system bug, not a valid "use zero" default, hence the guards below.
 */
#ifndef FLASH_BASE_ADDRESS
#error "FLASH_BASE_ADDRESS must be defined by config.cmake"
#endif
#ifndef FLASH_TOTAL_SIZE
#error "FLASH_TOTAL_SIZE must be defined by config.cmake"
#endif
#ifndef FLASH_AREA_IMAGE_SECTOR_SIZE
#error "FLASH_AREA_IMAGE_SECTOR_SIZE must be defined by config.cmake"
#endif
#ifndef FLASH_PAGE_SIZE
#error "FLASH_PAGE_SIZE must be defined by config.cmake"
#endif
#ifndef FLASH_PROGRAM_UNIT
#error "FLASH_PROGRAM_UNIT must be defined by config.cmake"
#endif
#ifndef FLASH_AREA_BL2_OFFSET
#error "FLASH_AREA_BL2_OFFSET must be defined by config.cmake"
#endif
#ifndef FLASH_AREA_BL2_SIZE
#error "FLASH_AREA_BL2_SIZE must be defined by config.cmake"
#endif
#ifndef FLASH_S_PARTITION_SIZE
#error "FLASH_S_PARTITION_SIZE must be defined by config.cmake"
#endif
#ifndef FLASH_CPU0_NS_PARTITION_SIZE
#error "FLASH_CPU0_NS_PARTITION_SIZE must be defined by config.cmake"
#endif
#ifndef FLASH_CPU1_NS_PARTITION_SIZE
#error "FLASH_CPU1_NS_PARTITION_SIZE must be defined by config.cmake"
#endif
#ifndef FLASH_PS_AREA_SIZE
#error "FLASH_PS_AREA_SIZE must be defined by config.cmake"
#endif
#ifndef FLASH_ITS_AREA_SIZE
#error "FLASH_ITS_AREA_SIZE must be defined by config.cmake"
#endif
#ifndef FLASH_OTP_NV_COUNTERS_AREA_SIZE
#error "FLASH_OTP_NV_COUNTERS_AREA_SIZE must be defined by config.cmake"
#endif
#ifndef TOTAL_RAM_SIZE
#error "TOTAL_RAM_SIZE must be defined by config.cmake"
#endif
#ifndef S_ROM_ALIAS_BASE
#error "S_ROM_ALIAS_BASE must be defined by config.cmake"
#endif
#ifndef CPU0_NS_ROM_ALIAS_BASE
#error "CPU0_NS_ROM_ALIAS_BASE must be defined by config.cmake"
#endif
#ifndef S_RAM_ALIAS_BASE
#error "S_RAM_ALIAS_BASE must be defined by config.cmake"
#endif
#ifndef CPU0_NS_RAM_ALIAS_BASE
#error "CPU0_NS_RAM_ALIAS_BASE must be defined by config.cmake"
#endif

/* TODO: Support BL2 with both CPU0 and CPU1 Non-secure images  */
#define FLASH_NS_PARTITION_SIZE FLASH_CPU0_NS_PARTITION_SIZE

#if (FLASH_S_PARTITION_SIZE > FLASH_NS_PARTITION_SIZE)
#define FLASH_MAX_PARTITION_SIZE FLASH_S_PARTITION_SIZE
#else
#define FLASH_MAX_PARTITION_SIZE FLASH_NS_PARTITION_SIZE
#endif

/* Offset and size definitions of the flash partitions that are handled by the
 * bootloader. The image swapping is done between IMAGE_PRIMARY and
 * IMAGE_SECONDARY, SCRATCH is used as a temporary storage during image
 * swapping. BL2 is unsupported on this platform (no MRAM-capable bootloader
 * flash driver exists), so the secondary/scratch areas collapse to zero and
 * are gated on BL2 below -- keeping them unconditional would chain PS/ITS/OTP
 * past the end of MRAM.
 */
#if !defined(MCUBOOT_IMAGE_NUMBER) || (MCUBOOT_IMAGE_NUMBER == 1)
/* Secure + Non-secure image primary slot */
#define FLASH_AREA_0_ID           (1)
#define FLASH_AREA_0_OFFSET       (FLASH_AREA_BL2_OFFSET + FLASH_AREA_BL2_SIZE)
#define FLASH_AREA_0_SIZE         (FLASH_S_PARTITION_SIZE + FLASH_NS_PARTITION_SIZE)
#ifdef BL2
/* Secure + Non-secure secondary slot */
#define FLASH_AREA_2_ID           (FLASH_AREA_0_ID + 1)
#define FLASH_AREA_2_OFFSET       (FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE)
#define FLASH_AREA_2_SIZE         (FLASH_S_PARTITION_SIZE + FLASH_NS_PARTITION_SIZE)
/* Scratch area */
#define FLASH_AREA_SCRATCH_ID     (FLASH_AREA_2_ID + 1)
#define FLASH_AREA_SCRATCH_OFFSET (FLASH_AREA_2_OFFSET + FLASH_AREA_2_SIZE)
#define FLASH_AREA_SCRATCH_SIZE   (FLASH_S_PARTITION_SIZE + FLASH_NS_PARTITION_SIZE)
/* The maximum number of status entries supported by the bootloader. */
#define MCUBOOT_STATUS_MAX_ENTRIES                                                                 \
	((FLASH_S_PARTITION_SIZE + FLASH_NS_PARTITION_SIZE) / FLASH_AREA_SCRATCH_SIZE)
/* Maximum number of image sectors supported by the bootloader. */
#define MCUBOOT_MAX_IMG_SECTORS                                                                    \
	((FLASH_S_PARTITION_SIZE + FLASH_NS_PARTITION_SIZE) / FLASH_AREA_IMAGE_SECTOR_SIZE)
#else /* !BL2 */
#define FLASH_AREA_2_OFFSET       (FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE)
#define FLASH_AREA_2_SIZE         (0x0)
#define FLASH_AREA_SCRATCH_OFFSET (FLASH_AREA_2_OFFSET + FLASH_AREA_2_SIZE)
#define FLASH_AREA_SCRATCH_SIZE   (0x0)
#endif /* BL2 */
#elif (MCUBOOT_IMAGE_NUMBER == 2)
/* Secure image primary slot */
#define FLASH_AREA_0_ID            (1)
#define FLASH_AREA_0_OFFSET        (FLASH_AREA_BL2_OFFSET + FLASH_AREA_BL2_SIZE)
#define FLASH_AREA_0_SIZE          (FLASH_S_PARTITION_SIZE)
/* Non-secure image primary slot */
#define FLASH_AREA_1_ID            (FLASH_AREA_0_ID + 1)
#define FLASH_AREA_1_OFFSET        (FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE)
#define FLASH_AREA_1_SIZE          (FLASH_NS_PARTITION_SIZE)
#ifdef BL2
/* Secure image secondary slot */
#define FLASH_AREA_2_ID            (FLASH_AREA_1_ID + 1)
#define FLASH_AREA_2_OFFSET        (FLASH_AREA_1_OFFSET + FLASH_AREA_1_SIZE)
#define FLASH_AREA_2_SIZE          (FLASH_S_PARTITION_SIZE)
/* Non-secure image secondary slot */
#define FLASH_AREA_3_ID            (FLASH_AREA_2_ID + 1)
#define FLASH_AREA_3_OFFSET        (FLASH_AREA_2_OFFSET + FLASH_AREA_2_SIZE)
#define FLASH_AREA_3_SIZE          (FLASH_NS_PARTITION_SIZE)
/* Scratch area */
#define FLASH_AREA_SCRATCH_ID      (FLASH_AREA_3_ID + 1)
#define FLASH_AREA_SCRATCH_OFFSET  (FLASH_AREA_3_OFFSET + FLASH_AREA_3_SIZE)
#define FLASH_AREA_SCRATCH_SIZE    (FLASH_MAX_PARTITION_SIZE)
/* The maximum number of status entries supported by the bootloader. */
#define MCUBOOT_STATUS_MAX_ENTRIES (FLASH_MAX_PARTITION_SIZE / FLASH_AREA_SCRATCH_SIZE)
/* Maximum number of image sectors supported by the bootloader. */
#define MCUBOOT_MAX_IMG_SECTORS    (FLASH_MAX_PARTITION_SIZE / FLASH_AREA_IMAGE_SECTOR_SIZE)
#else /* !BL2 */
#define FLASH_AREA_2_OFFSET        (FLASH_AREA_1_OFFSET + FLASH_AREA_1_SIZE)
#define FLASH_AREA_2_SIZE          (0x0)
#define FLASH_AREA_3_OFFSET        (FLASH_AREA_2_OFFSET + FLASH_AREA_2_SIZE)
#define FLASH_AREA_3_SIZE          (0x0)
#define FLASH_AREA_SCRATCH_OFFSET  (FLASH_AREA_3_OFFSET + FLASH_AREA_3_SIZE)
#define FLASH_AREA_SCRATCH_SIZE    (0x0)
#endif /* BL2 */
#else /* MCUBOOT_IMAGE_NUMBER > 2 */
#error "Only MCUBOOT_IMAGE_NUMBER 1 and 2 are supported!"
#endif /* MCUBOOT_IMAGE_NUMBER */

/* Protected Storage (PS) Service definitions
 * The storage areas (PS/ITS/OTP) are NOT part of the signed S image: they
 * live in dedicated secure-attributed MRAM directly after the S slot
 */
/* TODO: Add support for configuring the PS area in external memory. */
#define FLASH_PS_AREA_OFFSET (FLASH_AREA_0_OFFSET + FLASH_S_PARTITION_SIZE)

/* Internal Trusted Storage (ITS) Service definitions */
#define FLASH_ITS_AREA_OFFSET (FLASH_PS_AREA_OFFSET + FLASH_PS_AREA_SIZE)

/* OTP_definitions */
#define FLASH_OTP_NV_COUNTERS_AREA_OFFSET (FLASH_ITS_AREA_OFFSET + FLASH_ITS_AREA_SIZE)
#define FLASH_OTP_NV_COUNTERS_SECTOR_SIZE FLASH_AREA_IMAGE_SECTOR_SIZE

/* Offset and size definition in flash area used by assemble.py */
#define SECURE_IMAGE_OFFSET   (0x0) /* Used for BL2: currently not supported */
#define SECURE_IMAGE_MAX_SIZE FLASH_S_PARTITION_SIZE

#define NON_SECURE_IMAGE_OFFSET   (NS_IMAGE_PRIMARY_PARTITION_OFFSET)
#define NON_SECURE_IMAGE_MAX_SIZE FLASH_NS_PARTITION_SIZE

/* Flash device name used by BL2
 * Name is defined in flash driver file: Driver_Flash.c
 */
#define FLASH_DEV_NAME             Driver_Flash0
/* Smallest flash programmable unit in bytes */
#define TFM_HAL_FLASH_PROGRAM_UNIT (FLASH_PROGRAM_UNIT)

/* Protected Storage (PS) Service definitions
 * Note: Further documentation of these definitions can be found in the
 * TF-M PS Integration Guide.
 */
#define TFM_HAL_PS_FLASH_DRIVER Driver_Flash0

/* In this target the CMSIS driver requires only the offset from the base
 * address instead of the full memory address.
 */
/* Base address of dedicated flash area for PS */
#define TFM_HAL_PS_FLASH_AREA_ADDR   FLASH_PS_AREA_OFFSET
/* Size of dedicated flash area for PS */
#define TFM_HAL_PS_FLASH_AREA_SIZE   FLASH_PS_AREA_SIZE
#define PS_RAM_FS_SIZE               TFM_HAL_PS_FLASH_AREA_SIZE
/* Number of physical erase sectors per logical FS block */
#define TFM_HAL_PS_SECTORS_PER_BLOCK (1)
/* Smallest flash programmable unit in bytes */
#define TFM_HAL_PS_PROGRAM_UNIT      (FLASH_PROGRAM_UNIT)

/* Secure flash boundary information for on-chip flash */
/* TODO: Update the secure boundary when Protected Storage (PS) supports external memory. */
#define RA_HAL_FLASH_SECURE_BOUNDARY (FLASH_S_PARTITION_SIZE + \
                                      FLASH_PS_AREA_SIZE + \
                                      FLASH_ITS_AREA_SIZE + \
                                      FLASH_OTP_NV_COUNTERS_AREA_SIZE)

/* Internal Trusted Storage (ITS) Service definitions
 * Note: Further documentation of these definitions can be found in the
 * TF-M ITS Integration Guide. The ITS should be in the internal flash, but is
 * allocated in the external flash just for development platforms that don't
 * have internal flash available.
 */
#define TFM_HAL_ITS_FLASH_DRIVER Driver_Flash0

/* In this target the CMSIS driver requires only the offset from the base
 * address instead of the full memory address.
 */
/* Base address of dedicated flash area for ITS */
#define TFM_HAL_ITS_FLASH_AREA_ADDR   FLASH_ITS_AREA_OFFSET
/* Size of dedicated flash area for ITS */
#define TFM_HAL_ITS_FLASH_AREA_SIZE   FLASH_ITS_AREA_SIZE
#define ITS_RAM_FS_SIZE               TFM_HAL_ITS_FLASH_AREA_SIZE
/* Number of physical erase sectors per logical FS block */
#define TFM_HAL_ITS_SECTORS_PER_BLOCK (1)
/* Smallest flash programmable unit in bytes */
#define TFM_HAL_ITS_PROGRAM_UNIT      (FLASH_PROGRAM_UNIT)

/* NAND-style storage backend staging buffers (rp2350 pattern): a program unit
 * above 16 bytes forces the NAND backend in its_flash.c/its_flash_ps.c, which
 * #errors without these. 2 blocks is the smallest block-aligned staging size.
 * Only compiled in when the ITS/PS partitions are enabled.
 */
#define ITS_FLASH_NAND_BUF_SIZE (2 * FLASH_AREA_IMAGE_SECTOR_SIZE)
#define PS_FLASH_NAND_BUF_SIZE  (2 * FLASH_AREA_IMAGE_SECTOR_SIZE)

/* OTP / NV counter definitions */
#define TFM_OTP_NV_COUNTERS_AREA_SIZE   (FLASH_OTP_NV_COUNTERS_AREA_SIZE / 2)
#define TFM_OTP_NV_COUNTERS_AREA_ADDR   FLASH_OTP_NV_COUNTERS_AREA_OFFSET
#define TFM_OTP_NV_COUNTERS_SECTOR_SIZE FLASH_OTP_NV_COUNTERS_SECTOR_SIZE
#define TFM_OTP_NV_COUNTERS_BACKUP_AREA_ADDR                                                       \
	(TFM_OTP_NV_COUNTERS_AREA_ADDR + TFM_OTP_NV_COUNTERS_AREA_SIZE)

#define TOTAL_ROM_SIZE FLASH_TOTAL_SIZE

#define TFM_NV_COUNTERS_AREA_SIZE (0x20) /* 32 Bytes */

/* TODO: Enable TCM for secure runtime */
/* #if 0
 * #define S_RUNTIME_DTCM_START (DTCM_CPU0_S_START)
 * #define S_RUNTIME_DTCM_SIZE  (DTCM_CPU0_S_SIZE)
 * #define S_RUNTIME_ITCM_START (ITCM_CPU0_S_START)
 * #define S_RUNTIME_ITCM_SIZE  (ITCM_CPU0_S_SIZE)
 * #endif
 */

#endif /* __FLASH_LAYOUT_H__ */
