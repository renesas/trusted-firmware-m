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
 * SystemInit() computed is wiped. R_MRAM_Open() then derives FCLK = 0 and fails with
 * FSP_ERR_FCLK. Early init moves that state into .ram_noinit, which the port's linker scripts
 * keep out of .bss.
 *
 * The linker half of this fix is in the port; this half is in the e2 project and does not
 * carry over (E2_PROJECT_REQUIREMENTS.md). bl2_option_setting.c carries the same check for
 * BL2.
 * --------------------------------------------------------------------------------- */
#if !defined(BSP_CFG_EARLY_INIT) || !(BSP_CFG_EARLY_INIT)
#error "RA8M2: BSP_CFG_EARLY_INIT is 0 in the secure e2 project. Set BSP > Early BSP Initialization to Enabled, regenerate and rebuild in e2 - otherwise SystemCoreClock is zeroed after SystemInit() and R_MRAM_Open() fails with FSP_ERR_FCLK. See DESIGN.md 8.1."
#endif

_Static_assert(S_MSP_STACK_SIZE == (BSP_CFG_STACK_MAIN_BYTES + STACKSEAL_SIZE),
               "RA8M2: S_MSP_STACK_SIZE must be BSP_CFG_STACK_MAIN_BYTES + STACKSEAL_SIZE. "
               "FSP's SystemInit() writes its stack seal at "
               "&g_main_stack[BSP_CFG_STACK_MAIN_BYTES], which bsp_init_stub.c aliases to "
               "the TF-M stack base - so it must land on the reserved __StackSeal, not "
               "past the top of the stack. Update S_MSP_STACK_SIZE in region_defs.h to "
               "match the regenerated bsp_cfg.h.");

/* ---------------------------------------------------------------------------------
 * Vector table size
 *
 * startup_ra8m2.c sizes __VECTOR_TABLE from its own RA8M2_VECTOR_TABLE_ENTRIES literal (112)
 * and deliberately includes only CMSIS headers, so it cannot check itself against the
 * generated BSP config. This file can. The invariant that matters is that the table fits its
 * linker region, not the literal itself.
 *
 * A device change that grows the ICU overflows .TFM_VECTORS into the section after it. This
 * port once shipped RA6M4's 496-entry table (0x7C0) and overflowed into .ER_UNPRIV_CODE.
 * --------------------------------------------------------------------------------- */
_Static_assert(S_CODE_VECTOR_TABLE_SIZE >= (BSP_VECTOR_TABLE_MAX_ENTRIES * 4),
               "RA8M2: S_CODE_VECTOR_TABLE_SIZE in region_defs.h is smaller than the "
               "vector table the configured device needs. .TFM_VECTORS would overflow "
               "into the section after it. Also update RA8M2_VECTOR_TABLE_ENTRIES in "
               "startup_ra8m2.c, which must match BSP_VECTOR_TABLE_MAX_ENTRIES.");

/* ---------------------------------------------------------------------------------
 * FLASH_AREA_IMAGE_SECTOR_SIZE against FSP
 *
 * flash_layout.h states the sector size as the literal 0x8000 because it is also preprocessed
 * into ra8m2_bl2.ld. FSP hardcodes the same value in the generated mcuboot_config.h (there is
 * no BSP_FEATURE macro for the MRAM block size), and that header also defines
 * FLASH_AREA_IMAGE_SECTOR_SIZE, so it cannot be included here for comparison. The two
 * assertions below are what bsp_api.h allows.
 *
 * The 0x8000 itself is not machine-checked. After an FSP pack uprev, diff
 * ra_cfg/mcu-tools/include/mcuboot_config/mcuboot_config.h and confirm that
 * RM_MCUBOOT_MRAM_BLOCK_SIZE is still 0x8000. A wrong value is the D054 case: it was once
 * 0x1000, the slots came out 9.875 sectors long, and BL2 failed only on hardware.
 * --------------------------------------------------------------------------------- */
_Static_assert(BSP_FEATURE_MRAM_IS_AVAILABLE,
               "RA8M2: this flash_layout.h describes an MRAM part, but the configured "
               "device reports no MRAM. Every offset here is an MRAM alias offset.");

_Static_assert((FLASH_AREA_IMAGE_SECTOR_SIZE % BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES) == 0,
               "RA8M2: the erase sector is not a whole number of MRAM write units. "
               "MCUBOOT_ALIGN_VAL comes from BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES, so "
               "a trailer written at that alignment would straddle a sector boundary.");

