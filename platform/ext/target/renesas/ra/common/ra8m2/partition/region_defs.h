/*
 * Copyright (c) 2017-2022 Arm Limited. All rights reserved.
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

#ifndef __REGION_DEFS_H__
#define __REGION_DEFS_H__

#include "flash_layout.h"

#define BL2_HEAP_SIZE      (0x0) /* Currently not support BL2 */
#define BL2_MSP_STACK_SIZE (0x0) /* Currently not support BL2 */

#ifdef ENABLE_HEAP
#define S_HEAP_SIZE (0x00000200)
#else
#define S_HEAP_SIZE (0x00000000)
#endif

#ifdef TFM_FIH_PROFILE_ON
#define S_MSP_STACK_SIZE (0x00000A40)
#else
#define S_MSP_STACK_SIZE (0x00000800)
#endif
#define S_PSP_STACK_SIZE (0x00000800)

#define NS_HEAP_SIZE  (0x00001000)
#define NS_STACK_SIZE (0x000001E0)

/* Alignment of partitions to the MRAM security-attribution granularity (32 KB)
 * and SRAM security-attribution granularity (8 KB) is enforced at configure
 * time by check_config.cmake, not by this header.
 */

#ifdef BL2
#ifndef LINK_TO_SECONDARY_PARTITION
#define S_IMAGE_PRIMARY_PARTITION_OFFSET   (FLASH_AREA_0_OFFSET)
#define S_IMAGE_SECONDARY_PARTITION_OFFSET (FLASH_AREA_2_OFFSET)
#else
#define S_IMAGE_PRIMARY_PARTITION_OFFSET   (FLASH_AREA_2_OFFSET)
#define S_IMAGE_SECONDARY_PARTITION_OFFSET (FLASH_AREA_0_OFFSET)
#endif /* !LINK_TO_SECONDARY_PARTITION */
#else
#define S_IMAGE_PRIMARY_PARTITION_OFFSET (0x0)
#endif /* BL2 */

/* CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET is injected by
 * common/core/post_config.cmake (default: right after the S image and storage
 * partition in MRAM) so the NS image can be relocated off-chip independently
 * of the S image chain, e.g. onto the EK-RA8M2's OSPI0 CS0 external flash via
 * CPU0_NS_ROM_ALIAS_BASE.
 */
#ifndef CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET
#error "CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET must be defined by common/core/post_config.cmake"
#endif

/* Boot partition structure if MCUBoot is used:
 * 0x0_0000 Bootloader header
 * 0x0_0400 Image area
 * 0x7_0000 Trailer
 */
/* IMAGE_CODE_SIZE is the space available for the software binary image.
 * It is less than the FLASH_S_PARTITION_SIZE + FLASH_NS_PARTITION_SIZE
 * because we reserve space for the image header and trailer introduced
 * by the bootloader.
 */

#if (!defined(MCUBOOT_IMAGE_NUMBER) || (MCUBOOT_IMAGE_NUMBER == 1)) &&                             \
	(CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET > S_IMAGE_PRIMARY_PARTITION_OFFSET)
/* If secure image and nonsecure image are concatenated, and nonsecure image
 * locates at the higher memory range, then the secure image does not need
 * the trailer area.
 */
#define IMAGE_S_CODE_SIZE (FLASH_S_PARTITION_SIZE - BL2_HEADER_SIZE)
#else
#define IMAGE_S_CODE_SIZE (FLASH_S_PARTITION_SIZE - BL2_HEADER_SIZE - BL2_TRAILER_SIZE)
#endif

/* Alias definitions for secure areas*/
#define S_ROM_ALIAS(x)  (S_ROM_ALIAS_BASE + (x))
#define S_RAM_ALIAS(x)  (S_RAM_ALIAS_BASE + (x))

/* Secure regions */
#define S_IMAGE_PRIMARY_AREA_OFFSET (S_IMAGE_PRIMARY_PARTITION_OFFSET + BL2_HEADER_SIZE)
#define S_CODE_START                (S_ROM_ALIAS(S_IMAGE_PRIMARY_AREA_OFFSET))
#define S_CODE_SIZE                 (IMAGE_S_CODE_SIZE)
#define S_CODE_LIMIT                (S_CODE_START + S_CODE_SIZE - 1)

