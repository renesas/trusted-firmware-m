/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Build-time guards on the flash layout derived from the e2 solution.
 *
 * Everything here is a static assertion; the file emits no code. It exists because each
 * of these has already been shipped wrong once and was invisible until hardware.
 */

#include "config_tfm.h"
#include "flash_otp_nv_counters_backend.h"
#include "flash_layout.h"
#include "region_defs.h"
/* bsp_api.h, not bsp_cfg.h directly: bsp_cfg.h pulls the board headers, which use
 * FSP_HEADER before it is defined. Entering through bsp_api.h gets the ordering right. */
#include "bsp_api.h"

/* ---------------------------------------------------------------------------------
 * Main stack
 *
 * bsp_init_stub.c aliases FSP's g_main_stack onto Image$$ARM_LIB_STACK$$ZI$$Base, so
 * FSP's SystemInit() sets MSPLIM to the base of the stack the CPU is really running on.
 * The other thing it does with that symbol is write its stack seal at
 * &g_main_stack[BSP_CFG_STACK_MAIN_BYTES]. The generated scatter file sizes .msp_stack
 * as S_MSP_STACK_SIZE - STACKSEAL_SIZE and reserves STACKSEAL_SIZE above it, so the two
 * agree only when S_MSP_STACK_SIZE is BSP_CFG_STACK_MAIN_BYTES + STACKSEAL_SIZE. If the
 * solution is regenerated with a different FSP main stack size, that write moves off the
 * reserved seal and into whatever the linker put above the stack.
 * --------------------------------------------------------------------------------- */
#ifndef STACKSEAL_SIZE
#define STACKSEAL_SIZE (8)
#endif

/* ---------------------------------------------------------------------------------
 * BSP_CFG_EARLY_INIT must be 1 in the secure e2 project (DESIGN.md 8.1).
 *
 * TF-M's Reset_Handler calls SystemInit() BEFORE the C runtime zeroes .bss. With early
 * init off, FSP leaves SystemCoreClock and the rest of its clock state in ordinary .bss,
 * so the value SystemInit() computed is wiped straight after, R_MRAM_Open() derives
 * FCLK = 0 from it and fails with FSP_ERR_FCLK. Early init is what moves that state into
 * .ram_noinit, which the port's linker scripts keep out of .bss.
 *
 * The linker half of this fix lives in the port and carries over by itself; this half
 * lives in the e2 project and does not - the first RA8M2 generation had it off and failed
 * on hardware exactly this way. bl2_option_setting.c carries the same check for BL2.
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
 * MCUboot slot geometry
 *
 * Each slot must be contiguous (span == sum of its components) and a whole number of
 * erase sectors. flash_area_get_sectors() walks a slot sector by sector and returns an
 * error on any remainder, so a misaligned slot appears only as boot_read_sectors()
 * returning BOOT_EFLASH from boot_prepare_image_for_update(). Meanwhile the FSP
 * generator sums the component sizes and imgtool pads to the span, so a gap makes the
 * signed image and the bootloader disagree about where the trailer magic lives.
 * See the note on TFM_SLOT_SPAN in flash_layout.h.
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

/* A secondary slot holds a copy of the whole primary image. The primary expresses that
 * image as the solution's application regions - two of them for the secure image, since
 * the code and the NSC veneers carry different TrustZone attributes - while the
 * secondary is a single undifferentiated I region. Per the generator schema
 * (linker_macros.j2: "Secondary images must be same size as the area allocated for the
 * project"), the sizes have to match, or an upgrade overruns or wastes the slot. */
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
 * Both operands are compile-time constants, so when the area is too small the compiler
 * folds the test and discards the rest of the function - it becomes an unconditional
 * error return that never touches flash. Nothing warns, the image links, and the failure
 * only appears on hardware as OTP, ITS and PS all refusing to initialise.
 *
 * The struct grows with MCUBOOT_BUILTIN_KEY, PLATFORM_NS_NV_COUNTERS and
 * FLASH_NV_COUNTER_AM, none of which this port controls, so this is checked rather than
 * hard-coded. If it fires, give the secure DATA_FLASH partition more room in the e2
 * solution and raise TFM_NV_COUNTERS_AREA_SIZE in flash_layout.h - do not shrink the
 * services to fit.
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
 * The combined S+NS image (tfm_s_ns_signed.bin, built by TF-M's NSPE rules for any NS
 * application that does not exclude the target) is a plain concatenation, with
 * NON_SECURE_IMAGE_OFFSET saying where the second half begins. That only describes the
 * flash correctly while the two PRIMARY slots are adjacent.
 *
 * ON RA8M2 THEY ARE NOT. The secure primary ends at 0x61000, the secure SECONDARY occupies
 * 0x61000-0xB0000, and the non-secure primary starts at 0xB0000. RA6M5 - which this port
 * was derived from - had them adjacent and asserted it; here the inverted assertion is the
 * correct one, so that the SAME file stays honest if a future repartition makes them
 * adjacent again and the combined image silently becomes flashable.
 *
 * Nothing on this port flashes it (ns_app excludes the target), and flash_layout.h says so
 * at NON_SECURE_IMAGE_OFFSET. Flash tfm_s_signed.bin and tfm_ns_signed.bin separately.
 */
