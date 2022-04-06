/*
 * Copyright (c) 2018-2020 Arm Limited. All rights reserved.
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

 #include "bsp_api.h"

 #ifdef __cplusplus
extern "C" {
 #endif

/* MCU boot configuration for RA Platform*/

#define FLASH_DEVICE_INTERNAL_FLASH    (0x7F)
#define FLASH_DEVICE_EXTERNAL_FLASH    (0x80)
#define FLASH_DEVICE_QSPI              (FLASH_DEVICE_EXTERNAL_FLASH | 0x01)

#define FLASH_AREA_BOOTLOADER          0
#define FLASH_AREA_IMAGE_0             1
#define FLASH_AREA_IMAGE_1             2
#define FLASH_AREA_SCRATCH_ID          3
#define FLASH_AREA_IMAGE_2             5
#define FLASH_AREA_IMAGE_3             6
#define FLASH_AREA_MCUBOOT_OFFSET     (0x0)
#define QSPI_AREA_MCUBOOT_OFFSET      (0x0)
#if BSP_FEATURE_FLASH_HP_SUPPORTS_DUAL_BANK
 #define RM_MCUBOOT_DUAL_BANK_ENABLED    (!(0x7U & (BSP_CFG_ROM_REG_DUALSEL)))
#endif

#define RM_MCUBOOT_PORT_CFG_MCUBOOT_SIZE           (0x18000)
// #if (MCUBOOT_IMAGE_NUMBER == 1)
//  #define RM_MCUBOOT_PORT_CFG_PARTITION_SIZE        (0x40000)
// #else
#define RM_MCUBOOT_PORT_CFG_S_PARTITION_SIZE      (0x40000)
// #endif
#define RM_MCUBOOT_PORT_CFG_NS_PARTITION_SIZE      (0x10000)
#define RM_MCUBOOT_PORT_CFG_SCRATCH_SIZE           (0x0)

#if BSP_FEATURE_FLASH_HP_VERSION > 0
 #define FLASH_AREA_IMAGE_SECTOR_SIZE    (BSP_FEATURE_FLASH_HP_CF_REGION1_BLOCK_SIZE) /* 32 KB */
 #define MCUBOOT_BOOT_MAX_ALIGN          (BSP_FEATURE_FLASH_HP_CF_WRITE_SIZE)
#else
#define FLASH_AREA_IMAGE_SECTOR_SIZE    (BSP_FEATURE_FLASH_LP_CF_BLOCK_SIZE)         /* 2 KB */
#endif

// #if (MCUBOOT_IMAGE_NUMBER == 1)
//  #define RM_MCUBOOT_LARGEST_SECTOR       (RM_MCUBOOT_PORT_CFG_PARTITION_SIZE)
// #elif (MCUBOOT_IMAGE_NUMBER == 2)
#define RM_MCUBOOT_LARGEST_SECTOR       ((RM_MCUBOOT_PORT_CFG_S_PARTITION_SIZE > \
                                          RM_MCUBOOT_PORT_CFG_NS_PARTITION_SIZE) ? \
                                          RM_MCUBOOT_PORT_CFG_S_PARTITION_SIZE : \
                                          RM_MCUBOOT_PORT_CFG_NS_PARTITION_SIZE)
// #else                                  /* MCUBOOT_IMAGE_NUMBER > 2 */
//  #error "Only MCUBOOT_IMAGE_NUMBER 1 and 2 are supported!"
// #endif /* MCUBOOT_IMAGE_NUMBER */

/* Maximum number of image sectors supported by the bootloader. */
#define MCUBOOT_MAX_IMG_SECTORS     RM_MCUBOOT_LARGEST_SECTOR / \
                                    FLASH_AREA_IMAGE_SECTOR_SIZE

/**/


/* Sector size of the embedded flash hardware */
 #define FLASH_AREA_IMAGE_SECTOR_SIZE            (BSP_FEATURE_FLASH_HP_CF_REGION1_BLOCK_SIZE) /* 32 KB */
 #define FLASH_TOTAL_SIZE                        (BSP_ROM_SIZE_BYTES)

/* Sector size of the data flash hardware */
 #define DATA_FLASH_AREA_IMAGE_SECTOR_SIZE       (BSP_FEATURE_FLASH_HP_DF_BLOCK_SIZE)         /* 64 B */

