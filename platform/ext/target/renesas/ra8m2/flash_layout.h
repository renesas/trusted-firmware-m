/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __FLASH_LAYOUT_H__
#define __FLASH_LAYOUT_H__

/*
 * RA8M2 flash layout - DERIVED, NOT HAND-MAINTAINED.
 *
 * Every address and size below comes from bsp_linker_info.h, which the e2 studio
 * SOLUTION generates for each project from solution.xml. Change the partitions in
 * the solution, rebuild the projects in e2, and this header follows with no edits.
 *
 * This inverts the ra6m4 model, where flash_layout.h was authoritative and the RASC
 * values were vestigial (DESIGN.md 3). Here RASC/e2 is the source of truth.
 *
 * bsp_linker_info.h lives in <project>/Debug/, so the solution must have been built
 * in e2 at least once before TF-M can configure. The platform CMakeLists fails with a
 * clear message if it is absent.
 *
 * We include bsp_partitions.h, not bsp_linker_info.h directly: the platform CMakeLists
 * filters the generated file down to its #define BSP_PARTITION_* lines. This header is
 * preprocessed into ra8m2_bl2.ld as well as compiled, and bsp_linker_info.h also
 * declares C types and externs that a linker script cannot parse.
 *
 * Partition -> TF-M mapping:
 *   FLASH_BL_CPU0_S   -> BL2 (MCUboot)
 *   DF_EMULATION      -> ITS + PS + NV counters (see "Storage" below)
 *   __BL_0_P_*        -> image 0 primary   = secure     (FLASH_AREA_0)
 *   __BL_1_P_*        -> image 1 primary   = non-secure (FLASH_AREA_1)
 *   __BL_0_S_*        -> image 0 secondary = secure     (FLASH_AREA_2)
 *   __BL_1_S_*        -> image 1 secondary = non-secure (FLASH_AREA_3)
 *
 * ----------------------------------------------------------------------------------
 * HOW THIS DIFFERS FROM RA6M5, which it was derived from:
 *
 *   1. The storage is MRAM, not code flash. Write unit 32 B, not 128; the MCUboot
 *      sector stays 32 KB, which FSP defines as RM_MCUBOOT_MRAM_BLOCK_SIZE.
 *   2. There is NO DATA FLASH on this part (DATA_FLASH_CPU0_S_SIZE is 0), so ITS, PS
 *      and the NV counters come out of a reserved MRAM region, DF_EMULATION.
 *   3. BSP_PARTITION_*_START values are ABSOLUTE addresses in their own security
 *      alias, not offsets - see TFM_MRAM_S_OFF / TFM_MRAM_NS_OFF.
 *   4. The secure SECONDARY slot sits BELOW the primary, so the primary is the topmost
 *      secure slot and its NSC region ends exactly at the secure/non-secure boundary. RDPM
 *      only expresses a contiguous Secure|NSC|NS triple, not the split the SAU supports.
 *      RA6E1/RA6M5 order them the same way for the same reason.
 * ----------------------------------------------------------------------------------
 */

#include "bsp_partitions.h"

/* ---------------------------------------------------------------------------------
 * Device geometry - MRAM.
 *
 * MRAM is aliased twice: secure at 0x02000000 and non-secure at 0x12000000, one
 * megabyte each describing the SAME physical array. bsp_linker_info.h emits each
 * partition's start as an absolute address in whichever alias its security attribute
 * selects, so the secure partitions come out as 0x020xxxxx and the non-secure ones as
 * 0x120xxxxx. MCUboot wants a single flat offset into one device, so each start is
 * converted below by subtracting its own alias base. Both aliases give the same
 * physical offset, which is what makes one Driver_FLASH0 able to serve all four slots.
 *
 * RA6M5 needed none of this: its code flash is based at 0x00000000, so the generated
 * starts doubled as offsets and were used directly.
 * ------------------------------------------------------------------------------- */
#define FLASH_BASE_ADDRESS              (0x02000000)     /* MRAM, secure alias */
#define FLASH_NS_ALIAS_BASE             (0x12000000)     /* MRAM, non-secure alias */
#define FLASH_TOTAL_SIZE                (0x00100000)     /* 1 MB */
#define FLASH_DEV_NAME                  Driver_FLASH0