_Static_assert(FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE != FLASH_AREA_1_OFFSET,
               "RA8M2: the secure and non-secure PRIMARY slots have become adjacent. That is "
               "not a problem in itself - it means a concatenated tfm_s_ns_signed.bin would "
               "now describe the flash correctly - but flash_layout.h and this check both "
               "document the opposite. Update both, and reconsider whether ns_app should "
               "still exclude the combined-image target.");

/* The two SECURE areas are what sit between them; assert the actual adjacency this layout
 * does have, so a repartition that introduces a gap is caught. A gap would be wasted MRAM
 * on a device that is currently allocated to the last byte. */
_Static_assert(FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE == FLASH_AREA_2_OFFSET,
               "RA8M2: the secure primary and secondary slots are no longer contiguous.");
_Static_assert(FLASH_AREA_1_OFFSET + FLASH_AREA_1_SIZE == FLASH_AREA_3_OFFSET,
               "RA8M2: the non-secure primary and secondary slots are no longer contiguous.");

/*
 * DF_EMULATION must not overlap any MCUboot area.
 *
 * This part has no data flash, so ITS, PS and the NV counters live in a reserved region of
 * secure MRAM (flash_layout.h, "Storage"). An update overwrites a whole slot, so an overlap
 * would destroy the storage on the first image upgrade - and would do it silently, since
 * nothing at run time cross-checks the two views of the device.
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
 * Every MCUboot area must be a whole number of FLASH_AREA_IMAGE_SECTOR_SIZE, in BOTH its
 * offset and its size.
 *
 * flash_area_get_sectors() walks an area sector by sector and fails outright on a
 * remainder. Nothing earlier in the build notices: imgtool signs, the linker links, the
 * image flashes, and the failure appears only on hardware as boot_read_sectors() returning
 * BOOT_EFLASH.
 *
 * This is the check that a 0x4F000 secure slot would have failed - 9.875 sectors at FSP's
 * 32 KB MRAM block size. On a 1 MB device the constraint propagates into the partitioning:
 * BL2 at 64 KB leaves 960 KB, and DF_EMULATION must take 64 KB of it, because 8 or 32 KB
 * leaves a remainder that cannot split into two secure and two non-secure slots on a 32 KB
 * boundary. See flash_layout.h.
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

/* DF_EMULATION is not an MCUboot area, so it need not be sector-sized in itself - but it
 * sits between BL2 and the first slot, so its size shifts every slot offset after it. If it
 * is not a whole number of sectors, the slots cannot be either. */
_Static_assert((BSP_PARTITION_DF_EMULATION_SIZE % FLASH_AREA_IMAGE_SECTOR_SIZE) == 0,
               "RA8M2: DF_EMULATION is not a whole number of sectors, so it pushes every "
               "slot after it off a sector boundary.");

/* The device is allocated to the last byte; catch a repartition that leaves MRAM unused. */
_Static_assert(FLASH_AREA_3_OFFSET + FLASH_AREA_3_SIZE == FLASH_TOTAL_SIZE,
               "RA8M2: the partitioning no longer reaches the end of MRAM.");

/* DF_EMULATION is in SECURE MRAM on purpose: PS holds secure-side assets and the NV
 * counters gate rollback, so a non-secure-writable region would defeat both. */
_Static_assert(BSP_PARTITION_DF_EMULATION_START >= FLASH_BASE_ADDRESS &&
               BSP_PARTITION_DF_EMULATION_START < (FLASH_BASE_ADDRESS + FLASH_TOTAL_SIZE),
               "RA8M2: DF_EMULATION is not in the secure MRAM alias.");

/* ---------------------------------------------------------------------------------
 * Protected Storage capacity
 *
 * With num_blocks == 2 the flash FS keeps its metadata and logical data block 0 in the
 * same physical block, so every live PS object has to fit in one block together with the
 * metadata. The per-object ceiling in config_tfm_target.h is not the binding constraint -
 * the sum is, and it is dominated by fixed overhead that scales with PS_NUM_ASSETS.
 *
 * Sizes are from the flash FS and PS structs (its_flash_fs_mblock.h, ps_object_defs.h,
 * ps_object_table.c) at this port's settings: PS_ENCRYPTION on, PS_ROLLBACK_PROTECTION on,
 * PS_AES_KEY_USAGE_LIMIT 0. They are struct sizes, so they do not move with the program
 * unit - but the flash FS ALIGNS ITS WRITES to it, and this part's is 32 B of MRAM against
 * RA6M5's 4 B of data flash. The per-object rounding that implies is NOT modelled below and
 * has not been measured on hardware, so treat the margin as thinner than it reads: at
 * PS_NUM_ASSETS 5 the slack is 672 - 584 = 88 bytes, under three 32 B write units. If PS
 * returns PSA_ERROR_INSUFFICIENT_STORAGE on this part despite these assertions passing,
 * alignment overhead is the first thing to suspect (DECISIONS D051).
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
