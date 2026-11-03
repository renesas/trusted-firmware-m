/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __FLASH_LAYOUT_H__
#define __FLASH_LAYOUT_H__

/*
 * RA8M2 flash layout, derived from the e2 solution. Do not edit it by hand.
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
 * into ra8m2_bl2.ld, and a linker script cannot parse the C types and externs in
 * bsp_linker_info.h.
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
 * Differences from RA6M5, from which this port was derived:
 *
 *   1. Storage is MRAM, not code flash. The write unit is 32 B, not 128; the MCUboot sector
 *      stays 32 KB, which FSP defines as RM_MCUBOOT_MRAM_BLOCK_SIZE.
 *   2. There is no data flash (DATA_FLASH_CPU0_S_SIZE is 0), so ITS, PS and the NV counters
 *      use a reserved MRAM region, DF_EMULATION.
 *   3. BSP_PARTITION_*_START values are absolute addresses in their own security alias, not
 *      offsets; see TFM_MRAM_S_OFF / TFM_MRAM_NS_OFF.
 *   4. As on RA6E1/RA6M5, the secure secondary slot sits below the primary, so the primary is
 *      the topmost secure slot and its NSC region ends exactly at the secure/non-secure
 *      boundary. RDPM expresses only a contiguous Secure|NSC|NS triple, not the split the SAU
 *      supports.
 * ----------------------------------------------------------------------------------
 */

#include "bsp_partitions.h"

/* ---------------------------------------------------------------------------------
 * Device geometry - MRAM.
 *
 * MRAM is aliased twice, secure at 0x02000000 and non-secure at 0x12000000, 1 MB each over the
 * same physical array. bsp_linker_info.h emits each partition start as an absolute address in
 * the alias its security attribute selects: 0x020xxxxx for secure partitions, 0x120xxxxx for
 * non-secure ones. MCUboot wants one flat offset space, so each start is converted below by
 * subtracting its own alias base, and one Driver_FLASH0 serves all four slots.
 *
 * The reverse does not hold. An offset is alias-independent; the address formed from it is
 * not. Once RDPM has partitioned the part, cells below the boundary answer only on the secure
 * alias and cells at or above it only on the non-secure alias, so a driver cannot add one
 * fixed base to every offset. Driver_Flash.c's MRAM_ADDR() chooses the alias per offset; see
 * the comment there.
 *
 * RA6M5 needs none of this: its code flash is based at 0x00000000, so the generated starts
 * are also offsets.
 * ------------------------------------------------------------------------------- */
#define FLASH_BASE_ADDRESS              (0x02000000)     /* MRAM, secure alias */
#define FLASH_NS_ALIAS_BASE             (0x12000000)     /* MRAM, non-secure alias */
#define FLASH_TOTAL_SIZE                (0x00100000)     /* 1 MB */
#define FLASH_DEV_NAME                  Driver_FLASH0

#define TFM_MRAM_S_OFF(addr)            ((addr) - (FLASH_BASE_ADDRESS))
#define TFM_MRAM_NS_OFF(addr)           ((addr) - (FLASH_NS_ALIAS_BASE))

/* MCUboot's write unit: BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES, 32.
 *
 * WARNING: 32 is the size of the MRAM programming buffer, not a minimum write; read this
 * before reusing the number. MRAM write granularity is one byte: r_mram.c's flash_info reports
 * block_size 32 (the erase block) and block_size_write 1. mram_write_data() copies a byte at a
 * time and flushes a partial buffer through MRCFLR (the FSP comment there cites the hardware
 * manual's "smaller than 32-bytes programming" procedure), and
 * r_mram_write_erase_parameter_checking() enforces no alignment or multiple.
 *
 * It is 32 here because FSP chooses it for MCUboot: the generated mcuboot_config.h sets
 * MCUBOOT_BOOT_MAX_ALIGN from the same BSP feature. Driver_Flash.c advertises it as FLASH0's
 * program_unit, and bl2/src/flash_map.c returns it as flash_area_align(). config.cmake must
 * keep MCUBOOT_ALIGN_VAL equal to it: the images are signed with --align, and the trailer
 * geometry must match at run time. Consume FSP's choice; do not override it.
 *
 * The ITS/PS program unit is not 32; see TFM_HAL_DATA_FLASH_PROGRAM_UNIT below. */