#define TFM_LINKER_VENEERS_LOCATION_END
/* The veneer region sits at the end of the secure image partition. Must match
 * FLASH_CPU0_C_START/SIZE exported to the FSP by fsp_post_config.cmake.
 */
#ifndef CMSE_VENEER_REGION_SIZE
#error "CMSE_VENEER_REGION_SIZE must be defined by common/core/common.cmake"
#endif
#define TFM_LINKER_VENEERS_SIZE             (CMSE_VENEER_REGION_SIZE)
#define TFM_LINKER_VENEERS_START            (S_ROM_ALIAS(FLASH_S_PARTITION_SIZE) - TFM_LINKER_VENEERS_SIZE)

/* Size of vector table: 96 interrupt handlers + 16 ARM exceptions + 4 bytes MPS initial value */
#define S_CODE_VECTOR_TABLE_SIZE (0x1c4)

#ifndef S_DATA_SIZE
#error "S_DATA_SIZE must be defined by config.cmake"
#endif

#define S_DATA_START (S_RAM_ALIAS(0x0))
#define S_DATA_LIMIT (S_DATA_START + S_DATA_SIZE - 1)

#ifdef S_RAM_CODE_SIZE
/* RAM-resident MRAM code (r_mram .ram_from_flash: R_MRAM_Write and the
 * write/erase helpers) must execute from SRAM: MRAM cannot be read while it
 * is being programmed. The region is carved from the top of the SECURE SRAM
 * (the S_DATA_SIZE window; the NSC SRAM carve-out is unsupported and fixed at 0,
 * see RAM_CPU0_C_SIZE in s_ns_memory.cmake).
 * S_RAM_CODE_SIZE is configured via the board config.cmake. If caches are
 * ever enabled, the boot-time copy must flush/invalidate (currently both
 * D/I-Cache are disabled, BSP_CFG_DCACHE_ENABLED=0).
 */
#define S_RAM_CODE_START              (S_DATA_START + S_DATA_SIZE - S_RAM_CODE_SIZE)
#define S_RAM_CODE_EXTRA_SECTION_NAME ".ram_from_flash"
#endif

/* Non-secure regions, derived from the TF-M memory map (config.cmake /
 * post_config.cmake), independent of the FSP partition macros.
 */
#define CPU0_NS_CODE_PARTITION_START (CPU0_NS_ROM_ALIAS_BASE + CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET)
#define CPU1_NS_CODE_PARTITION_START (CPU1_NS_ROM_ALIAS_BASE + CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET)
#define CPU0_NS_DATA_START           (CPU0_NS_RAM_ALIAS_BASE + CPU0_NS_RAM_OFFSET)
#define CPU1_NS_DATA_START           (CPU1_NS_RAM_ALIAS_BASE + CPU1_NS_RAM_OFFSET)

#if (RA_CORE == RA_CM33) /* CPU1 remote NS (Mailbox Agent) */
#define NS_CODE_START                 (CPU1_NS_CODE_PARTITION_START + BL2_HEADER_SIZE)
#define NS_CODE_SIZE                  (FLASH_CPU1_NS_PARTITION_SIZE - BL2_HEADER_SIZE - BL2_TRAILER_SIZE)
#define NS_DATA_START                 (CPU1_NS_DATA_START)
#define NS_DATA_SIZE                  (CPU1_NS_RAM_DATA_SIZE)
#elif (RA_CORE == RA_CM85) /* CPU0 local NS (TrustZone) */
#define NS_CODE_START                 (CPU0_NS_CODE_PARTITION_START + BL2_HEADER_SIZE)
#define NS_CODE_SIZE                  (FLASH_CPU0_NS_PARTITION_SIZE - BL2_HEADER_SIZE - BL2_TRAILER_SIZE)
#define NS_DATA_START                 (CPU0_NS_DATA_START)
#define NS_DATA_SIZE                  (CPU0_NS_RAM_DATA_SIZE)
#endif /* RA_CORE */

