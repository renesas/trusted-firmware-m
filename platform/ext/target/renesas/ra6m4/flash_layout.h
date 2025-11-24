/*
 * Copyright (c) 2025 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __FLASH_LAYOUT_H__
#define __FLASH_LAYOUT_H__

/* Flash layout for Renesas RA6M4 (R7FA6M4AF)
 *
 * Total Flash: 1MB (0x100000)
 * Flash Block Size: 8KB (minimum erase size)
 *
 * Optimized Memory Map for MCUboot Swap Mode:
 *
 * Without BL2:
 * 0x0000_0000 - 0x000F_FFFF : All available for application (1MB)
 *
 * With BL2 (MCUboot) - Full configuration with all TF-M services:
 * 0x0000_0000 - 0x0001_FFFF : BL2 Bootloader (128KB allocated, ~26KB used)
 * 0x0002_0000 - 0x0004_FFFF : Secure Image Primary Slot (192KB allocated, ~170KB used)
 * 0x0005_0000 - 0x0006_FFFF : Non-Secure Image Primary Slot (128KB)
 * 0x0007_0000 - 0x0009_FFFF : Secure Image Secondary Slot (192KB) - OTA updates
 * 0x000A_0000 - 0x000B_FFFF : Non-Secure Image Secondary Slot (128KB) - OTA updates
 * 0x000C_0000 - 0x000F_FFFF : Scratch Area (256KB) - for MCUboot swap operations
 *
 * Current TF-M build (with Crypto, ITS, PS, Attestation, Platform services):
 *   - BL2 (MCUboot): 26KB flash, 20KB RAM (20.31% of allocation)
 *   - TF-M Secure: 170KB flash, 47KB RAM (88.54% of allocation)
 *
 * Data Flash (8KB at 0x0800_0000):
 *   - OTP/NV Counters: 2KB
 *   - Protected Storage (PS): 3KB - encrypted with AES-GCM
 *   - Internal Trusted Storage (ITS): 2KB
 */

/* Flash base address */
#define FLASH_BASE_ADDRESS              0x00000000

/* Flash total size */
#define FLASH_TOTAL_SIZE                0x00100000  /* 1MB */

/* Flash area for BL2 bootloader */
#define FLASH_AREA_BL2_OFFSET           0x0
#define FLASH_AREA_BL2_SIZE             0x20000     /* 128KB */

/* Sector size (minimum erase unit) for RA6M4 */
#define FLASH_AREA_IMAGE_SECTOR_SIZE    0x2000      /* 8KB */

/* Flash area for secure image (primary slot) */
#define FLASH_AREA_0_ID                 1
#ifdef BL2
#define FLASH_AREA_0_OFFSET             (FLASH_AREA_BL2_OFFSET + FLASH_AREA_BL2_SIZE)  /* 0x20000 */
#define FLASH_AREA_0_SIZE               0x30000     /* 192KB - increased for modular build */
#else
#define FLASH_AREA_0_OFFSET             0x0         /* Without BL2, start at beginning of flash */
#define FLASH_AREA_0_SIZE               0xA0000     /* 640KB without BL2 */
#endif

/* Flash area for non-secure image (primary slot) */
#define FLASH_AREA_1_ID                 2
#define FLASH_AREA_1_OFFSET             (FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE)  /* 0x40000 */
#define FLASH_AREA_1_SIZE               0x20000     /* 128KB */

/* Secondary slot for secure image (for MCUboot swap upgrade) */
#define FLASH_AREA_2_ID                 (FLASH_AREA_1_ID + 1)
#define FLASH_AREA_2_OFFSET             (FLASH_AREA_1_OFFSET + FLASH_AREA_1_SIZE)  /* 0x60000 */
#define FLASH_AREA_2_SIZE               0x30000     /* 192KB - matches primary secure slot */

/* Secondary slot for non-secure image (for MCUboot swap upgrade) */
#define FLASH_AREA_3_ID                 (FLASH_AREA_2_ID + 1)
#define FLASH_AREA_3_OFFSET             (FLASH_AREA_2_OFFSET + FLASH_AREA_2_SIZE)  /* 0x80000 */
#define FLASH_AREA_3_SIZE               0x20000     /* 128KB */

/* Scratch area for MCUboot swap operations */
#define FLASH_AREA_SCRATCH_ID           (FLASH_AREA_3_ID + 1)
#define FLASH_AREA_SCRATCH_OFFSET       (FLASH_AREA_3_OFFSET + FLASH_AREA_3_SIZE)  /* 0xA0000 */
#define FLASH_AREA_SCRATCH_SIZE         0x40000     /* 256KB - reduced from 384KB */