#define TFM_HAL_FLASH_PROGRAM_UNIT      (32)
#define TFM_HAL_CODE_FLASH_PROGRAM_UNIT (32)

/* Sector size, taken from FSP, not chosen here.
 *
 * FSP's generated ra_cfg/mcu-tools/include/mcuboot_config/mcuboot_config.h has the
 * authoritative value:
 *
 *     #if BSP_FEATURE_MRAM_IS_AVAILABLE
 *      #define RM_MCUBOOT_MRAM_BLOCK_SIZE     (0x8000)
 *      #define FLASH_AREA_IMAGE_SECTOR_SIZE   (RM_MCUBOOT_MRAM_BLOCK_SIZE)
 *      #define MCUBOOT_BOOT_MAX_ALIGN         (RM_MCUBOOT_MRAM_PROGRAMMING_SIZE_BYTES)
 *
 * That directory is on this build's include path (cmake/modules/fsp_bsp.cmake), so a
 * different value here is a conflicting redefinition. The port consumes FSP-generated
 * configuration rather than restating it (DESIGN.md §1).
 *
 * The sector and the write unit are different numbers, and both matter:
 *   - 0x8000 is the MCUboot sector, which flash_area_get_sectors() walks.
 *   - 32 is the MRAM write unit (BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES), which FSP uses for
 *     MCUBOOT_BOOT_MAX_ALIGN and MCUBOOT_ALIGN_VAL matches.
 * r_mram.c's flash_info block_size is 32, not the sector size. Reading it as the sector size
 * produced slots 9.875 sectors long, which every build step accepted and which failed only on
 * hardware, as boot_read_sectors() returning BOOT_EFLASH.
 *
 * Every MCUboot area's offset and size must be a whole multiple of this. On a 1 MB device that
 * constrains the partitioning: with BL2 at 64 KB, DF_EMULATION must be 64 KB, because 8 or
 * 32 KB leaves a remainder that cannot split into two secure and two non-secure slots on a
 * 32 KB boundary. ra8m2_layout_checks.c asserts that sizing.
 *
 * The 0x8000 itself is not asserted against FSP; see ra8m2_layout_checks.c,
 * "FLASH_AREA_IMAGE_SECTOR_SIZE against FSP". After an FSP uprev, diff the generated
 * mcuboot_config.h.
 *
 * It is a literal, not an include of mcuboot_config.h, because this header is also
 * preprocessed into ra8m2_bl2.ld and mcuboot_config.h includes bsp_api.h.
 * TODO: extract it into the generated bsp_partitions.h, as is done for the partitions. */
#define FLASH_AREA_IMAGE_SECTOR_SIZE    (0x8000)         /* == RM_MCUBOOT_MRAM_BLOCK_SIZE */

/* BL2 */
#define FLASH_AREA_BL2_OFFSET           TFM_MRAM_S_OFF(BSP_PARTITION_FLASH_BL_CPU0_S_START)
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
 * Both ends are in the same security alias, so the alias base cancels and no TFM_MRAM_*_OFF
 * conversion is needed.
 *
 * The trailers are zero-size end markers, as on RA6E1 and RA6M5. A real trailer after
 * FLASH_CPU0_C would put 256 secure bytes between the NSC region and the non-secure boundary,
 * which RDPM cannot describe. MCUboot takes its trailer from the slot end backwards, out of
 * the image region.
 *
 * ra8m2_layout_checks.c asserts contiguity and whole-sector sizing at build time. */
#define TFM_SLOT_SPAN(h, t)             (((t##_START) + (t##_SIZE)) - (h##_START))

/* Image 0 = secure. Secondary at 0x20000, primary at 0x68000, 0x48000 each; the secondary is
 * the lower slot. */
#define FLASH_AREA_0_ID                 (1)
#define FLASH_AREA_0_OFFSET             TFM_MRAM_S_OFF(BSP_PARTITION___BL_0_P_H_START)
#define FLASH_AREA_0_SIZE               TFM_SLOT_SPAN(BSP_PARTITION___BL_0_P_H, \
                                                      BSP_PARTITION___BL_0_P_T)