/* Sector size of the QSPI flash hardware */
 #define QSPI_FLASH_AREA_IMAGE_SECTOR_SIZE       (0x1000)                                     /* 4 KB */
 #define QSPI_FLASH_TOTAL_SIZE                   (0x800000)                                   /* 8 MB */

/* Flash layout info for BL2 bootloader */
 #define FLASH_BASE_ADDRESS                      (0x00000000)

/* Used by the flash service to make sure that the writes are only to the update areas */
 #define RM_TFM_PORT_SECURE_UPDATE_ADDRESS       FLASH_BASE_ADDRESS + RM_MCUBOOT_PORT_CFG_SCRATCH_SIZE + \
    RM_MCUBOOT_PORT_CFG_MCUBOOT_SIZE
 #define RM_TFM_PORT_SECURE_IMAGE_SIZE           RM_MCUBOOT_PORT_CFG_S_PARTITION_SIZE

 #define RM_TFM_PORT_NONSECURE_UPDATE_ADDRESS    FLASH_BASE_ADDRESS + RM_MCUBOOT_PORT_CFG_SCRATCH_SIZE + \
    RM_MCUBOOT_PORT_CFG_MCUBOOT_SIZE +                                                                   \
    (RM_MCUBOOT_PORT_CFG_S_PARTITION_SIZE * 2) + RM_MCUBOOT_PORT_CFG_NS_PARTITION_SIZE
 #define RM_TFM_PORT_NONSECURE_IMAGE_SIZE        RM_MCUBOOT_PORT_CFG_NS_PARTITION_SIZE

 #define DATA_FLASH_BASE_ADDRESS                 (BSP_FEATURE_FLASH_DATA_FLASH_START)

/* Not used, only the Non-swapping firmware upgrade operation
 * is supported. The maximum number of status entries
 * supported by the bootloader.
 */
 #define MCUBOOT_STATUS_MAX_ENTRIES              (0)

/* Internal Trusted Storage (ITS) Service definitions (4 KB) */
 #define FLASH_ITS_AREA_OFFSET                   (BSP_FEATURE_FLASH_DATA_FLASH_START)
 #define FLASH_ITS_AREA_SIZE                     (0xFC0)

/* NV Counters definitions */
 #define FLASH_NV_COUNTERS_AREA_OFFSET           (0x08000FC0)
 #define FLASH_NV_COUNTERS_AREA_SIZE             (0x40)

/* Protected Storage (PS) Service definitions size is 20 KB. */
 #define QSPI_FLASH_BASE_ADDRESS                 (0x10000000)
 #define FLASH_PS_AREA_OFFSET                    (0x0)
 #define FLASH_PS_AREA_SIZE                      (5 * QSPI_FLASH_AREA_IMAGE_SECTOR_SIZE)

/* Flash device name used by BL2
 * Name is defined in flash driver file: Driver_Flash.c
 */
 #define FLASH_DEV_NAME                          Driver_CFLASH

/* Protected Storage (PS) Service definitions
 * Note: Further documentation of these definitions can be found in the
 * TF-M PS Integration Guide.
 */
 #define TFM_HAL_PS_FLASH_DRIVER                 Driver_QSPI_FLASH0

/* In this target the CMSIS driver requires only the offset from the base
 * address instead of the full memory address.
 */

/* Base address of dedicated flash area for PS */
 #define TFM_HAL_PS_FLASH_AREA_ADDR              FLASH_PS_AREA_OFFSET

/* Size of dedicated flash area for PS */
 #define TFM_HAL_PS_FLASH_AREA_SIZE              FLASH_PS_AREA_SIZE
 #define PS_SECTOR_SIZE                          QSPI_FLASH_AREA_IMAGE_SECTOR_SIZE

/* Number of PS_SECTOR_SIZE per block */
 #define TFM_HAL_PS_SECTORS_PER_BLOCK            (0x1)

/* Specifies the smallest flash programmable unit in bytes */
 #define TFM_HAL_PS_PROGRAM_UNIT                 (0x1)

/* The maximum asset size to be stored in the PS area */
 #define PS_MAX_ASSET_SIZE                       (2048)

/* The maximum number of assets to be stored in the PS area */
 #define PS_NUM_ASSETS                           (10)