#define TFM_MRAM_S_OFF(addr)            ((addr) - (FLASH_BASE_ADDRESS))
#define TFM_MRAM_NS_OFF(addr)           ((addr) - (FLASH_NS_ALIAS_BASE))

/* MCUboot's write unit: BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES, 32.
 *
 * READ THE NEXT PARAGRAPH BEFORE REUSING THIS NUMBER. 32 is the size of the MRAM
 * PROGRAMMING BUFFER, not a minimum write. MRAM's write granularity is ONE BYTE:
 * r_mram.c's flash_info reports block_size 32 (the ERASE block) and
 * block_size_write 1, mram_write_data() copies byte at a time and flushes a partial
 * buffer through MRCFLR - the FSP comment there cites the hardware manual's
 * "smaller than 32-bytes programming" procedure - and
 * r_mram_write_erase_parameter_checking() enforces no alignment or multiple at all.
 *
 * It is 32 HERE because FSP says so for MCUboot specifically: its generated
 * mcuboot_config.h sets MCUBOOT_BOOT_MAX_ALIGN from the same BSP feature. Driver_Flash.c
 * advertises it as FLASH0's program_unit and bl2/src/flash_map.c returns it as
 * flash_area_align(); config.cmake must keep MCUBOOT_ALIGN_VAL equal to it, since the
 * images are signed with --align and the trailer geometry has to match at runtime.
 * Consume FSP's choice rather than second-guessing it.
 *
 * The ITS/PS side is NOT 32 - see TFM_HAL_DATA_FLASH_PROGRAM_UNIT below. */
#define TFM_HAL_FLASH_PROGRAM_UNIT      (32)
#define TFM_HAL_CODE_FLASH_PROGRAM_UNIT (32)

/* Sector size - TAKEN FROM FSP, NOT CHOSEN HERE.
 *
 * FSP's generated ra_cfg/mcu-tools/include/mcuboot_config/mcuboot_config.h has the
 * authoritative value:
 *
 *     #if BSP_FEATURE_MRAM_IS_AVAILABLE
 *      #define RM_MCUBOOT_MRAM_BLOCK_SIZE     (0x8000)
 *      #define FLASH_AREA_IMAGE_SECTOR_SIZE   (RM_MCUBOOT_MRAM_BLOCK_SIZE)
 *      #define MCUBOOT_BOOT_MAX_ALIGN         (RM_MCUBOOT_MRAM_PROGRAMMING_SIZE_BYTES)
 *
 * That directory IS on this build's include path (cmake/modules/fsp_bsp.cmake), so a
 * different value here is a conflicting redefinition, not merely a duplicated constant.
 * The port's contract is that FSP-generated configuration is consumed, not restated -
 * the user configures in e2/RASC and the TF-M build follows.
 *
 * NOTE the two are DIFFERENT numbers and both matter:
 *   - 0x8000 is the MCUboot SECTOR, what flash_area_get_sectors() walks.
 *   - 32 is the MRAM WRITE unit (BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES), which FSP
 *     uses for MCUBOOT_BOOT_MAX_ALIGN and which MCUBOOT_ALIGN_VAL matches.
 * r_mram.c's flash_info block_size is the latter (32), NOT the sector size. Reading it
 * as the sector size produced a layout whose slots were 9.875 sectors long - accepted by
 * every build step and failing only on hardware, as boot_read_sectors() returning
 * BOOT_EFLASH.
 *
 * Every MCUboot area's offset AND size must be a whole multiple of this. On a 1 MB
 * device that is a real constraint on the partitioning: with BL2 at 64 KB it forces
 * DF_EMULATION to 64 KB, because 8 or 32 KB leaves a remainder that cannot be split into
 * two secure and two non-secure slots on a 32 KB boundary. ra8m2_layout_checks.c asserts
 * the sizing, and that this value still agrees with FSP's header.
 *
 * Stated as a literal rather than including mcuboot_config.h because this header is also
 * preprocessed into ra8m2_bl2.ld, and that header includes bsp_api.h. Extracting it into
 * the generated bsp_partitions.h - the mechanism already used for the partitions - is the
 * proper fix and is on the TODO list. */