#define FLASH_AREA_2_ID                 (FLASH_AREA_0_ID + 2)
#define FLASH_AREA_2_OFFSET             TFM_MRAM_S_OFF(BSP_PARTITION___BL_0_S_H_START)
#define FLASH_AREA_2_SIZE               TFM_SLOT_SPAN(BSP_PARTITION___BL_0_S_H, \
                                                      BSP_PARTITION___BL_0_S_T)

/* Image 1 = non-secure. Primary at 0xB0000, secondary at 0xD8000, 0x28000 each. These use the
 * non-secure alias base; see "Device geometry" above. */
#define FLASH_AREA_1_ID                 (FLASH_AREA_0_ID + 1)
#define FLASH_AREA_1_OFFSET             TFM_MRAM_NS_OFF(BSP_PARTITION___BL_1_P_H_START)
#define FLASH_AREA_1_SIZE               TFM_SLOT_SPAN(BSP_PARTITION___BL_1_P_H, \
                                                      BSP_PARTITION___BL_1_P_T)
#define FLASH_AREA_3_ID                 (FLASH_AREA_0_ID + 3)
#define FLASH_AREA_3_OFFSET             TFM_MRAM_NS_OFF(BSP_PARTITION___BL_1_S_H_START)
#define FLASH_AREA_3_SIZE               TFM_SLOT_SPAN(BSP_PARTITION___BL_1_S_H, \
                                                      BSP_PARTITION___BL_1_S_T)

/* The solution is overwrite-only, so there is no scratch area.
 *
 * Unlike RA6M5, no block is reserved for one. The 1 MB device is fully allocated
 * (64 K BL2 + 64 K DF_EMULATION + 2 x 288 K secure + 2 x 160 K non-secure = 1024 K exactly),
 * so swap-using-scratch needs the solution repartitioned first, not only these three macros
 * repointed. A scratch area must be secure: during a swap it holds fragments of the secure
 * image, so an NS-writable scratch would leak and corrupt it. */
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
 * of the combined image. Adjacency follows from putting the secure primary slot topmost so
 * that its NSC region touches the boundary; it is not a goal in itself.
 * ra8m2_layout_checks.c asserts it, because it is a property of the current partitioning, not
 * a rule.
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
 * sized from the largest slot, rounded up: 0x48000 / 0x8000 = 9.
 *
 * FSP computes its own value as RM_MCUBOOT_LARGEST_SECTOR / FLASH_AREA_IMAGE_SECTOR_SIZE over
 * its partition view. TF-M has its own flash map, so this one is derived from TF-M's areas:
 * the sector size is shared, the sector count deliberately is not. */
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
 * RA8M2 has no data flash: BSP_PARTITION_DATA_FLASH_CPU0_S_SIZE is 0, and the rzone gives the
 * whole DATA_FLASH region size 0. On RA6E1 and RA6M5 these services share 0x2000 of secure
 * data flash; here they share DF_EMULATION, an 0x10000 region of secure MRAM at 0x02010000,
 * between BL2 and the secure secondary slot.
 *
 * ra8m2_layout_checks.c asserts that DF_EMULATION is a whole number of 0x8000 MRAM sectors,
 * so its size cannot drift from the solution without failing the build.
 *
 * DF_EMULATION is deliberately outside every MCUboot area. An update overwrites a whole slot,
 * so storage inside one would be destroyed by the first image upgrade; for the same reason it
 * cannot use the slack in a slot.
 *
 * The split follows RA6M5's proportions: NV counters 2048 B, then PS and ITS take half the
 * remainder each. At DF_EMULATION 0x10000 that is 31,744 B each for PS and ITS, about 10x what
 * the RA6 parts get from 8 KB of data flash. The program unit is 4, as on those parts, because
 * MRAM writes single bytes; the backends behave identically and only the capacity differs.
 * ============================================================================== */
#define FLASH_DEV_NAME_DATA             Driver_FLASH1

