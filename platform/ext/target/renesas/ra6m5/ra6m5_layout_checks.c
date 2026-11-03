/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Build-time guards on the flash layout derived from the e2 solution.
 *
 * Every check is a static assertion; the file emits no code. Each guards a defect that has
 * shipped once and was invisible until it ran on hardware.
 */

#include "config_tfm.h"
#include "flash_otp_nv_counters_backend.h"
#include "flash_layout.h"
#include "region_defs.h"
/* bsp_api.h, not bsp_cfg.h: bsp_cfg.h pulls in the board headers, which use FSP_HEADER before
 * it is defined. bsp_api.h includes them in the right order. */
#include "bsp_api.h"

/* ---------------------------------------------------------------------------------
 * Vector table size
 *
 * startup_ra6m5.c sizes __VECTOR_TABLE from its own RA6M5_VECTOR_TABLE_ENTRIES literal (112)
 * and deliberately includes only CMSIS headers, so it cannot check itself against the
 * generated BSP config. This file can. The invariant that matters is that the table fits its
 * linker region, not the literal itself.
 *
 * A device change that grows the ICU overflows .TFM_VECTORS into the section after it. The
 * RA8M2 port once shipped RA6M4's 496-entry table (0x7C0) and hit this (DECISIONS D077).
 * --------------------------------------------------------------------------------- */
_Static_assert(S_CODE_VECTOR_TABLE_SIZE >= (BSP_VECTOR_TABLE_MAX_ENTRIES * 4),
               "RA6M5: S_CODE_VECTOR_TABLE_SIZE in region_defs.h is smaller than the "
               "vector table the configured device needs. .TFM_VECTORS would overflow "
               "into the section after it. Also update RA6M5_VECTOR_TABLE_ENTRIES in "
               "startup_ra6m5.c, which must match BSP_VECTOR_TABLE_MAX_ENTRIES.");

/* ---------------------------------------------------------------------------------
 * Main stack
 *
 * bsp_init_stub.c aliases FSP's g_main_stack onto Image$$ARM_LIB_STACK$$ZI$$Base, so FSP's
 * SystemInit() sets MSPLIM to the base of the running stack. SystemInit() also writes its
 * stack seal at &g_main_stack[BSP_CFG_STACK_MAIN_BYTES]. The generated scatter file sizes
 * .msp_stack as S_MSP_STACK_SIZE - STACKSEAL_SIZE and reserves STACKSEAL_SIZE above it, so the
 * two agree only when S_MSP_STACK_SIZE is BSP_CFG_STACK_MAIN_BYTES + STACKSEAL_SIZE. A
 * regenerated solution with a different FSP main stack size moves that write off the seal and
 * into whatever the linker put above the stack.
 * --------------------------------------------------------------------------------- */
#ifndef STACKSEAL_SIZE
#define STACKSEAL_SIZE (8)
#endif

/* ---------------------------------------------------------------------------------
 * BSP_CFG_EARLY_INIT must be 1 in the secure e2 project (DESIGN.md §8.1).
 *
 * TF-M's Reset_Handler calls SystemInit() before the C runtime zeroes .bss. With early init
 * off, FSP keeps SystemCoreClock and its other clock state in ordinary .bss, so the value
 * SystemInit() computed is wiped. R_FLASH_HP_Open() then derives FCLK = 0 and fails with
 * FSP_ERR_FCLK. Early init moves that state into .ram_noinit, which the port's linker scripts
 * keep out of .bss.
 *
 * The linker half of this fix is in the port; this half is in the e2 project and does not
 * carry over (E2_PROJECT_REQUIREMENTS.md). bl2_option_setting.c carries the same check for
 * BL2.
 * --------------------------------------------------------------------------------- */
#if !defined(BSP_CFG_EARLY_INIT) || !(BSP_CFG_EARLY_INIT)
#error "RA6M5: BSP_CFG_EARLY_INIT is 0 in the secure e2 project. Set BSP > Early BSP Initialization to Enabled, regenerate and rebuild in e2 - otherwise SystemCoreClock is zeroed after SystemInit() and R_FLASH_HP_Open() fails with FSP_ERR_FCLK. See DESIGN.md 8.1."
#endif

_Static_assert(S_MSP_STACK_SIZE == (BSP_CFG_STACK_MAIN_BYTES + STACKSEAL_SIZE),
               "RA6M5: S_MSP_STACK_SIZE must be BSP_CFG_STACK_MAIN_BYTES + STACKSEAL_SIZE. "
               "FSP's SystemInit() writes its stack seal at "
               "&g_main_stack[BSP_CFG_STACK_MAIN_BYTES], which bsp_init_stub.c aliases to "
               "the TF-M stack base - so it must land on the reserved __StackSeal, not "
               "past the top of the stack. Update S_MSP_STACK_SIZE in region_defs.h to "
               "match the regenerated bsp_cfg.h.");