#define FLASH_AREA_IMAGE_SECTOR_SIZE    (0x8000)         /* == RM_MCUBOOT_MRAM_BLOCK_SIZE */

/* BL2 */
#define FLASH_AREA_BL2_OFFSET           TFM_MRAM_S_OFF(BSP_PARTITION_FLASH_BL_CPU0_S_START)
#define FLASH_AREA_BL2_SIZE             (BSP_PARTITION_FLASH_BL_CPU0_S_SIZE)

/* A slot spans its header through the end of its trailer.
 *
 * The solution's components MUST be contiguous, so that this span equals the sum of the
 * component sizes. Everything downstream assumes that: the FSP generator emits
 * .fa_size as a plain sum of the parts (linker_macros_bsp_h.j2), imgtool pads to
 * FLASH_AREA_0_SIZE via RE_SIGN_BIN_SIZE, and bootutil reads the trailer magic at
 * fa_off + fa_size - 16. A gap between components makes the span exceed the sum, and
 * the two views of the slot silently diverge.
 *
 * Both ends are in the same security alias, so the alias base cancels and this needs no
 * TFM_MRAM_*_OFF conversion - it is a difference, not an address.
 *
 * The trailers are ZERO-SIZE END MARKERS, as on RA6E1 and RA6M5 - the solution used real
 * 0x100 trailers until the 2026-09-24 repartition. The arithmetic is identical either way,
 * but the marker convention is required here: a real trailer after FLASH_CPU0_C would put
 * 256 secure bytes between the NSC and the non-secure boundary, and RDPM cannot describe
 * that. MCUboot takes its trailer from the slot end backwards, out of the image region.
 *
 * ra8m2_layout_checks.c asserts contiguity and whole-sector sizing at build time. */
#define TFM_SLOT_SPAN(h, t)             (((t##_START) + (t##_SIZE)) - (h##_START))

/* Image 0 = secure. Primary 0x12000, secondary 0x61000, 0x4F000 each. */
#define FLASH_AREA_0_ID                 (1)
#define FLASH_AREA_0_OFFSET             TFM_MRAM_S_OFF(BSP_PARTITION___BL_0_P_H_START)
#define FLASH_AREA_0_SIZE               TFM_SLOT_SPAN(BSP_PARTITION___BL_0_P_H, \
                                                      BSP_PARTITION___BL_0_P_T)
#define FLASH_AREA_2_ID                 (FLASH_AREA_0_ID + 2)
#define FLASH_AREA_2_OFFSET             TFM_MRAM_S_OFF(BSP_PARTITION___BL_0_S_H_START)
#define FLASH_AREA_2_SIZE               TFM_SLOT_SPAN(BSP_PARTITION___BL_0_S_H, \
                                                      BSP_PARTITION___BL_0_S_T)

/* Image 1 = non-secure. Primary 0xB0000, secondary 0xD8000, 0x28000 each.
 * NS alias base, not the secure one - see the geometry note above. */
#define FLASH_AREA_1_ID                 (FLASH_AREA_0_ID + 1)
#define FLASH_AREA_1_OFFSET             TFM_MRAM_NS_OFF(BSP_PARTITION___BL_1_P_H_START)
#define FLASH_AREA_1_SIZE               TFM_SLOT_SPAN(BSP_PARTITION___BL_1_P_H, \
                                                      BSP_PARTITION___BL_1_P_T)
#define FLASH_AREA_3_ID                 (FLASH_AREA_0_ID + 3)
#define FLASH_AREA_3_OFFSET             TFM_MRAM_NS_OFF(BSP_PARTITION___BL_1_S_H_START)
#define FLASH_AREA_3_SIZE               TFM_SLOT_SPAN(BSP_PARTITION___BL_1_S_H, \
                                                      BSP_PARTITION___BL_1_S_T)

