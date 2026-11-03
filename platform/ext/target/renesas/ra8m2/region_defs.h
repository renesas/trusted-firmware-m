/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __REGION_DEFS_H__
#define __REGION_DEFS_H__

#include "flash_layout.h"     /* pulls in bsp_partitions.h */

/*
 * RA8M2 memory regions, derived from the e2 solution; see flash_layout.h. Do not edit them
 * by hand: change the partitions in solution.xml and rebuild in e2.
 */

#define TOTAL_ROM_SIZE                  (FLASH_TOTAL_SIZE)
#define TOTAL_RAM_SIZE                  (BSP_PARTITION_RAM_CPU0_S_SIZE + \
                                         BSP_PARTITION_RAM_CPU0_C_SIZE + \
                                         BSP_PARTITION_RAM_CPU0_N_SIZE)

#define S_ROM_ALIAS_BASE                (BSP_PARTITION___BL_0_P_H_START)
#define S_RAM_ALIAS_BASE                (BSP_PARTITION_RAM_CPU0_S_START)
#define NS_ROM_ALIAS_BASE               (BSP_PARTITION___BL_1_P_H_START)
#define NS_RAM_ALIAS_BASE               (BSP_PARTITION_RAM_CPU0_N_START)

/*
 * MCUboot header and trailer sizes, from the solution's partitioning.
 *
 * CMake also passes these as -D, and imgtool signs with the CMake values, so the two must
 * agree. TF-M defaults both to 0x400. With a 0x400 header imgtool places the payload 0x400
 * into the slot while the image is linked for slot+0x200; the signed image is offset by 0x200
 * and does not boot, with no other symptom. config.cmake reads the real values from
 * bsp_linker_info.h only on a clean CMake cache, so this checks rather than redefines.
 */
#if defined(BL2_HEADER_SIZE) && (BL2_HEADER_SIZE != BSP_PARTITION___BL_0_P_H_SIZE)
#error "BL2_HEADER_SIZE from CMake disagrees with the solution. Configure with a clean CMake cache."
#endif
#if defined(BL2_TRAILER_SIZE) && (BL2_TRAILER_SIZE != BSP_PARTITION___BL_0_P_T_SIZE)
#error "BL2_TRAILER_SIZE from CMake disagrees with the solution. Configure with a clean CMake cache."
#endif

#ifndef BL2_HEADER_SIZE
#define BL2_HEADER_SIZE                 (BSP_PARTITION___BL_0_P_H_SIZE)
#endif
#ifndef BL2_TRAILER_SIZE
#define BL2_TRAILER_SIZE                (BSP_PARTITION___BL_0_P_T_SIZE)
#endif

/*
 * Secure image. FLASH_CPU0_S is the code; FLASH_CPU0_C holds the non-secure-callable veneers
 * at its end. The solution places NSC at the end of secure flash, so no linker macros are
 * needed to put it there.
 */
#define S_CODE_START                    (BSP_PARTITION_FLASH_CPU0_S_START)
#define S_CODE_SIZE                     (BSP_PARTITION_FLASH_CPU0_S_SIZE + \
                                         BSP_PARTITION_FLASH_CPU0_C_SIZE)
#define S_CODE_LIMIT                    (S_CODE_START + S_CODE_SIZE - 1)