/* ---------------------------------------------------------------------------------
 * MCUboot slot geometry
 *
 * Each slot must be contiguous (span == sum of its components) and a whole number of erase
 * sectors. Either fault appears only on hardware: boot_read_sectors() returns BOOT_EFLASH, or
 * the signed image and the bootloader disagree about where the trailer magic lives. See
 * flash_layout.h, TFM_SLOT_SPAN.
 * --------------------------------------------------------------------------------- */
#define RA6M5_SLOT_SUM_0 (BSP_PARTITION___BL_0_P_H_SIZE + BSP_PARTITION_FLASH_CPU0_S_SIZE + \
                          BSP_PARTITION_FLASH_CPU0_C_SIZE + BSP_PARTITION___BL_0_P_T_SIZE)
#define RA6M5_SLOT_SUM_2 (BSP_PARTITION___BL_0_S_H_SIZE + BSP_PARTITION___BL_0_S_I_SIZE + \
                          BSP_PARTITION___BL_0_S_T_SIZE)
#define RA6M5_SLOT_SUM_1 (BSP_PARTITION___BL_1_P_H_SIZE + BSP_PARTITION_FLASH_CPU0_N_SIZE + \
                          BSP_PARTITION___BL_1_P_T_SIZE)
#define RA6M5_SLOT_SUM_3 (BSP_PARTITION___BL_1_S_H_SIZE + BSP_PARTITION___BL_1_S_I_SIZE + \
                          BSP_PARTITION___BL_1_S_T_SIZE)