/* ---------------------------------------------------------------------------------
 * MCUboot slot geometry
 *
 * Each slot must be contiguous (span == sum of its components) and a whole number of erase
 * sectors. Either fault appears only on hardware: boot_read_sectors() returns BOOT_EFLASH, or
 * the signed image and the bootloader disagree about where the trailer magic lives. See
 * flash_layout.h, TFM_SLOT_SPAN.
 * --------------------------------------------------------------------------------- */
#define RA8M2_SLOT_SUM_0 (BSP_PARTITION___BL_0_P_H_SIZE + BSP_PARTITION_FLASH_CPU0_S_SIZE + \
                          BSP_PARTITION_FLASH_CPU0_C_SIZE + BSP_PARTITION___BL_0_P_T_SIZE)
#define RA8M2_SLOT_SUM_2 (BSP_PARTITION___BL_0_S_H_SIZE + BSP_PARTITION___BL_0_S_I_SIZE + \
                          BSP_PARTITION___BL_0_S_T_SIZE)
#define RA8M2_SLOT_SUM_1 (BSP_PARTITION___BL_1_P_H_SIZE + BSP_PARTITION_FLASH_CPU0_N_SIZE + \
                          BSP_PARTITION___BL_1_P_T_SIZE)
#define RA8M2_SLOT_SUM_3 (BSP_PARTITION___BL_1_S_H_SIZE + BSP_PARTITION___BL_1_S_I_SIZE + \
                          BSP_PARTITION___BL_1_S_T_SIZE)