/* The solution is configured overwrite-only, so there is no scratch area.
 *
 * Unlike RA6M5, NO block is reserved for one: the 1 MB device is fully allocated
 * (64 K BL2 + 8 K DF_EMULATION + 2 x 316 K secure + 2 x 160 K non-secure = 1024 K
 * exactly). Switching to swap-using-scratch therefore needs the partitioning changed in
 * the solution first, not just these three macros repointed - something has to give up
 * the space. A scratch area must be SECURE when it exists: during a swap it holds
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
 * The two primary slots ARE adjacent - the secure slot ends exactly where the non-secure slot
 * begins - so a plain concatenation describes the flash correctly, with these as the offsets
 * relative to the start of the combined image. That was NOT true before the 2026-09-24
 * repartition, when the secure secondary sat between them; both this note and the assertion in
 * ra8m2_layout_checks.c said so, and both have been corrected. Adjacency is a consequence of
 * putting the primary slot topmost so its NSC touches the boundary, not a goal in itself, so
 * the assertion stays: it is a property of the current partitioning, not a rule.
 *
 * Nothing on this port FLASHES the combined image: ns_app excludes the target, because as an
 * extra row in the debug session's program list it carries no address of its own and is
 * silently destructive at the wrong one. But the macros stay, because TF-M's NSPE rules build
 * tfm_s_ns_signed.bin for any NS application that does not exclude it - tf-m-tests' regression
 * app does not - and without them assemble.py dies with
 * "NameError: name 'SECURE_IMAGE_OFFSET' is not defined". Removing them once already broke
 * that build.
 */
#define SECURE_IMAGE_OFFSET             (0x0)
#define SECURE_IMAGE_MAX_SIZE           FLASH_AREA_0_SIZE
#define NON_SECURE_IMAGE_OFFSET         (SECURE_IMAGE_OFFSET + SECURE_IMAGE_MAX_SIZE)
#define NON_SECURE_IMAGE_MAX_SIZE       FLASH_AREA_1_SIZE

/* Sectors bootutil must be able to track for one image. ra6m4 sized this from the
 * scratch area; overwrite-only has no scratch, so size it from the largest slot,
 * rounded up. 0x48000 / 0x8000 = 9.
 *
 * FSP computes its own as RM_MCUBOOT_LARGEST_SECTOR / FLASH_AREA_IMAGE_SECTOR_SIZE,
 * over FSP's partition view. TF-M has its own flash map, so this one is derived from
 * TF-M's areas - the sector SIZE is shared, the sector COUNT deliberately is not. */
#define MCUBOOT_MAX_IMG_SECTORS         ((FLASH_MAX_PARTITION_SIZE + \
                                          FLASH_AREA_IMAGE_SECTOR_SIZE - 1) / \
                                         FLASH_AREA_IMAGE_SECTOR_SIZE)

/* Image sizes seen by imgtool/bootutil (slot minus header and trailer). */
#define IMAGE_S_CODE_SIZE               (BSP_PARTITION_FLASH_CPU0_S_SIZE + \
                                         BSP_PARTITION_FLASH_CPU0_C_SIZE)
#define IMAGE_NS_CODE_SIZE              (BSP_PARTITION_FLASH_CPU0_N_SIZE)

/* =================================================================================
 * Storage - ITS, PS and the MCUboot NV counters.
 *
 * RA8M2 HAS NO DATA FLASH. BSP_PARTITION_DATA_FLASH_CPU0_S_SIZE is 0, and the rzone
 * gives the whole DATA_FLASH region size 0. On RA6E1 and RA6M5 these three services
 * shared 0x2000 of secure data flash; here they share DF_EMULATION, an 0x2000 region
 * of SECURE MRAM at 0x02010000, between BL2 and the secure primary slot.
 *
 * DF_EMULATION is deliberately OUTSIDE every MCUboot area. An update overwrites a whole
 * slot, so storage placed inside one would be destroyed by the first image upgrade -
 * which is also why it cannot simply be carved out of the slack in a slot.
 *
 * The split follows RA6M5's proportions: NV counters 2048 B, then PS and ITS take half the
 * remainder each. At DF_EMULATION 0x10000 that is PS and ITS 31,744 B apiece, about 10x what
 * the RA6 parts get from 8 KB of data flash. The program unit is the SAME 4 as theirs - MRAM
 * writes single bytes - so the backends behave identically; only the capacity differs.
 * ============================================================================== */