/* Internal Trusted Storage (ITS) Service definitions
 * Note: Further documentation of these definitions can be found in the
 * TF-M ITS Integration Guide. The ITS should be in the internal flash, but is
 * allocated in the external flash just for development platforms that don't
 * have internal flash available.
 */
 #define ITS_FLASH_DEV_NAME                      Driver_DFLASH
 #define TFM_HAL_ITS_FLASH_DRIVER                Driver_DFLASH

/* In this target the CMSIS driver requires only the offset from the base
 * address instead of the full memory address.
 */

/* Base address of dedicated flash area for ITS */
 #define TFM_HAL_ITS_FLASH_AREA_ADDR             FLASH_ITS_AREA_OFFSET

/* Size of dedicated flash area for ITS */
 #define TFM_HAL_ITS_FLASH_AREA_SIZE             FLASH_ITS_AREA_SIZE
 #define ITS_SECTOR_SIZE                         DATA_FLASH_AREA_IMAGE_SECTOR_SIZE

/* Number of ITS_SECTOR_SIZE per block */
 #define TFM_HAL_ITS_SECTORS_PER_BLOCK           (8)

/* Specifies the smallest flash programmable unit in bytes */
 #define TFM_HAL_ITS_PROGRAM_UNIT                (BSP_FEATURE_FLASH_HP_DF_WRITE_SIZE)

/* The maximum asset size to be stored in the ITS area */
 #define ITS_MAX_ASSET_SIZE                      (512)

/* The maximum number of assets to be stored in the ITS area */
 #define ITS_NUM_ASSETS                          (4)

/* NV Counters definitions */
 #define TFM_NV_COUNTERS_AREA_ADDR               FLASH_NV_COUNTERS_AREA_OFFSET
 #define TFM_NV_COUNTERS_AREA_SIZE               (FLASH_NV_COUNTERS_AREA_SIZE)
 #define TFM_NV_COUNTERS_SECTOR_ADDR             FLASH_NV_COUNTERS_AREA_OFFSET
 #define TFM_NV_COUNTERS_SECTOR_SIZE             FLASH_NV_COUNTERS_AREA_SIZE

/* Use eFlash 0 memory to store Code data */
 #define S_ROM_ALIAS_BASE                        (0x00000000)
 #define NS_ROM_ALIAS_BASE                       (0x00000000)

 #define S_RAM_ALIAS_BASE                        (0x20000000)

 #define TOTAL_ROM_SIZE                          FLASH_TOTAL_SIZE
 #define TOTAL_RAM_SIZE                          (BSP_RAM_SIZE_BYTES)


 #if (MCUBOOT_IMAGE_NUMBER == 1)

/* Secure + Non-Secure image primary slot */
 #ifndef FLASH_AREA_0_ID
  #define FLASH_AREA_0_ID               (FLASH_AREA_IMAGE_0)
 #endif
 #ifndef FLASH_AREA_0_OFFSET
  #define FLASH_AREA_0_OFFSET           (FLASH_AREA_MCUBOOT_OFFSET + RM_MCUBOOT_PORT_CFG_MCUBOOT_SIZE)
 #endif
 #ifndef FLASH_AREA_0_SIZE
  #define FLASH_AREA_0_SIZE             (RM_MCUBOOT_PORT_CFG_PARTITION_SIZE)
 #endif

/* Secure + Non-secure secondary slot */
 #ifndef FLASH_AREA_2_ID
  #define FLASH_AREA_2_ID               (FLASH_AREA_IMAGE_1)
 #endif
 #ifndef FLASH_AREA_2_OFFSET
  #ifdef RM_MCUBOOT_PORT_CFG_SECONDARY_USE_QSPI
   #define FLASH_AREA_2_OFFSET           (BSP_FEATURE_QSPI_DEVICE_START_ADDRESS + QSPI_AREA_MCUBOOT_OFFSET)
  #elif RM_MCUBOOT_DUAL_BANK_ENABLED
  #define FLASH_AREA_2_OFFSET           (BSP_FEATURE_FLASH_HP_CF_DUAL_BANK_START + FLASH_AREA_MCUBOOT_OFFSET + \
                                         RM_MCUBOOT_PORT_CFG_MCUBOOT_SIZE)
  #else
   #define FLASH_AREA_2_OFFSET           (FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE)
  #endif
 #endif
 #ifndef FLASH_AREA_2_SIZE
  #define FLASH_AREA_2_SIZE             (RM_MCUBOOT_PORT_CFG_PARTITION_SIZE)
 #endif

