/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __FLASH_LAYOUT_H__
#define __FLASH_LAYOUT_H__

/*
 * RA6M5 flash layout, derived from the e2 solution. Do not edit it by hand.
 *
 * Every address and size comes from bsp_linker_info.h, which e2 studio generates for each
 * project from solution.xml. To change the layout, change the partitions in the solution and
 * rebuild the projects in e2; this header follows. RASC/e2 is the source of truth
 * (DESIGN.md §3).
 *
 * bsp_linker_info.h is in <project>/Debug/, so the solution must be built in e2 at least once
 * before TF-M can configure. The platform CMakeLists stops with a message if it is missing.
 *
 * This header includes bsp_partitions.h, which the platform CMakeLists filters from
 * bsp_linker_info.h down to its #define BSP_PARTITION_* lines. The header is also preprocessed
 * into ra6m5_bl2.ld, and a linker script cannot parse the C types and externs in
 * bsp_linker_info.h.
 *
 * Partition -> TF-M mapping:
 *   FLASH_BL_CPU0_S   -> BL2 (MCUboot)
 *   __BL_0_P_*        -> image 0 primary   = secure     (FLASH_AREA_0)
 *   __BL_1_P_*        -> image 1 primary   = non-secure (FLASH_AREA_1)
 *   __BL_0_S_*        -> image 0 secondary = secure     (FLASH_AREA_2)
 *   __BL_1_S_*        -> image 1 secondary = non-secure (FLASH_AREA_3)
 */

#include "bsp_partitions.h"

/* Device geometry. Code flash region 1 (0x10000 and up) erases in 32 KB blocks and holds every
 * MCUboot slot, so 32 KB is the MCUboot sector size. Region 0 is 8 KB, but no slot boundary
 * falls in it. Check BSP_FEATURE_FLASH_HP_CF_REGION1_BLOCK_SIZE if the device changes. */
#define FLASH_BASE_ADDRESS              (0x00000000)
#define FLASH_TOTAL_SIZE                (0x00200000)     /* 2 MB */
#define FLASH_AREA_IMAGE_SECTOR_SIZE    (0x8000)         /* 32 KB */
#define FLASH_DEV_NAME                  Driver_FLASH0
#define TFM_HAL_FLASH_PROGRAM_UNIT      (128)            /* code flash write size */

/* BL2 */
#define FLASH_AREA_BL2_OFFSET           (BSP_PARTITION_FLASH_BL_CPU0_S_START)
#define FLASH_AREA_BL2_SIZE             (BSP_PARTITION_FLASH_BL_CPU0_S_SIZE)

/* A slot spans its header through the end of its trailer.
 *
 * The solution's components must be contiguous, so that the span equals the sum of the
 * component sizes. Everything downstream assumes this: the FSP generator emits .fa_size as the
 * sum of the parts (linker_macros_bsp_h.j2), imgtool pads to FLASH_AREA_0_SIZE via
 * RE_SIGN_BIN_SIZE, and bootutil reads the trailer magic at fa_off + fa_size - 16. A gap
 * between components makes the span exceed the sum, and the two views of the slot silently
 * diverge.
 *
 * The slot must also be a whole number of FLASH_AREA_IMAGE_SECTOR_SIZE erase sectors.
 * flash_area_get_sectors() fails on a remainder, which shows only on hardware, as
 * boot_read_sectors() returning BOOT_EFLASH.
 *
 * ra6m5_layout_checks.c asserts both properties at build time. */
#define TFM_SLOT_SPAN(h, t)             (((t##_START) + (t##_SIZE)) - (h##_START))

/* Image 0 = secure */
#define FLASH_AREA_0_ID                 (1)
#define FLASH_AREA_0_OFFSET             (BSP_PARTITION___BL_0_P_H_START)
#define FLASH_AREA_0_SIZE               TFM_SLOT_SPAN(BSP_PARTITION___BL_0_P_H, \
                                                      BSP_PARTITION___BL_0_P_T)