#define FLASH_DEV_NAME_DATA             Driver_FLASH1

/* 4, NOT the 32 that MCUboot uses - and this is load-bearing, not cosmetic.
 *
 * MRAM writes single bytes (see TFM_HAL_FLASH_PROGRAM_UNIT above), so any small value is
 * honest. 4 matches what RA6E1/RA6M5 advertise for their data flash, which keeps the ITS and
 * PS backends on the same code path across all three ports.
 *
 * WHY IT MATTERS: its_flash.c selects its backend on this number.
 *
 *     #elif (TFM_HAL_ITS_PROGRAM_UNIT > 16)
 *     #ifndef ITS_FLASH_NAND_BUF_SIZE
 *     #error "ITS_FLASH_NAND_BUF_SIZE must be defined by the target in flash_layout.h"
 *
 * Above 16 it switches to the NAND emulation, which needs ITS_FLASH_NAND_BUF_SIZE and
 * PS_FLASH_NAND_BUF_SIZE and allocates TWO static buffers of that size for each of ITS and
 * PS. At this port's block size that is 4 x 15,872 = about 62 KB of secure RAM bought for
 * nothing, since the hardware never needed the coarse unit. Setting 32 here is what first
 * revealed the distinction - the build stopped on exactly that #error.
 *
 * Both backends also CHECK this at run time against the driver's advertised program_unit
 * (tfm_internal_trusted_storage.c:210 and :246, returning PSA_ERROR_PROGRAMMER_ERROR), so
 * Driver_Flash.c's DataFlashInfo must derive from this macro and not carry its own literal. */
#define TFM_HAL_DATA_FLASH_PROGRAM_UNIT (4)

/* DF_EMULATION geometry, as cmsis_drivers/Driver_Flash.c names it.
 *
 * Driver_FLASH1 is a SECOND INSTANCE OVER THE SAME MRAM DEVICE, windowed to this region
 * - not a second physical device as it was on RA6M5. The offsets the PS/ITS backends
 * pass are relative to this base, so the driver adds it; the region is in the secure
 * alias and is never reachable from the non-secure side.
 *
 * There is no erase sector to respect, so 64 B is a logical choice carried over from
 * RA6M5's data flash rather than a device property. It must stay a multiple of
 * TFM_HAL_DATA_FLASH_PROGRAM_UNIT. */
#define FLASH_DATA_FLASH_BASE           (BSP_PARTITION_DF_EMULATION_START)
#define FLASH_DATA_FLASH_SIZE           (BSP_PARTITION_DF_EMULATION_SIZE)
#define FLASH_DATA_FLASH_SECTOR_SIZE    (64)

#define TFM_NV_COUNTERS_AREA_OFFSET     (BSP_PARTITION_DF_EMULATION_START)

/* 2048 B = 32 sectors, split into a 1024 B area and a 1024 B backup mirror.
 *
 * The floor is sizeof(struct flash_otp_nv_counters_region_t), which the backend requires
 * to fit in EACH half. That is 784 B for this configuration:
 *
 *     init_value 4 | OTP items 284 | BL2 ROTPKs x4 + BL2 NV counters x4 384
 *     | entropy_seed + secure_debug_pk 96 | flash_nv_counters[3] 12 | swap_count 4
 *
 * It grows with MCUBOOT_BUILTIN_KEY (ROTPKs become 68/100 B each, not a 32 B hash),
 * PLATFORM_NS_NV_COUNTERS (64 B each, currently 0) and FLASH_NV_COUNTER_AM. On RA6E1
 * this was 512 B total, i.e. 256 B per half - far under the floor - and the effect was
 * invisible: init_otp_nv_counters_flash() opens with a compile-time-constant size check,
 * so the whole function folded to "return TFM_PLAT_ERR_SYSTEM_ERR" and never touched
 * flash. ra8m2_layout_checks.c makes that a build error instead.
 *
 * NOTE for this part: R_MRAM also exposes HARDWARE anti-rollback counters
 * (R_MRAM_AntiRollbackCounterIncrement / Refresh / Read). They are a better home for
 * MCUboot's NV counters than an emulated-OTP region, and are not used here - the port
 * keeps the portable backend so RA8M2 behaves as RA6M5 does for bring-up. Worth
 * revisiting once the port boots.
 */