/*
 * Secure RAM.
 *
 * Unlike RA6E1 and RA6M5, this part has no code-from-RAM window. Those parts need one because
 * r_flash_hp carries PLACE_IN_RAM_SECTION on its code-flash program/erase routines; see
 * ra6m5/region_defs.h, S_RAM_CODE_SIZE.
 *
 * MRAM does not become unreadable during programming, and FSP's code reflects this:
 *
 *   - PLACE_IN_RAM_SECTION appears in no FSP source in this project. r_mram.h defines the
 *     macro (copied from r_flash_hp.h, comment and all, including a reference to a "Code
 *     Flash programming is enabled" option that r_mram_cfg.h does not have), but r_mram.c
 *     never applies it. RA6M5's r_flash_hp.c applies it 17 times.
 *   - In the e2-built secure image the linked block is empty: "__ram_from_flash 0x2200'0000
 *     0x0 <Block>" in the map.
 *
 * So FSP's MRAM driver executes from MRAM while programming MRAM, which it could not do if the
 * array went unreadable. This is inferred from FSP's behaviour, not quoted from a datasheet.
 * To undo it, define S_RAM_CODE_SIZE and S_RAM_CODE_START again (taking the space off the top
 * of the secure RAM partition, as RA6M5 does); TF-M's generated linker script then re-emits
 * .ER_CODE_SRAM with its copy-table entry. The mechanism is gated only on S_RAM_CODE_START
 * being defined.
 *
 * DECISIONS D029/D048 (ILINK declining to copy-init .ram_from_flash at isolation 3, leaving
 * the P/E routines in flash while isolation 1 relocated them) cannot arise here, because there
 * is nothing to copy.
 *
 * THREE FILES HAVE TO AGREE, and all three now say nothing:
 *   - this file             - no S_RAM_CODE_START, so TF-M's template creates no ER_CODE_SRAM
 *   - ra8m2_fsp_sections.icf - places no .ram_from_flash (naming it would target a block that
 *                              does not exist)
 *   - tfm_hal_platform.c    - no ra_ram_code_init() helper and no call
 * They were briefly left in place as inert no-ops; that is worse than removing them, because
 * __section_begin() on a section nothing creates is a link-time question rather than a
 * no-op, and a reader cannot tell an intentional stub from an oversight. The recipe is
 * at the top of ra8m2_fsp_sections.icf.
 *
 * Symptom if the inference is wrong: a hang, not a fault, on the first Protected Storage or
 * ITS write. Those live in DF_EMULATION, which is MRAM (flash_layout.h), so unlike RA6M5 the
 * secure image programs MRAM on a normal code path, not only on an armed but unused one. If
 * that happens, restore the window first.
 */
#define S_DATA_START                    (BSP_PARTITION_RAM_CPU0_S_START)
#define S_DATA_SIZE                     (BSP_PARTITION_RAM_CPU0_S_SIZE)
#define S_DATA_LIMIT                    (S_DATA_START + S_DATA_SIZE - 1)

/* IAR reaches the platform's vendor sections through literal ICF text, because a section
 * name cannot survive iccarm's preprocessor as a macro. See ra8m2_fsp_sections.icf and the
 * note at the hook in platform/ext/common/iar/tfm_isolation_s.icf.template. */
#ifdef __ICCARM__
#define S_ICF_PLATFORM_SECTIONS         "ra8m2_fsp_sections.icf"
#endif

/* FSP's do-not-initialise sections; see ra6m5/region_defs.h,
 * S_DATA_EXTRA_NOINIT_SECTION_NAME. */
#define S_DATA_EXTRA_NOINIT_SECTION_NAME   .ram_noinit* .noinit*

/* RA8M2 has 96 IRQs; the vector table comes from ra_gen/vector_data.c. */
#define S_CODE_VECTOR_TABLE_SIZE        (0x200)

/* Non-secure image. */
#define NS_CODE_START                   (BSP_PARTITION_FLASH_CPU0_N_START)
#define NS_CODE_SIZE                    (BSP_PARTITION_FLASH_CPU0_N_SIZE)
#define NS_CODE_LIMIT                   (NS_CODE_START + NS_CODE_SIZE - 1)

#define NS_DATA_START                   (BSP_PARTITION_RAM_CPU0_N_START)
#define NS_DATA_SIZE                    (BSP_PARTITION_RAM_CPU0_N_SIZE)
#define NS_DATA_LIMIT                   (NS_DATA_START + NS_DATA_SIZE - 1)

#define NS_PARTITION_START              (BSP_PARTITION___BL_1_P_H_START)
#define NS_PARTITION_SIZE               (FLASH_AREA_1_SIZE)

/*
 * Secure secondary (staging) slot, FLASH_AREA_2 (image 0 secondary). TF-M's generated secure
 * linker script emits the LR_SECONDARY_PARTITION load-region symbol from these, and
 * target_cfg.c publishes them in memory_regions.
 *
 * These take an address, not an offset, and on this part the two differ. Three consumers use
 * the value as an address: "place at address SECONDARY_PARTITION_START" in the IAR ICF,
 * "Load$$LR$$LR_SECONDARY_PARTITION$$Base =" in the GNU script, and
 * .secondary_partition_base in target_cfg.c. It therefore takes the partition's START, not
 * FLASH_AREA_2_OFFSET.
 *
 * FLASH_AREA_*_OFFSET is MCUboot's flat offset into the device, with the security alias base
 * subtracted (flash_layout.h, TFM_MRAM_S_OFF / TFM_MRAM_NS_OFF). MRAM is based at 0x02000000,
 * so the offset 0x20000 and the address 0x02020000 are 32 MB apart. On RA6E1 and RA6M5 code
 * flash is based at 0x00000000, and the two coincide.
 *
 * Sizes are alias-independent, so FLASH_AREA_2_SIZE is used as is.
 */