/* Swap space.  */
 #ifndef FLASH_AREA_SCRATCH_OFFSET
  #ifdef RM_MCUBOOT_PORT_CFG_SECONDARY_USE_QSPI
   #define FLASH_AREA_SCRATCH_OFFSET     (FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE)
  #else
   #define FLASH_AREA_SCRATCH_OFFSET     (FLASH_AREA_2_OFFSET + FLASH_AREA_2_SIZE)
  #endif
 #endif
 #ifndef FLASH_AREA_SCRATCH_SIZE
  #define FLASH_AREA_SCRATCH_SIZE       (RM_MCUBOOT_PORT_CFG_SCRATCH_SIZE)
 #endif

#elif (MCUBOOT_IMAGE_NUMBER == 2)

/* Swap space.  */
 #ifndef FLASH_AREA_SCRATCH_OFFSET
  #define FLASH_AREA_SCRATCH_OFFSET     (FLASH_AREA_MCUBOOT_OFFSET + RM_MCUBOOT_PORT_CFG_MCUBOOT_SIZE)
 #endif
 #ifndef FLASH_AREA_SCRATCH_SIZE
  #define FLASH_AREA_SCRATCH_SIZE       (RM_MCUBOOT_PORT_CFG_SCRATCH_SIZE)
 #endif

/* Secure image secondary slot */
 #ifndef FLASH_AREA_2_ID
  #define FLASH_AREA_2_ID               (FLASH_AREA_IMAGE_2)
 #endif
 #ifndef FLASH_AREA_2_OFFSET
  #ifdef RM_MCUBOOT_PORT_CFG_SECONDARY_USE_QSPI
   #define FLASH_AREA_2_OFFSET           (BSP_FEATURE_QSPI_DEVICE_START_ADDRESS + QSPI_AREA_MCUBOOT_OFFSET)
   #else
   #define FLASH_AREA_2_OFFSET           (FLASH_AREA_SCRATCH_OFFSET + FLASH_AREA_SCRATCH_SIZE)
  #endif
 #endif
 #ifndef FLASH_AREA_2_SIZE
  #define FLASH_AREA_2_SIZE             (RM_MCUBOOT_PORT_CFG_S_PARTITION_SIZE)
 #endif

/* Secure image primary slot */
 #ifndef FLASH_AREA_0_ID
  #define FLASH_AREA_0_ID               (FLASH_AREA_IMAGE_0)
 #endif
 #ifndef FLASH_AREA_0_OFFSET
  #define FLASH_AREA_0_OFFSET           (FLASH_AREA_2_OFFSET + FLASH_AREA_2_SIZE)
 #endif
 #ifndef FLASH_AREA_0_SIZE
  #define FLASH_AREA_0_SIZE             (RM_MCUBOOT_PORT_CFG_S_PARTITION_SIZE)
 #endif

/* Non-Secure image primary slot */
 #ifndef FLASH_AREA_1_ID
  #define FLASH_AREA_1_ID               (FLASH_AREA_IMAGE_1)
 #endif
 #ifndef FLASH_AREA_1_OFFSET
  #define FLASH_AREA_1_OFFSET           (FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE)
 #endif
 #ifndef FLASH_AREA_1_SIZE
  #define FLASH_AREA_1_SIZE             (RM_MCUBOOT_PORT_CFG_NS_PARTITION_SIZE)
 #endif

/* Non-Secure image secondary slot */
 #ifndef FLASH_AREA_3_ID
  #define FLASH_AREA_3_ID               (FLASH_AREA_IMAGE_3)
 #endif
 #ifndef FLASH_AREA_3_OFFSET
   #ifdef RM_MCUBOOT_PORT_CFG_SECONDARY_USE_QSPI
   #define FLASH_AREA_3_OFFSET           (BSP_FEATURE_QSPI_DEVICE_START_ADDRESS + QSPI_AREA_MCUBOOT_OFFSET + FLASH_AREA_2_SIZE)
   #else
   #define FLASH_AREA_3_OFFSET           (FLASH_AREA_1_OFFSET + FLASH_AREA_1_SIZE)
  #endif
 #endif
 #ifndef FLASH_AREA_3_SIZE
  #define FLASH_AREA_3_SIZE             (RM_MCUBOOT_PORT_CFG_NS_PARTITION_SIZE)
 #endif

 #else
 #endif

 #ifdef __cplusplus
}
 #endif

#endif                                 /* __FLASH_LAYOUT_H__ */