#define TFM_NV_COUNTERS_AREA_SIZE       (FLASH_DATA_FLASH_SECTOR_SIZE * 32)   /* 2048 B */

/* platform/ext/common/template/flash_otp_nv_counters_backend.c is the backend, and it
 * mirrors the area so a power loss mid-write is recoverable - hence the backup, which
 * must be a separate erase sector. The two halves split TFM_NV_COUNTERS_AREA_SIZE. */
#define TFM_OTP_NV_COUNTERS_SECTOR_SIZE      (FLASH_DATA_FLASH_SECTOR_SIZE)
#define TFM_OTP_NV_COUNTERS_AREA_SIZE        (TFM_NV_COUNTERS_AREA_SIZE / 2)
#define TFM_OTP_NV_COUNTERS_AREA_ADDR        (TFM_NV_COUNTERS_AREA_OFFSET)
#define TFM_OTP_NV_COUNTERS_BACKUP_AREA_ADDR (TFM_OTP_NV_COUNTERS_AREA_ADDR + \
                                              TFM_OTP_NV_COUNTERS_AREA_SIZE)
#define OTP_NV_COUNTERS_WRITE_BLOCK_SIZE     (TFM_HAL_DATA_FLASH_PROGRAM_UNIT)

#define TFM_HAL_PS_FLASH_AREA_ADDR      (TFM_NV_COUNTERS_AREA_OFFSET + \
                                         TFM_NV_COUNTERS_AREA_SIZE)
#define TFM_HAL_PS_FLASH_AREA_SIZE      ((BSP_PARTITION_DF_EMULATION_SIZE - \
                                          TFM_NV_COUNTERS_AREA_SIZE) / 2)
#define TFM_HAL_PS_SECTOR_SIZE          (FLASH_DATA_FLASH_SECTOR_SIZE)
#define PS_RAM_FS_SIZE                  TFM_HAL_PS_FLASH_AREA_SIZE

/* Half the area per logical block, i.e. two blocks, which is the minimum the flash FS
 * needs to rotate between on a write. Derived rather than fixed so it still holds if
 * DF_EMULATION is resized in the solution. */
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

/* DF_EMULATION must hold NV counters + PS + ITS with room for wear levelling. If this
 * fires, give DF_EMULATION more space in the solution rather than shrinking the
 * services here - and note that on a 1 MB device the space has to come from a slot, so
 * check the image sizes still fit (RA8M2_SOLUTION.md has the measured figures). */
#if (BSP_PARTITION_DF_EMULATION_SIZE) < 0x1000
#error "RA8M2: DF_EMULATION is too small for NV counters + PS + ITS"
#endif

/* This part genuinely has no data flash; if a future device in the family does, the
 * services should move back onto it rather than staying in MRAM. Catch that change. */
#if (BSP_PARTITION_DATA_FLASH_CPU0_S_SIZE) != 0
#error "RA8M2: secure data flash is no longer 0 - storage should move off DF_EMULATION"
#endif

/* Flash device IDs used by the CMSIS flash driver shim.
 *
 * Deliberately NO FLASH_DEV_NAME_0 / FLASH_DEV_NAME_1. Those are per-flash-area driver
 * overrides for a target whose images live on different devices; all four of our areas
 * are in MRAM, so bl2/src/default_flash_map.c defaulting them to FLASH_DEV_NAME is what
 * we want. Defining a _0/_1 name without the matching FLASH_DEVICE_ID_0/_1 also trips
 * the paired-definition check in bl2/ext/mcuboot/include/target.h. */
#define FLASH_DEVICE_ID                 (100)
#define FLASH_DEVICE_ID_DATA            (101)

#endif /* __FLASH_LAYOUT_H__ */