/* 4, not the 32 that MCUboot uses. This value selects the ITS/PS backend.
 *
 * MRAM writes single bytes (see TFM_HAL_FLASH_PROGRAM_UNIT above), so any small value is
 * valid. 4 matches the RA6E1/RA6M5 data flash and keeps the ITS and PS backends on the same
 * code path across all three ports.
 *
 * its_flash.c selects its backend on this number:
 *
 *     #elif (TFM_HAL_ITS_PROGRAM_UNIT > 16)
 *     #ifndef ITS_FLASH_NAND_BUF_SIZE
 *     #error "ITS_FLASH_NAND_BUF_SIZE must be defined by the target in flash_layout.h"
 *
 * Above 16 it switches to NAND emulation, which needs ITS_FLASH_NAND_BUF_SIZE and
 * PS_FLASH_NAND_BUF_SIZE and allocates two static buffers of that size for each of ITS and
 * PS. At this port's block size that is 4 x 15,872 = about 62 KB of secure RAM for no benefit,
 * since the hardware does not need the coarse unit. A value of 32 stops the build at that
 * #error.
 *
 * Both backends also check this at run time against the driver's advertised program_unit
 * (tfm_internal_trusted_storage.c:210 and :246, returning PSA_ERROR_PROGRAMMER_ERROR), so
 * Driver_Flash.c's DataFlashInfo must derive from this macro, not carry its own literal. */
#define TFM_HAL_DATA_FLASH_PROGRAM_UNIT (4)

/* DF_EMULATION geometry, as cmsis_drivers/Driver_Flash.c names it.
 *
 * Driver_FLASH1 is a second instance over the same MRAM device, windowed to this region, not a
 * second physical device as on RA6M5. The PS/ITS backends pass an absolute address and the
 * driver uses it as given, unlike FLASH0, which converts through MRAM_ADDR(). The region is in
 * the secure alias and unreachable from the non-secure side.
 *
 * There is no erase sector to respect, so 64 B is a logical size carried over from RA6M5's
 * data flash, not a device property. It must stay a multiple of
 * TFM_HAL_DATA_FLASH_PROGRAM_UNIT. */
#define FLASH_DATA_FLASH_BASE           (BSP_PARTITION_DF_EMULATION_START)
#define FLASH_DATA_FLASH_SIZE           (BSP_PARTITION_DF_EMULATION_SIZE)
#define FLASH_DATA_FLASH_SECTOR_SIZE    (64)

#define TFM_NV_COUNTERS_AREA_OFFSET     (BSP_PARTITION_DF_EMULATION_START)

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
 * ra8m2_layout_checks.c ("OTP / NV counter area size") makes that a build error.
 *
 * NOTE: R_MRAM also has hardware anti-rollback counters (R_MRAM_AntiRollbackCounterIncrement /
 * Refresh / Read), a better home for MCUboot's NV counters than an emulated-OTP region. They
 * are not used: the port keeps the portable backend so that RA8M2 behaves as RA6M5 does.
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

/* DF_EMULATION must hold the NV counters, PS and ITS with room for wear levelling. If this
 * fires, give DF_EMULATION more space in the solution rather than shrinking the services. On a
 * 1 MB device the space must come from a slot, so check that the image sizes still fit
 * (RA8M2_SOLUTION.md has the measured figures). */
#if (BSP_PARTITION_DF_EMULATION_SIZE) < 0x1000
#error "RA8M2: DF_EMULATION is too small for NV counters + PS + ITS"
#endif

/* RA8M2 has no data flash. If a future device in the family has it, the services should move
 * back onto it rather than stay in MRAM; this catches that change. */
#if (BSP_PARTITION_DATA_FLASH_CPU0_S_SIZE) != 0
#error "RA8M2: secure data flash is no longer 0 - storage should move off DF_EMULATION"
#endif

/* Flash device IDs used by the CMSIS flash driver shim.
 *
 * FLASH_DEV_NAME_0 / FLASH_DEV_NAME_1 are deliberately not defined. They are per-area driver
 * overrides for a target whose images live on different devices. All four areas are in MRAM, so
 * bl2/src/default_flash_map.c correctly defaults them to FLASH_DEV_NAME. Defining a _0/_1
 * name without the matching FLASH_DEVICE_ID_0/_1 also trips the paired-definition check in
 * bl2/ext/mcuboot/include/target.h. */
#define FLASH_DEVICE_ID                 (100)
#define FLASH_DEVICE_ID_DATA            (101)

#endif /* __FLASH_LAYOUT_H__ */