#if defined(PSA_API_TEST_NS) && !defined(PSA_API_TEST_IPC)
#define DEV_APIS_TEST_NVMEM_REGION_SIZE 0x400
/* Shrink the injected NS_DATA_SIZE by the PSA API test's reserved NVMEM
 * region; the linker scripts consume NS_DATA_SIZE directly for the RAM
 * region length, so it must reflect the reduced value in this configuration.
 */
#define NS_DATA_SIZE_INJECTED NS_DATA_SIZE
#undef NS_DATA_SIZE
#define NS_DATA_SIZE (NS_DATA_SIZE_INJECTED - DEV_APIS_TEST_NVMEM_REGION_SIZE)
#endif

#define NS_CODE_LIMIT (NS_CODE_START + NS_CODE_SIZE - 1)
#define NS_DATA_LIMIT (NS_DATA_START + NS_DATA_SIZE - 1)

/* NS partition information is used for MPC and SAU configuration */
#define NS_PARTITION_START (CPU0_NS_ROM_ALIAS_BASE + CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET)
#define NS_PARTITION_SIZE  (FLASH_CPU0_NS_PARTITION_SIZE + FLASH_CPU1_NS_PARTITION_SIZE)

/* TODO: Add support for CPU1 partitions when BL2 is available */
/* Secondary partition for new images in case of firmware upgrade */
#define SECONDARY_PARTITION_START (CPU0_NS_ROM_ALIAS_BASE + S_IMAGE_SECONDARY_PARTITION_OFFSET)
#define SECONDARY_PARTITION_SIZE  (FLASH_S_PARTITION_SIZE + FLASH_CPU0_NS_PARTITION_SIZE)

#ifdef BL2
/* Bootloader regions */
#define BL2_CODE_START (S_ROM_ALIAS(FLASH_AREA_BL2_OFFSET))
#define BL2_CODE_SIZE  (FLASH_AREA_BL2_SIZE)
#define BL2_CODE_LIMIT (BL2_CODE_START + BL2_CODE_SIZE - 1)

#define BL2_DATA_START (S_RAM_ALIAS(0x0))
#define BL2_DATA_SIZE  (TOTAL_RAM_SIZE)
#define BL2_DATA_LIMIT (BL2_DATA_START + BL2_DATA_SIZE - 1)
#endif /* BL2 */

/* Shared symbol area between bootloader and runtime firmware. Global variables
 * in the shared code can be placed here.
 */
#ifdef CODE_SHARING
#define SHARED_SYMBOL_AREA_BASE S_RAM_ALIAS_BASE
#define SHARED_SYMBOL_AREA_SIZE 0x20
#else
#define SHARED_SYMBOL_AREA_BASE S_RAM_ALIAS_BASE
#define SHARED_SYMBOL_AREA_SIZE 0x0
#endif /* CODE_SHARING */

/* Shared data area between bootloader and runtime firmware.
 * These areas are allocated at the beginning of the RAM, it is overlapping
 * with TF-M Secure code's MSP stack
 */
#define BOOT_TFM_SHARED_DATA_BASE     (SHARED_SYMBOL_AREA_BASE + SHARED_SYMBOL_AREA_SIZE)
#define BOOT_TFM_SHARED_DATA_SIZE     (0x400)
#define BOOT_TFM_SHARED_DATA_LIMIT    (BOOT_TFM_SHARED_DATA_BASE + BOOT_TFM_SHARED_DATA_SIZE - 1)
#define SHARED_BOOT_MEASUREMENT_BASE  BOOT_TFM_SHARED_DATA_BASE
#define SHARED_BOOT_MEASUREMENT_SIZE  BOOT_TFM_SHARED_DATA_SIZE
#define SHARED_BOOT_MEASUREMENT_LIMIT BOOT_TFM_SHARED_DATA_LIMIT

#endif /* __REGION_DEFS_H__ */