#define FLASH_AREA_2_ID                 (FLASH_AREA_0_ID + 2)
#define FLASH_AREA_2_OFFSET             (BSP_PARTITION___BL_0_S_H_START)
#define FLASH_AREA_2_SIZE               TFM_SLOT_SPAN(BSP_PARTITION___BL_0_S_H, \
                                                      BSP_PARTITION___BL_0_S_T)

/* Image 1 = non-secure */
#define FLASH_AREA_1_ID                 (FLASH_AREA_0_ID + 1)
#define FLASH_AREA_1_OFFSET             (BSP_PARTITION___BL_1_P_H_START)
#define FLASH_AREA_1_SIZE               TFM_SLOT_SPAN(BSP_PARTITION___BL_1_P_H, \
                                                      BSP_PARTITION___BL_1_P_T)
#define FLASH_AREA_3_ID                 (FLASH_AREA_0_ID + 3)
#define FLASH_AREA_3_OFFSET             (BSP_PARTITION___BL_1_S_H_START)
#define FLASH_AREA_3_SIZE               TFM_SLOT_SPAN(BSP_PARTITION___BL_1_S_H, \
                                                      BSP_PARTITION___BL_1_S_T)

/* The solution is overwrite-only, so there is no scratch area.
 *
 * The layout reserves a block for one: UNUSED_SCRATCH, 32 KB of secure flash at 0x18000,
 * after BL2. It is deliberately not declared to MCUboot, which would only cost code in
 * overwrite-only mode. To switch to swap-using-scratch, set the mode in e2 and point these
 * three macros at that partition. A scratch area must be secure: during a swap it holds
 * fragments of the secure image, so an NS-writable scratch would leak and corrupt it. */
#define FLASH_AREA_SCRATCH_ID           (FLASH_AREA_0_ID + 4)
#define FLASH_AREA_SCRATCH_OFFSET       (0)
#define FLASH_AREA_SCRATCH_SIZE         (0)
#define MCUBOOT_STATUS_MAX_ENTRIES      (0)

#define FLASH_MAX_PARTITION_SIZE        ((FLASH_AREA_0_SIZE > FLASH_AREA_1_SIZE) ? \
                                          FLASH_AREA_0_SIZE : FLASH_AREA_1_SIZE)

/*
 * Combined S+NS image, for assemble.py.
 *
 * The two primary slots are adjacent (the secure slot ends exactly where the non-secure slot
 * begins), so a plain concatenation describes the flash, with these as offsets from the start
 * of the combined image. ra6m5_layout_checks.c asserts the adjacency, because it is a property
 * of the current partitioning, not a rule.
 *
 * Nothing on this port flashes the combined image. ns_app excludes the target: as an extra row
 * in the debug session's program list it has no address of its own and is silently
 * destructive at the wrong one. The macros stay because TF-M's NSPE rules build
 * tfm_s_ns_signed.bin for any NS application that does not exclude it, including tf-m-tests'
 * regression app. Without them assemble.py fails with
 * "NameError: name 'SECURE_IMAGE_OFFSET' is not defined".
 */
#define SECURE_IMAGE_OFFSET             (0x0)
#define SECURE_IMAGE_MAX_SIZE           FLASH_AREA_0_SIZE
#define NON_SECURE_IMAGE_OFFSET         (SECURE_IMAGE_OFFSET + SECURE_IMAGE_MAX_SIZE)
#define NON_SECURE_IMAGE_MAX_SIZE       FLASH_AREA_1_SIZE

/* Sectors bootutil must track for one image. Overwrite-only has no scratch area, so this is
 * sized from the largest slot, rounded up. */
#define MCUBOOT_MAX_IMG_SECTORS         ((FLASH_MAX_PARTITION_SIZE + \
                                          FLASH_AREA_IMAGE_SECTOR_SIZE - 1) / \
                                         FLASH_AREA_IMAGE_SECTOR_SIZE)