#define RA8M2_CHECK_SLOT(n)                                                              \
    _Static_assert((FLASH_AREA_##n##_OFFSET % FLASH_AREA_IMAGE_SECTOR_SIZE) == 0,        \
                   "RA8M2: slot " #n " does not start on an erase-sector boundary. "     \
                   "boot_read_sectors() would erase from the wrong address on upgrade."); \
    _Static_assert((FLASH_AREA_##n##_SIZE % FLASH_AREA_IMAGE_SECTOR_SIZE) == 0,           \
                   "RA8M2: slot " #n " is not a whole number of erase sectors. "          \
                   "flash_area_get_sectors() rejects the remainder and BL2 fails with "   \
                   "BOOT_EFLASH. Fix the partitioning in the e2 solution.");               \
    _Static_assert(FLASH_AREA_##n##_SIZE == RA8M2_SLOT_SUM_##n,                            \
                   "RA8M2: slot " #n " has a gap between its components. The span and "    \
                   "the sum disagree, so imgtool and bootutil would place the trailer "    \
                   "magic differently. Make the solution partitions contiguous.")

RA8M2_CHECK_SLOT(0);
RA8M2_CHECK_SLOT(1);
RA8M2_CHECK_SLOT(2);
RA8M2_CHECK_SLOT(3);

/* A secondary slot holds a copy of the whole primary image. The primary describes that image
 * as the solution's application regions, two for the secure image because the code and the
 * NSC veneers carry different TrustZone attributes; the secondary is a single I region. The
 * generator schema requires equal sizes (linker_macros.j2: "Secondary images must be same
 * size as the area allocated for the project"), or an upgrade overruns or wastes the slot. */
_Static_assert(BSP_PARTITION___BL_0_S_I_SIZE ==
                   (BSP_PARTITION_FLASH_CPU0_S_SIZE + BSP_PARTITION_FLASH_CPU0_C_SIZE),
               "RA8M2: image 0 secondary is not the size of the primary image "
               "(FLASH_CPU0_S + FLASH_CPU0_C).");
_Static_assert(BSP_PARTITION___BL_1_S_I_SIZE == BSP_PARTITION_FLASH_CPU0_N_SIZE,
               "RA8M2: image 1 secondary is not the size of the primary image "
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
 * fires, give DF_EMULATION more room in the e2 solution and raise
 * TFM_NV_COUNTERS_AREA_SIZE in flash_layout.h; do not shrink the services to fit.
 */

#if !defined(TFM_OTP_NV_COUNTERS_AREA_SIZE)
#error "RA8M2: TFM_OTP_NV_COUNTERS_AREA_SIZE is not defined - check flash_layout.h"
#endif

/* The backend keeps a primary copy and a backup mirror, each of this size. */
_Static_assert(TFM_OTP_NV_COUNTERS_AREA_SIZE >=
                   sizeof(struct flash_otp_nv_counters_region_t),
               "RA8M2: TFM_OTP_NV_COUNTERS_AREA_SIZE is smaller than "
               "sizeof(struct flash_otp_nv_counters_region_t). "
               "init_otp_nv_counters_flash() would compile to an unconditional failure. "
               "Raise TFM_NV_COUNTERS_AREA_SIZE in flash_layout.h - it is halved to give "
               "the area and its backup mirror.");

/* Both halves must be whole erase sectors: the backend erases with EraseSector() and
 * rejects a start address that is not sector aligned. */
_Static_assert((TFM_OTP_NV_COUNTERS_AREA_SIZE % TFM_OTP_NV_COUNTERS_SECTOR_SIZE) == 0,
               "RA8M2: the OTP area must be a whole number of erase sectors.");

/*
 * The combined S+NS image (tfm_s_ns_signed.bin, which TF-M's NSPE rules build for any NS
 * application that does not exclude the target) is a plain concatenation, with
 * NON_SECURE_IMAGE_OFFSET marking where the second half begins. It describes the flash only
 * while the two primary slots are adjacent. Adjacency follows from the secure slot ordering
 * asserted below; it is not a goal in itself. ns_app excludes the target; see flash_layout.h,
 * NON_SECURE_IMAGE_OFFSET.
 */
_Static_assert(FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE == FLASH_AREA_1_OFFSET,
               "RA8M2: the secure and non-secure PRIMARY slots are no longer adjacent, so a "
               "concatenated tfm_s_ns_signed.bin would place the non-secure image at the wrong "
               "address. Either restore adjacency, or update flash_layout.h's "
               "NON_SECURE_IMAGE_OFFSET note and this check together.");

/*
 * Slot ordering: secondary below primary on the secure side.
 *
 * The primary slot must be the topmost secure region so that FLASH_CPU0_C, at its top, ends
 * exactly at the secure/non-secure boundary. With the secondary above the primary, 288 KB of
 * secure flash would sit between the NSC region and the boundary, which RDPM cannot describe.
 * RA6E1 and RA6M5 order the slots the same way for the same reason.
 */
_Static_assert(FLASH_AREA_2_OFFSET + FLASH_AREA_2_SIZE == FLASH_AREA_0_OFFSET,
               "RA8M2: the secure secondary slot no longer sits immediately below the primary. "
               "The primary must be the topmost secure slot so its NSC region touches the "
               "secure/non-secure boundary - see the note above.");

/* The NSC region must end exactly at the non-secure boundary; otherwise RDPM cannot express
 * the partitioning, although the SAU could. */
_Static_assert(TFM_MRAM_S_OFF(BSP_PARTITION_FLASH_CPU0_C_START) +
                   BSP_PARTITION_FLASH_CPU0_C_SIZE == FLASH_AREA_1_OFFSET,
               "RA8M2: the NSC region does not end at the secure/non-secure boundary. RDPM "
               "only expresses a contiguous Secure|NSC|NS triple, so the NSC must be the last "
               "thing in secure space.");

_Static_assert(FLASH_AREA_1_OFFSET + FLASH_AREA_1_SIZE == FLASH_AREA_3_OFFSET,
               "RA8M2: the non-secure primary and secondary slots are no longer contiguous.");

/* flash_layout.h states the non-secure alias base as a literal because it is preprocessed
 * into ra8m2_bl2.ld and cannot reach bsp_feature.h. Driver_Flash.c derives the same base from
 * the BSP; this assertion keeps the two equal. */
_Static_assert(FLASH_NS_ALIAS_BASE == FLASH_BASE_ADDRESS + BSP_FEATURE_TZ_NS_OFFSET,
               "RA8M2: FLASH_NS_ALIAS_BASE in flash_layout.h no longer matches "
               "FLASH_BASE_ADDRESS + BSP_FEATURE_TZ_NS_OFFSET. The literal exists only "
               "because that header feeds the linker script; FSP's value is authoritative.");

/* Driver_Flash.c's MRAM_ADDR() selects the secure or non-secure MRAM alias with one
 * comparison against the end of the NSC region. That holds only while every secure area lies
 * below it and every non-secure area at or above it, which these four assert (the NSC end is
 * asserted equal to FLASH_AREA_1_OFFSET above). If the ordering breaks, the driver forms an
 * address in an alias that does not answer for that byte: a HardFault inside memcpy(), and
 * only on a part RDPM has already partitioned. */
_Static_assert(FLASH_AREA_BL2_OFFSET + FLASH_AREA_BL2_SIZE <= FLASH_AREA_1_OFFSET,
               "RA8M2: BL2 is not below the non-secure boundary - MRAM_ADDR() would use the "
               "wrong alias for it.");
_Static_assert(FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE <= FLASH_AREA_1_OFFSET,
               "RA8M2: the secure primary slot is not below the non-secure boundary - "
               "MRAM_ADDR() would use the wrong alias for it.");
_Static_assert(FLASH_AREA_2_OFFSET + FLASH_AREA_2_SIZE <= FLASH_AREA_1_OFFSET,
               "RA8M2: the secure secondary slot is not below the non-secure boundary - "
               "MRAM_ADDR() would use the wrong alias for it.");
_Static_assert(FLASH_AREA_3_OFFSET >= FLASH_AREA_1_OFFSET,
               "RA8M2: the non-secure secondary slot is below the non-secure boundary - "
               "MRAM_ADDR() would use the wrong alias for it.");

/*
 * DF_EMULATION must not overlap any MCUboot area.
 *
 * ITS, PS and the NV counters live in a reserved region of secure MRAM (flash_layout.h,
 * "Storage"). An update overwrites a whole slot, so an overlap would silently destroy the
 * storage on the first image upgrade; nothing at run time cross-checks the two views.
 */
#define RA8M2_DF_OFF    (TFM_MRAM_S_OFF(BSP_PARTITION_DF_EMULATION_START))
#define RA8M2_DF_END    (RA8M2_DF_OFF + BSP_PARTITION_DF_EMULATION_SIZE)
#define RA8M2_NO_OVERLAP(o, s)  ((RA8M2_DF_END <= (o)) || (RA8M2_DF_OFF >= ((o) + (s))))

_Static_assert(RA8M2_NO_OVERLAP(FLASH_AREA_BL2_OFFSET, FLASH_AREA_BL2_SIZE),
               "RA8M2: DF_EMULATION overlaps BL2.");
_Static_assert(RA8M2_NO_OVERLAP(FLASH_AREA_0_OFFSET, FLASH_AREA_0_SIZE),
               "RA8M2: DF_EMULATION overlaps the secure primary slot - an image update "
               "would erase ITS, PS and the NV counters.");
_Static_assert(RA8M2_NO_OVERLAP(FLASH_AREA_2_OFFSET, FLASH_AREA_2_SIZE),
               "RA8M2: DF_EMULATION overlaps the secure secondary slot.");
_Static_assert(RA8M2_NO_OVERLAP(FLASH_AREA_1_OFFSET, FLASH_AREA_1_SIZE),
               "RA8M2: DF_EMULATION overlaps the non-secure primary slot.");
_Static_assert(RA8M2_NO_OVERLAP(FLASH_AREA_3_OFFSET, FLASH_AREA_3_SIZE),
               "RA8M2: DF_EMULATION overlaps the non-secure secondary slot.");

/*
 * Every MCUboot area must be a whole number of FLASH_AREA_IMAGE_SECTOR_SIZE sectors, in both
 * offset and size.
 *
 * flash_area_get_sectors() fails on a remainder. imgtool, the linker and the flasher all
 * accept it, and the failure appears only on hardware, as boot_read_sectors() returning
 * BOOT_EFLASH. A 0x4F000 secure slot (9.875 sectors at FSP's 32 KB MRAM block size) fails this
 * check. For the effect on DF_EMULATION sizing, see flash_layout.h,
 * FLASH_AREA_IMAGE_SECTOR_SIZE.
 */
#define RA8M2_WHOLE_SECTORS(o, s)  ((((o) % FLASH_AREA_IMAGE_SECTOR_SIZE) == 0) &&                                     (((s) % FLASH_AREA_IMAGE_SECTOR_SIZE) == 0))

_Static_assert(RA8M2_WHOLE_SECTORS(FLASH_AREA_BL2_OFFSET, FLASH_AREA_BL2_SIZE),
               "RA8M2: the BL2 area is not a whole number of sectors.");
_Static_assert(RA8M2_WHOLE_SECTORS(FLASH_AREA_0_OFFSET, FLASH_AREA_0_SIZE),
               "RA8M2: the secure PRIMARY slot is not a whole number of "
               "FLASH_AREA_IMAGE_SECTOR_SIZE sectors. boot_read_sectors() would return "
               "BOOT_EFLASH on hardware. Repartition in the solution - see flash_layout.h.");
_Static_assert(RA8M2_WHOLE_SECTORS(FLASH_AREA_2_OFFSET, FLASH_AREA_2_SIZE),
               "RA8M2: the secure SECONDARY slot is not a whole number of sectors.");
_Static_assert(RA8M2_WHOLE_SECTORS(FLASH_AREA_1_OFFSET, FLASH_AREA_1_SIZE),
               "RA8M2: the non-secure PRIMARY slot is not a whole number of sectors.");
_Static_assert(RA8M2_WHOLE_SECTORS(FLASH_AREA_3_OFFSET, FLASH_AREA_3_SIZE),
               "RA8M2: the non-secure SECONDARY slot is not a whole number of sectors.");

/* DF_EMULATION is not an MCUboot area, so it need not be sector-sized in itself. It sits
 * between BL2 and the first slot, so its size shifts every slot offset after it; if it is not
 * a whole number of sectors, the slots cannot be either. */
_Static_assert((BSP_PARTITION_DF_EMULATION_SIZE % FLASH_AREA_IMAGE_SECTOR_SIZE) == 0,
               "RA8M2: DF_EMULATION is not a whole number of sectors, so it pushes every "
               "slot after it off a sector boundary.");

/* The device is allocated to the last byte; catch a repartition that leaves MRAM unused. */
_Static_assert(FLASH_AREA_3_OFFSET + FLASH_AREA_3_SIZE == FLASH_TOTAL_SIZE,
               "RA8M2: the partitioning no longer reaches the end of MRAM.");

/* DF_EMULATION is deliberately in secure MRAM: PS holds secure-side assets and the NV
 * counters gate rollback, so a non-secure-writable region would defeat both. */
_Static_assert(BSP_PARTITION_DF_EMULATION_START >= FLASH_BASE_ADDRESS &&
               BSP_PARTITION_DF_EMULATION_START < (FLASH_BASE_ADDRESS + FLASH_TOTAL_SIZE),
               "RA8M2: DF_EMULATION is not in the secure MRAM alias.");

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
 * PS_AES_KEY_USAGE_LIMIT 0, 4-byte program unit. The program unit is the same as RA6M5's
 * because MRAM writes single bytes and TFM_HAL_PS_PROGRAM_UNIT is 4 here too (flash_layout.h),
 * so the write-alignment overhead matches the validated port.
 *
 * The margin is far wider than on RA6M5: DF_EMULATION gives PS 31,744 B against 3,072, so the
 * assertion below clears by about 15,000 bytes rather than 88.
 * --------------------------------------------------------------------------------- */
#define RA8M2_PS_BLOCK_SIZE     (TFM_HAL_PS_SECTOR_SIZE * TFM_HAL_PS_SECTORS_PER_BLOCK)
#define RA8M2_PS_NUM_BLOCKS     (TFM_HAL_PS_FLASH_AREA_SIZE / RA8M2_PS_BLOCK_SIZE)

/* Block metadata header + one block_meta + one file_meta per file. */
#define RA8M2_PS_FS_METADATA    (8 + 12 + ((PS_NUM_ASSETS + 3) * 32))
/* sizeof(struct ps_obj_table_t), and both FS IDs are always resident. */
#define RA8M2_PS_TABLES         (2 * (48 + ((PS_NUM_ASSETS + 1) * 32)))
/* sizeof(struct ps_obj_header_t) + PS_TAG_LEN_BYTES, added to every asset. */
#define RA8M2_PS_OBJ_OVERHEAD   (56 + 16)
/* One small asset assumed permanently resident. tf-m-tests' TFM_NS_PS_TEST_1004 creates a
 * 36-byte PSA_STORAGE_FLAG_WRITE_ONCE asset that by definition can never be removed. */
#define RA8M2_PS_RESIDENT       (36 + RA8M2_PS_OBJ_OVERHEAD)

_Static_assert(RA8M2_PS_NUM_BLOCKS == 2,
               "RA8M2: the PS budget below assumes the two-block filesystem, where data "
               "block 0 shares the metadata block. Rework it if the PS area is resized to "
               "give the FS dedicated data blocks.");

_Static_assert((RA8M2_PS_BLOCK_SIZE - RA8M2_PS_FS_METADATA - RA8M2_PS_TABLES
                - RA8M2_PS_RESIDENT) >= (PS_MAX_ASSET_SIZE + RA8M2_PS_OBJ_OVERHEAD),
               "RA8M2: PS cannot hold one PS_MAX_ASSET_SIZE asset alongside its own "
               "metadata and object tables. Each asset costs 96 bytes of fixed overhead, "
               "so lower PS_NUM_ASSETS in config_tfm_target.h - lowering PS_MAX_ASSET_SIZE "
               "recovers far less and has a floor of its own. This fires as "
               "PSA_ERROR_INSUFFICIENT_STORAGE from psa_ps_set at run time.");
