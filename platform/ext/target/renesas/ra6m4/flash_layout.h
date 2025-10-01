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
 * Memory Map:
 * 0x0000_0000 - 0x0001_FFFF : BL2 Bootloader (128KB)
 * 0x0002_0000 - 0x0007_FFFF : Secure Image (384KB)
 * 0x0008_0000 - 0x000F_FFFF : Non-Secure Image (512KB)
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

/* Maximum number of status entries supported by the bootloader */
#define MCUBOOT_STATUS_MAX_ENTRIES      ((FLASH_NS_PARTITION_SIZE) / \
                                         FLASH_AREA_IMAGE_SECTOR_SIZE)

/* Flash area for secure image (primary slot) */
#define FLASH_AREA_0_ID                 1
#define FLASH_AREA_0_OFFSET             (FLASH_AREA_BL2_OFFSET + FLASH_AREA_BL2_SIZE)
#define FLASH_AREA_0_SIZE               0x60000     /* 384KB */

/* Flash area for non-secure image (primary slot) */
#define FLASH_AREA_1_ID                 2
#define FLASH_AREA_1_OFFSET             (FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE)
#define FLASH_AREA_1_SIZE               0x80000     /* 512KB */

/* Secure partition sizes */
#define FLASH_S_PARTITION_SIZE          FLASH_AREA_0_SIZE
#define FLASH_NS_PARTITION_SIZE         FLASH_AREA_1_SIZE

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
#define OTP_NV_COUNTERS_FLASH_DEV           FLASH_DEVICE_DATA
#define TFM_HAL_ITS_FLASH_DRIVER            FLASH_DEVICE_DATA
#define TFM_HAL_ITS_PROGRAM_UNIT            0x1

/* OTP NV Counters configuration */
#define TFM_OTP_NV_COUNTERS_AREA_ADDR       FLASH_OTP_NV_COUNTERS_AREA_OFFSET
#define TFM_OTP_NV_COUNTERS_AREA_SIZE       FLASH_OTP_NV_COUNTERS_AREA_SIZE
#define TFM_OTP_NV_COUNTERS_SECTOR_SIZE     FLASH_DATA_FLASH_SECTOR_SIZE
#define TFM_OTP_NV_COUNTERS_BACKUP_AREA_ADDR (TFM_OTP_NV_COUNTERS_AREA_ADDR +                                               TFM_OTP_NV_COUNTERS_AREA_SIZE)

#endif /* __FLASH_LAYOUT_H__ */