/* Secure partition sizes */
#define FLASH_S_PARTITION_SIZE          FLASH_AREA_0_SIZE
#define FLASH_NS_PARTITION_SIZE         FLASH_AREA_1_SIZE

/* Maximum number of image sectors (for MCUboot) */
/* Based on largest partition size (scratch area is 256KB) */
#define MCUBOOT_MAX_IMG_SECTORS         (FLASH_AREA_SCRATCH_SIZE / \
                                         FLASH_AREA_IMAGE_SECTOR_SIZE)  /* 256KB / 8KB = 32 sectors */

/* Maximum number of status entries supported by the bootloader */
#define MCUBOOT_STATUS_MAX_ENTRIES      MCUBOOT_MAX_IMG_SECTORS

/* Data Flash area for PS/ITS/NV counters */
#define FLASH_DATA_FLASH_BASE           0x08000000
#define FLASH_DATA_FLASH_SIZE           0x2000      /* 8KB */
#define FLASH_DATA_FLASH_SECTOR_SIZE    0x40        /* 64 bytes */

/* OTP / NV counters area in data flash */
#define FLASH_OTP_NV_COUNTERS_AREA_OFFSET   FLASH_DATA_FLASH_BASE
#define FLASH_OTP_NV_COUNTERS_AREA_SIZE     0x800   /* 2KB */

/* PS area in data flash */
#define FLASH_PS_AREA_OFFSET                (FLASH_OTP_NV_COUNTERS_AREA_OFFSET + \
                                             FLASH_OTP_NV_COUNTERS_AREA_SIZE)
#define FLASH_PS_AREA_SIZE                  0xC00   /* 3KB */

/* ITS area in data flash */
#define FLASH_ITS_AREA_OFFSET               (FLASH_PS_AREA_OFFSET + \
                                             FLASH_PS_AREA_SIZE)
#define FLASH_ITS_AREA_SIZE                 0x800   /* 2KB */

/* Flash device IDs */
#define FLASH_DEVICE_ID                     0
#define FLASH_DEVICE_DATA                   1

/* Offset and size definitions for TFM flash driver */
#define TFM_HAL_FLASH_PROGRAM_UNIT          0x1     /* 1 byte programming */

/* Flash driver definitions for OTP/NV counters */
/* Flash device name for BL2 (MCUboot) */
#define FLASH_DEV_NAME                  Driver_FLASH0

/* Use Driver_FLASH1 for data flash (OTP/ITS storage) */
#define TFM_HAL_ITS_FLASH_DRIVER            Driver_FLASH1
#define TFM_HAL_ITS_PROGRAM_UNIT            0x1

#define TFM_HAL_ITS_FLASH_AREA_ADDR         FLASH_ITS_AREA_OFFSET
#define TFM_HAL_ITS_FLASH_AREA_SIZE         FLASH_ITS_AREA_SIZE
#define TFM_HAL_ITS_SECTORS_PER_BLOCK       (0x800 / FLASH_DATA_FLASH_SECTOR_SIZE)   /* 32 sectors per block */

#define TFM_HAL_PS_FLASH_DRIVER             Driver_FLASH1
#define TFM_HAL_PS_PROGRAM_UNIT             0x1
#define TFM_HAL_PS_FLASH_AREA_ADDR          FLASH_PS_AREA_OFFSET
#define TFM_HAL_PS_FLASH_AREA_SIZE          FLASH_PS_AREA_SIZE
#define TFM_HAL_PS_SECTORS_PER_BLOCK        (0xC00 / FLASH_DATA_FLASH_SECTOR_SIZE)   /* 48 sectors per block */

/* OTP_NV_COUNTERS_FLASH_DEV will default to TFM_HAL_ITS_FLASH_DRIVER if not defined */
/* Uncomment below to use a different driver for OTP counters */
/* #define OTP_NV_COUNTERS_FLASH_DEV       Driver_FLASH1 */

/* OTP NV Counters configuration */
#define TFM_OTP_NV_COUNTERS_AREA_ADDR       FLASH_OTP_NV_COUNTERS_AREA_OFFSET
#define TFM_OTP_NV_COUNTERS_AREA_SIZE       FLASH_OTP_NV_COUNTERS_AREA_SIZE
#define TFM_OTP_NV_COUNTERS_SECTOR_SIZE     FLASH_DATA_FLASH_SECTOR_SIZE
#define TFM_OTP_NV_COUNTERS_BACKUP_AREA_ADDR (TFM_OTP_NV_COUNTERS_AREA_ADDR + \
                                              TFM_OTP_NV_COUNTERS_AREA_SIZE)
#define OTP_NV_COUNTERS_WRITE_BLOCK_SIZE    1  /* 1 byte write unit for data flash */

#endif /* __FLASH_LAYOUT_H__ */
