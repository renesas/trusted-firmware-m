/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __FLASH_LAYOUT_H__
#define __FLASH_LAYOUT_H__

/*
 * RA6E1 flash layout - DERIVED, NOT HAND-MAINTAINED.
 *
 * Every address and size below comes from bsp_linker_info.h, which the e2 studio
 * SOLUTION generates for each project from solution.xml. Change the partitions in
 * the solution, rebuild the projects in e2, and this header follows with no edits.
 *
 * This inverts the ra6m4 model, where flash_layout.h was authoritative and the RASC
 * values were vestigial (DESIGN.md 3). Here RASC/e2 is the source of truth.
 *
 * bsp_linker_info.h lives in <project>/Debug/, so the solution must have been built
 * in e2 at least once before TF-M can configure. The platform CMakeLists adds that
 * directory to the include path and fails with a clear message if it is absent.
 *
 * Partition -> TF-M mapping:
 *   FLASH_BL_CPU0_S   -> BL2 (MCUboot)
 *   __BL_0_P_*        -> image 0 primary   = secure     (FLASH_AREA_0)
 *   __BL_1_P_*        -> image 1 primary   = non-secure (FLASH_AREA_1)
 *   __BL_0_S_*        -> image 0 secondary = secure     (FLASH_AREA_2)
 *   __BL_1_S_*        -> image 1 secondary = non-secure (FLASH_AREA_3)
 */

#include "bsp_linker_info.h"

/* Device geometry. Region 1 (0x10000+) erases in 32 KB blocks; every MCUboot slot
 * lives there, so that is the sector size MCUboot must use. Region 0 is 8 KB but no
 * slot boundary falls in it. Confirm against BSP_FEATURE_FLASH_HP_CF_REGION1_BLOCK_SIZE
 * if the device is ever changed. */
#define FLASH_BASE_ADDRESS              (0x00000000)
#define FLASH_TOTAL_SIZE                (0x00100000)     /* 1 MB */
#define FLASH_AREA_IMAGE_SECTOR_SIZE    (0x8000)         /* 32 KB */
#define FLASH_DEV_NAME                  Driver_FLASH0
#define TFM_HAL_FLASH_PROGRAM_UNIT      (128)            /* code flash write size */

/* BL2 */
#define FLASH_AREA_BL2_OFFSET           (BSP_PARTITION_FLASH_BL_CPU0_S_START)
#define FLASH_AREA_BL2_SIZE             (BSP_PARTITION_FLASH_BL_CPU0_S_SIZE)

/* A slot spans its header through the end of its trailer. Deriving the size this way
 * rather than assuming header+image+trailer matters: the primary secure slot carries
 * alignment padding between the NSC region and the trailer, because the trailer is the
 * first non-secure partition and that boundary must be 32 KB aligned (DESIGN.md 7.1).
 * So FLASH_AREA_0_SIZE is legitimately 0x100 larger than FLASH_AREA_2_SIZE. */
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

/* The solution is configured overwrite-only, so there is no scratch area. If the
 * upgrade mode is changed to swap-using-scratch in e2, a scratch partition must be
 * added to the solution and wired up here. */
#define FLASH_AREA_SCRATCH_ID           (FLASH_AREA_0_ID + 4)
#define FLASH_AREA_SCRATCH_OFFSET       (0)
#define FLASH_AREA_SCRATCH_SIZE         (0)
#define MCUBOOT_STATUS_MAX_ENTRIES      (0)

#define FLASH_MAX_PARTITION_SIZE        ((FLASH_AREA_0_SIZE > FLASH_AREA_1_SIZE) ? \
                                          FLASH_AREA_0_SIZE : FLASH_AREA_1_SIZE)

/* Image sizes seen by imgtool/bootutil (slot minus header and trailer). */
#define IMAGE_S_CODE_SIZE               (BSP_PARTITION_FLASH_CPU0_S_SIZE + \
                                         BSP_PARTITION_FLASH_CPU0_C_SIZE)
#define IMAGE_NS_CODE_SIZE              (BSP_PARTITION_FLASH_CPU0_N_SIZE)

/*
 * Data flash: ITS, PS and the MCUboot NV counters all live in the SECURE data flash
 * partition. The split below is proportional so it tracks a resized partition, and
 * the static assert fires if the solution ever gives secure data flash less than the
 * services need.
 */
#define FLASH_DEV_NAME_DATA             Driver_FLASH1
#define TFM_HAL_DATA_FLASH_PROGRAM_UNIT (4)
#define DATA_FLASH_SECTOR_SIZE          (64)

#define TFM_NV_COUNTERS_AREA_OFFSET     (BSP_PARTITION_DATA_FLASH_CPU0_S_START)
#define TFM_NV_COUNTERS_AREA_SIZE       (DATA_FLASH_SECTOR_SIZE * 8)     /* 512 B */

#define TFM_HAL_PS_FLASH_AREA_ADDR      (TFM_NV_COUNTERS_AREA_OFFSET + \
                                         TFM_NV_COUNTERS_AREA_SIZE)
#define TFM_HAL_PS_FLASH_AREA_SIZE      ((BSP_PARTITION_DATA_FLASH_CPU0_S_SIZE - \
                                          TFM_NV_COUNTERS_AREA_SIZE) / 2)
#define TFM_HAL_PS_SECTOR_SIZE          (DATA_FLASH_SECTOR_SIZE)
#define PS_RAM_FS_SIZE                  TFM_HAL_PS_FLASH_AREA_SIZE

#define TFM_HAL_ITS_FLASH_AREA_ADDR     (TFM_HAL_PS_FLASH_AREA_ADDR + \
                                         TFM_HAL_PS_FLASH_AREA_SIZE)
#define TFM_HAL_ITS_FLASH_AREA_SIZE     (TFM_HAL_PS_FLASH_AREA_SIZE)
#define TFM_HAL_ITS_SECTOR_SIZE         (DATA_FLASH_SECTOR_SIZE)
#define ITS_RAM_FS_SIZE                 TFM_HAL_ITS_FLASH_AREA_SIZE

#define TFM_HAL_PS_FLASH_DRIVER         Driver_FLASH1
#define TFM_HAL_ITS_FLASH_DRIVER        Driver_FLASH1
#define TFM_HAL_PS_PROGRAM_UNIT         TFM_HAL_DATA_FLASH_PROGRAM_UNIT
#define TFM_HAL_ITS_PROGRAM_UNIT        TFM_HAL_DATA_FLASH_PROGRAM_UNIT

/* Secure data flash must hold NV counters + PS + ITS with room for wear levelling.
 * The e2 solution currently allocates 4 KB (half the device's 8 KB) to secure; the
 * ra6m4 port used ~7 KB. If this fires, give the secure DATA_FLASH partition more
 * space in solution.xml rather than shrinking the services here. */
#if (BSP_PARTITION_DATA_FLASH_CPU0_S_SIZE) < 0x1000
#error "RA6E1: secure data flash partition too small for NV counters + PS + ITS"
#endif

/* Flash device IDs used by the CMSIS flash driver shim. */
#define FLASH_DEVICE_ID                 (100)
#define FLASH_DEVICE_ID_DATA            (101)
#define FLASH_DEV_NAME_0                Driver_FLASH0
#define FLASH_DEV_NAME_1                Driver_FLASH1

#endif /* __FLASH_LAYOUT_H__ */