#define RA6M5_CHECK_SLOT(n)                                                              \
    _Static_assert((FLASH_AREA_##n##_OFFSET % FLASH_AREA_IMAGE_SECTOR_SIZE) == 0,        \
                   "RA6M5: slot " #n " does not start on an erase-sector boundary. "     \
                   "boot_read_sectors() would erase from the wrong address on upgrade."); \
    _Static_assert((FLASH_AREA_##n##_SIZE % FLASH_AREA_IMAGE_SECTOR_SIZE) == 0,           \
                   "RA6M5: slot " #n " is not a whole number of erase sectors. "          \
                   "flash_area_get_sectors() rejects the remainder and BL2 fails with "   \
                   "BOOT_EFLASH. Fix the partitioning in the e2 solution.");               \
    _Static_assert(FLASH_AREA_##n##_SIZE == RA6M5_SLOT_SUM_##n,                            \
                   "RA6M5: slot " #n " has a gap between its components. The span and "    \
                   "the sum disagree, so imgtool and bootutil would place the trailer "    \
                   "magic differently. Make the solution partitions contiguous.")

RA6M5_CHECK_SLOT(0);
RA6M5_CHECK_SLOT(1);
RA6M5_CHECK_SLOT(2);
RA6M5_CHECK_SLOT(3);

/* A secondary slot holds a copy of the whole primary image. The primary describes that image
 * as the solution's application regions, two for the secure image because the code and the
 * NSC veneers carry different TrustZone attributes; the secondary is a single I region. The
 * generator schema requires equal sizes (linker_macros.j2: "Secondary images must be same
 * size as the area allocated for the project"), or an upgrade overruns or wastes the slot. */
_Static_assert(BSP_PARTITION___BL_0_S_I_SIZE ==
                   (BSP_PARTITION_FLASH_CPU0_S_SIZE + BSP_PARTITION_FLASH_CPU0_C_SIZE),
               "RA6M5: image 0 secondary is not the size of the primary image "
               "(FLASH_CPU0_S + FLASH_CPU0_C).");
_Static_assert(BSP_PARTITION___BL_1_S_I_SIZE == BSP_PARTITION_FLASH_CPU0_N_SIZE,
               "RA6M5: image 1 secondary is not the size of the primary image "
               "(FLASH_CPU0_N).");

/* ---------------------------------------------------------------------------------
 * OTP / NV counter area size.
 *
 * flash_otp_nv_counters_backend.c opens init_otp_nv_counters_flash() with
 *
 *     if ((TFM_OTP_NV_COUNTERS_AREA_SIZE) < sizeof(struct flash_otp_nv_counters_region_t))
 *         return TFM_PLAT_ERR_SYSTEM_ERR;
 *
 * Both operands are compile-time constants, so when the area is too small the compiler folds
 * the test and the function becomes an unconditional error return that never touches flash.
 * Nothing warns and the image links; on hardware OTP, ITS and PS all fail to initialise.
 *
 * The struct grows with MCUBOOT_BUILTIN_KEY, PLATFORM_NS_NV_COUNTERS and FLASH_NV_COUNTER_AM,
 * none of which this port controls, so the size is checked rather than hard-coded. If this
 * fires, give the secure DATA_FLASH partition more room in the e2 solution and raise
 * TFM_NV_COUNTERS_AREA_SIZE in flash_layout.h; do not shrink the services to fit.
 */

#if !defined(TFM_OTP_NV_COUNTERS_AREA_SIZE)
#error "RA6M5: TFM_OTP_NV_COUNTERS_AREA_SIZE is not defined - check flash_layout.h"
#endif

/* The backend keeps a primary copy and a backup mirror, each of this size. */
_Static_assert(TFM_OTP_NV_COUNTERS_AREA_SIZE >=
                   sizeof(struct flash_otp_nv_counters_region_t),
               "RA6M5: TFM_OTP_NV_COUNTERS_AREA_SIZE is smaller than "
               "sizeof(struct flash_otp_nv_counters_region_t). "
               "init_otp_nv_counters_flash() would compile to an unconditional failure. "
               "Raise TFM_NV_COUNTERS_AREA_SIZE in flash_layout.h - it is halved to give "
               "the area and its backup mirror.");

/* Both halves must be whole erase sectors: the backend erases with EraseSector() and
 * rejects a start address that is not sector aligned. */
_Static_assert((TFM_OTP_NV_COUNTERS_AREA_SIZE % TFM_OTP_NV_COUNTERS_SECTOR_SIZE) == 0,
               "RA6M5: the OTP area must be a whole number of erase sectors.");

/*
 * The combined S+NS image (tfm_s_ns_signed.bin, which TF-M's NSPE rules build for any NS
 * application that does not exclude the target) is a plain concatenation, with
 * NON_SECURE_IMAGE_OFFSET marking where the second half begins. It describes the flash only
 * while the two primary slots are adjacent. A repartition that leaves a gap would produce a
 * combined image that flashes but puts the non-secure half short of its slot. See
 * flash_layout.h, NON_SECURE_IMAGE_OFFSET.
 */
_Static_assert(FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE == FLASH_AREA_1_OFFSET,
               "RA6M5: the secure and non-secure PRIMARY slots are no longer adjacent, so "
               "a concatenated tfm_s_ns_signed.bin would place the non-secure image at the "
               "wrong address. Restore adjacency in the solution, or stop building the "
               "combined image (see NON_SECURE_IMAGE_OFFSET in flash_layout.h).");

/* ---------------------------------------------------------------------------------
 * Protected Storage capacity
 *
 * With num_blocks == 2 the flash FS keeps its metadata and logical data block 0 in the same
 * physical block, so every live PS object must fit in one block together with the metadata.
 * The per-object ceiling in config_tfm_target.h is not the binding constraint; the sum is,
 * and fixed overhead that scales with PS_NUM_ASSETS dominates it.
 *
 * Sizes are from the flash FS and PS structs (its_flash_fs_mblock.h, ps_object_defs.h,
 * ps_object_table.c) at this port's settings: PS_ENCRYPTION on, PS_ROLLBACK_PROTECTION on,
 * PS_AES_KEY_USAGE_LIMIT 0, 4-byte program unit. Verified against the built objects.
 * --------------------------------------------------------------------------------- */
#define RA6M5_PS_BLOCK_SIZE     (TFM_HAL_PS_SECTOR_SIZE * TFM_HAL_PS_SECTORS_PER_BLOCK)
#define RA6M5_PS_NUM_BLOCKS     (TFM_HAL_PS_FLASH_AREA_SIZE / RA6M5_PS_BLOCK_SIZE)

/* Block metadata header + one block_meta + one file_meta per file. */
#define RA6M5_PS_FS_METADATA    (8 + 12 + ((PS_NUM_ASSETS + 3) * 32))
/* sizeof(struct ps_obj_table_t), and both FS IDs are always resident. */
#define RA6M5_PS_TABLES         (2 * (48 + ((PS_NUM_ASSETS + 1) * 32)))
/* sizeof(struct ps_obj_header_t) + PS_TAG_LEN_BYTES, added to every asset. */
#define RA6M5_PS_OBJ_OVERHEAD   (56 + 16)
/* One small asset assumed permanently resident. tf-m-tests' TFM_NS_PS_TEST_1004 creates a
 * 36-byte PSA_STORAGE_FLAG_WRITE_ONCE asset that by definition can never be removed. */
#define RA6M5_PS_RESIDENT       (36 + RA6M5_PS_OBJ_OVERHEAD)

_Static_assert(RA6M5_PS_NUM_BLOCKS == 2,
               "RA6M5: the PS budget below assumes the two-block filesystem, where data "
               "block 0 shares the metadata block. Rework it if the PS area is resized to "
               "give the FS dedicated data blocks.");

_Static_assert((RA6M5_PS_BLOCK_SIZE - RA6M5_PS_FS_METADATA - RA6M5_PS_TABLES
                - RA6M5_PS_RESIDENT) >= (PS_MAX_ASSET_SIZE + RA6M5_PS_OBJ_OVERHEAD),
               "RA6M5: PS cannot hold one PS_MAX_ASSET_SIZE asset alongside its own "
               "metadata and object tables. Each asset costs 96 bytes of fixed overhead, "
               "so lower PS_NUM_ASSETS in config_tfm_target.h - lowering PS_MAX_ASSET_SIZE "
               "recovers far less and has a floor of its own. This fires as "
               "PSA_ERROR_INSUFFICIENT_STORAGE from psa_ps_set at run time.");