#define SECONDARY_PARTITION_START       (BSP_PARTITION___BL_0_S_H_START)
#define SECONDARY_PARTITION_SIZE        (FLASH_AREA_2_SIZE)

/*
 * Non-secure callable / CMSE veneers.
 *
 * The solution owns this window (FLASH_CPU0_C): 0x400 at 0x020AFC00, ending exactly on the
 * 32 KB secure/non-secure boundary at 0x020B0000 (0xB0000 = 22 x 32 KB).
 *
 * The Renesas Device Partition Manager (RDPM), not the e2 Partition Manager, takes sizes in
 * KB, not these addresses, and counts NSC separately from secure:
 *
 *   code flash   secure 703 KB + NSC 1 KB   (= 0xB0000, the whole secure MRAM span)
 *   SRAM         secure 935 KB + NSC 1 KB   (= 0xEA000)
 *
 * BSP_FEATURE_TZ_HAS_DLM is 1, so FSP compiles out its runtime PSCU monitor writes, and the
 * partition is a provisioned, non-volatile device property. Wrong KB values mis-provision the
 * device, and provisioning does not come back. Do not derive an IDAU boundary from these
 * macros without reading DESIGN.md §7.1 first.
 */
#define CMSE_VENEER_REGION_START        (BSP_PARTITION_FLASH_CPU0_C_START)
#define CMSE_VENEER_REGION_SIZE         (BSP_PARTITION_FLASH_CPU0_C_SIZE)
#define CMSE_VENEER_REGION_LIMIT        (CMSE_VENEER_REGION_START + \
                                         CMSE_VENEER_REGION_SIZE - 1)

/* Pin the veneers where the solution places them, in TF-M's generated linker script. */
#ifndef TFM_LINKER_VENEERS_START
#define TFM_LINKER_VENEERS_START        CMSE_VENEER_REGION_START
#endif
#ifndef TFM_LINKER_VENEERS_LOCATION_END
#define TFM_LINKER_VENEERS_LOCATION_END
#endif
#ifndef TFM_LINKER_VENEERS_SIZE
#define TFM_LINKER_VENEERS_SIZE         CMSE_VENEER_REGION_SIZE
#endif

/*
 * Give the veneers their own MEMORY region in TF-M's generated secure linker script.
 *
 * TFM_LINKER_VENEERS_START is absolute: the solution fixes the NSC window at the top of the
 * secure partition. Without a separate region, `.gnu.sgstubs <abs> : ... > FLASH` moves the
 * FLASH region's allocation pointer up there, and every later `AT > FLASH` section (.TFM_DATA
 * and the other RAM-init load images) gets an LMA in the few spare bytes above the veneers,
 * while the flash below them goes unused. The app build places about 200 bytes of load images
 * there and fits; the regression tests overflow by a few hundred bytes with hundreds of KB
 * free (DECISIONS D063).
 */
#ifndef TFM_LINKER_VENEERS_OWN_REGION
#define TFM_LINKER_VENEERS_OWN_REGION
#endif

/*
 * BL2 is the bootloader and lives at the base of flash, where the reset vector is. Deriving it
 * from the secure image base puts BL2 in the secure slot, and the device never runs it
 * (DESIGN.md §8.3).
 *
 * BL2 owns all secure RAM while it runs. The solution's RAM_BL partition
 * (BSP_PARTITION_RAM_BL_CPU0_S_SIZE) is the budget of the solution's own bootloader and is
 * smaller than TF-M's BL2 needs, so BL2 uses the whole secure RAM partition.
 */
#define BL2_CODE_START                  (BSP_PARTITION_FLASH_BL_CPU0_S_START)
#define BL2_CODE_SIZE                   (BSP_PARTITION_FLASH_BL_CPU0_S_SIZE)
#define BL2_CODE_LIMIT                  (BL2_CODE_START + BL2_CODE_SIZE - 1)