/* Image sizes seen by imgtool/bootutil (slot minus header and trailer). */
#define IMAGE_S_CODE_SIZE               (BSP_PARTITION_FLASH_CPU0_S_SIZE + \
                                         BSP_PARTITION_FLASH_CPU0_C_SIZE)
#define IMAGE_NS_CODE_SIZE              (BSP_PARTITION_FLASH_CPU0_N_SIZE)

/*
 * Data flash: ITS, PS and the MCUboot NV counters all live in the secure data flash
 * partition. The split is proportional so that it tracks a resized partition, and the #error
 * check below fires if the solution gives secure data flash less than the services need.
 */
#define FLASH_DEV_NAME_DATA             Driver_FLASH1
#define TFM_HAL_DATA_FLASH_PROGRAM_UNIT (4)

/* Code flash minimum write, BSP_FEATURE_FLASH_HP_CF_WRITE_SIZE. Driver_Flash.c advertises it
 * as the code-flash program_unit, and MCUboot gets it as flash_area_align(). config.cmake must
 * keep MCUBOOT_ALIGN_VAL equal to it: the images are signed with --align, and the trailer
 * geometry must match at run time. A named macro keeps the coupling greppable. */
#define TFM_HAL_CODE_FLASH_PROGRAM_UNIT (128)

/* Data flash geometry, as cmsis_drivers/Driver_Flash.c names it. The secure partition
 * starts at the device base, and the two partitions cover the whole device. */
#define FLASH_DATA_FLASH_BASE           (BSP_PARTITION_DATA_FLASH_CPU0_S_START)
#define FLASH_DATA_FLASH_SIZE           (BSP_PARTITION_DATA_FLASH_CPU0_S_SIZE + \
                                         BSP_PARTITION_DATA_FLASH_CPU0_N_SIZE)
#define FLASH_DATA_FLASH_SECTOR_SIZE    (64)

#define TFM_NV_COUNTERS_AREA_OFFSET     (BSP_PARTITION_DATA_FLASH_CPU0_S_START)

/* 2048 B = 32 sectors, split into a 1024 B area and a 1024 B backup mirror.
 *
 * The floor is sizeof(struct flash_otp_nv_counters_region_t), which must fit in each half.
 * That is 784 B for this configuration:
 *
 *     init_value 4 | OTP items 284 | BL2 ROTPKs x4 + BL2 NV counters x4 384
 *     | entropy_seed + secure_debug_pk 96 | flash_nv_counters[3] 12 | swap_count 4
 *
 * It grows with MCUBOOT_BUILTIN_KEY (ROTPKs become 68/100 B each, not a 32 B hash),
 * PLATFORM_NS_NV_COUNTERS (64 B each, currently 0) and FLASH_NV_COUNTER_AM. Below the floor,
 * init_otp_nv_counters_flash() compiles to an unconditional failure that never touches flash.
 * ra6m5_layout_checks.c ("OTP / NV counter area size") makes that a build error.
 */
#define TFM_NV_COUNTERS_AREA_SIZE       (FLASH_DATA_FLASH_SECTOR_SIZE * 32)   /* 2048 B */

/* The backend, platform/ext/common/template/flash_otp_nv_counters_backend.c, mirrors the area
 * so that a power loss mid-write is recoverable. The backup must be a separate erase sector.
 * The two halves split TFM_NV_COUNTERS_AREA_SIZE. */
#define TFM_OTP_NV_COUNTERS_SECTOR_SIZE      (FLASH_DATA_FLASH_SECTOR_SIZE)
#define TFM_OTP_NV_COUNTERS_AREA_SIZE        (TFM_NV_COUNTERS_AREA_SIZE / 2)
#define TFM_OTP_NV_COUNTERS_AREA_ADDR        (TFM_NV_COUNTERS_AREA_OFFSET)
#define TFM_OTP_NV_COUNTERS_BACKUP_AREA_ADDR (TFM_OTP_NV_COUNTERS_AREA_ADDR + \
                                              TFM_OTP_NV_COUNTERS_AREA_SIZE)
#define OTP_NV_COUNTERS_WRITE_BLOCK_SIZE     (TFM_HAL_DATA_FLASH_PROGRAM_UNIT)

#define TFM_HAL_PS_FLASH_AREA_ADDR      (TFM_NV_COUNTERS_AREA_OFFSET + \
                                         TFM_NV_COUNTERS_AREA_SIZE)
#define TFM_HAL_PS_FLASH_AREA_SIZE      ((BSP_PARTITION_DATA_FLASH_CPU0_S_SIZE - \
                                          TFM_NV_COUNTERS_AREA_SIZE) / 2)
#define TFM_HAL_PS_SECTOR_SIZE          (FLASH_DATA_FLASH_SECTOR_SIZE)
#define PS_RAM_FS_SIZE                  TFM_HAL_PS_FLASH_AREA_SIZE

/* Half the area per logical block, i.e. two blocks, which is the minimum the flash FS
 * needs to rotate between on a write. Derived rather than fixed so it still holds if
 * the secure data flash partition is resized in the solution. */
#define TFM_HAL_PS_SECTORS_PER_BLOCK    ((TFM_HAL_PS_FLASH_AREA_SIZE / \
                                          TFM_HAL_PS_SECTOR_SIZE) / 2)

#define TFM_HAL_ITS_FLASH_AREA_ADDR     (TFM_HAL_PS_FLASH_AREA_ADDR + \
                                         TFM_HAL_PS_FLASH_AREA_SIZE)
#define TFM_HAL_ITS_FLASH_AREA_SIZE     (TFM_HAL_PS_FLASH_AREA_SIZE)
#define TFM_HAL_ITS_SECTOR_SIZE         (FLASH_DATA_FLASH_SECTOR_SIZE)
#define ITS_RAM_FS_SIZE                 TFM_HAL_ITS_FLASH_AREA_SIZE
#define TFM_HAL_ITS_SECTORS_PER_BLOCK   ((TFM_HAL_ITS_FLASH_AREA_SIZE / \
                                          TFM_HAL_ITS_SECTOR_SIZE) / 2)

#define TFM_HAL_PS_FLASH_DRIVER         Driver_FLASH1
#define TFM_HAL_ITS_FLASH_DRIVER        Driver_FLASH1
#define TFM_HAL_PS_PROGRAM_UNIT         TFM_HAL_DATA_FLASH_PROGRAM_UNIT
#define TFM_HAL_ITS_PROGRAM_UNIT        TFM_HAL_DATA_FLASH_PROGRAM_UNIT

/* Secure data flash must hold the NV counters, PS and ITS with room for wear levelling. The e2
 * solution allocates all 8 KB of the device's data flash to secure. If this fires, give the
 * secure DATA_FLASH partition more space in solution.xml rather than shrinking the services. */
#if (BSP_PARTITION_DATA_FLASH_CPU0_S_SIZE) < 0x1000
#error "RA6M5: secure data flash partition too small for NV counters + PS + ITS"
#endif

/* Flash device IDs used by the CMSIS flash driver shim.
 *
 * FLASH_DEV_NAME_0 / FLASH_DEV_NAME_1 are deliberately not defined. They are per-area driver
 * overrides for a target whose images live on different devices. Both images are in code flash, so
 * bl2/src/default_flash_map.c correctly defaults them to FLASH_DEV_NAME. Defining a _0/_1
 * name without the matching FLASH_DEVICE_ID_0/_1 also trips the paired-definition check in
 * bl2/ext/mcuboot/include/target.h. */
#define FLASH_DEVICE_ID                 (100)
#define FLASH_DEVICE_ID_DATA            (101)

#endif /* __FLASH_LAYOUT_H__ */