#define BL2_DATA_START                  (BSP_PARTITION_RAM_CPU0_S_START)
#define BL2_DATA_SIZE                   (BSP_PARTITION_RAM_CPU0_S_SIZE)
#define BL2_DATA_LIMIT                  (BL2_DATA_START + BL2_DATA_SIZE - 1)

#define BL2_HEAP_SIZE                   (0x1000)
#define BL2_MSP_STACK_SIZE              (0x1800)

/* Shared boot data (BL2 -> secure). */
#define BOOT_TFM_SHARED_DATA_SIZE       (0x400)
#define BOOT_TFM_SHARED_DATA_BASE       (S_DATA_START)
#define BOOT_TFM_SHARED_DATA_LIMIT      (BOOT_TFM_SHARED_DATA_BASE + \
                                         BOOT_TFM_SHARED_DATA_SIZE - 1)

/*
 * BL2 writes the measurement TLVs through MCUBOOT_SHARED_DATA_BASE (aliased to these in
 * bl2/ext/mcuboot/include/flash_map/flash_map.h), and the SPM reads them at
 * SHARED_BOOT_MEASUREMENT_BASE in tfm_boot_data.c. Both must name the same region, so they
 * are aliased onto the shared data area, the upstream convention (rp2350, psoc64, the stm32
 * family).
 */
#define SHARED_BOOT_MEASUREMENT_BASE    (BOOT_TFM_SHARED_DATA_BASE)
#define SHARED_BOOT_MEASUREMENT_SIZE    (BOOT_TFM_SHARED_DATA_SIZE)

#define S_HEAP_SIZE                     (0x800)

/* Must be BSP_CFG_STACK_MAIN_BYTES + STACKSEAL_SIZE (0x1000 + 8 for this solution), so that
 * FSP's SystemInit() stack seal lands exactly on __StackSeal, not in live data above it.
 * ra8m2_layout_checks.c asserts this; its "Main stack" section explains why. */
#define S_MSP_STACK_SIZE                (0x1008)
#define S_PSP_STACK_SIZE                (0x800)
#define NS_HEAP_SIZE                    (0x1000)
#define NS_STACK_SIZE                   (0x1000)

/* Peripherals: the whole space. Security attribution comes from the IDAU/SAU, not these. */
#define PERIPHERAL_BASE                 (0x40000000)
#define PERIPHERAL_SIZE                 (0x10000000)
#define PERIPHERALS_BASE_S              (PERIPHERAL_BASE)
#define PERIPHERALS_LIMIT_S             (PERIPHERAL_BASE + PERIPHERAL_SIZE - 1)
#define PERIPHERALS_BASE_NS             (PERIPHERAL_BASE)
#define PERIPHERALS_LIMIT_NS            (PERIPHERAL_BASE + PERIPHERAL_SIZE - 1)

/*
 * RA8M2 option-setting memory: discrete regions, one per register group.
 *
 * The addresses are deliberately not in this file. They come from option_settings.h, which the
 * platform CMakeLists generates at configure time from the solution's own memory_regions.icf,
 * included below. Do not hand-write them here: RA6M5's values (OFS0 at 0x0100A100) are not
 * this device's option memory (OFS0 at 0x02c9f040), and an image linked with them never
 * writes the real OFS and block-protect registers.
 *
 * RA8M2's set differs from RA6's: 26 groups against 13. OFS2, OFS3 (+_SEC/_SEL) and SAS are
 * new; there is a separate OTP block (FSBLCTRL0-2, SAMR, SACC00-13, PBPS +_SEC, ZHUK); and
 * DUALSEL, BANKSEL and the non-OTP PBPS do not exist here. BPS is 0x80, not 0x10.
 *
 * WARNING: the placement rule is the brick hazard. These must stay separate regions in
 * ra8m2_bl2.ld / .icf, each section assigned with '> REGION'. One coalesced PT_LOAD zero-fills
 * every register and reserved byte in the gaps, including the block-protect permanence word,
 * and bricks the part with no known recovery (DECISIONS D002).
 *
 * The fill is invisible in the srec: it is in the program header, and an srec is emitted from
 * sections. Verify the segments, not the sections:
 *     arm-none-eabi-readelf -l bin/bl2.axf
 * Expect small separate LOAD segments in 0x02c9fxxx, never one spanning several groups.
 * Rationale: DESIGN.md §8.4.
 */
#include "option_settings.h"

#endif /* __REGION_DEFS_H__ */
