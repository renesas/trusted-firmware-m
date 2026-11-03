/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __REGION_DEFS_H__
#define __REGION_DEFS_H__

#include "flash_layout.h"     /* pulls in bsp_partitions.h */

/*
 * RA6M5 memory regions, derived from the e2 solution; see flash_layout.h. Do not edit them
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
 * Secure RAM, split into data and a small code-from-RAM window.
 *
 * S_RAM_CODE holds FSP's .ram_from_flash: the r_flash_hp code-flash program/erase routines.
 * They carry PLACE_IN_RAM_SECTION because the FCU makes the entire code flash unreadable during
 * a code-flash P/E; run from code flash, they prefetch-abort mid-operation with the FCU left
 * in P/E mode. FSP deliberately does not RAM-place the data-flash path (flash_hp_df_write,
 * df_erase and enter_pe_df_mode have no attribute), because code flash stays readable during
 * data-flash P/E.
 *
 * The secure image uses only data flash: it instantiates Driver_FLASH1, not Driver_FLASH0.
 * FLASH_HP_CFG_CODE_FLASH_PROGRAMMING_ENABLE is 1 in ra6m5_secure, however, so the code-flash
 * path is compiled in and R_FLASH_HP_Write/Erase dispatch to it by address. The path is
 * armed, not absent; see RA6M5_SOLUTION.md.
 *
 * TF-M's generated secure linker script provides the mechanism, gated on S_RAM_CODE_START: it
 * places .ER_CODE_SRAM in the CODE_RAM region with a copy-table entry, and the CMSIS copy loop
 * in Reset_Handler relocates it before main(). S_RAM_CODE_EXTRA_SECTION_NAME is the hook for
 * a vendor section name; nothing upstream uses it. Defining these three macros is the whole
 * fix; no upstream file changes.
 *
 * Size: .ram_from_flash measures 0x7d0 in tfm_s (check with
 *   arm-none-eabi-objdump -h .../fsp_flash_s.dir/.../r_flash_hp.o | grep ram_from_flash).
 * The linked section is larger, because ld appends a long-branch veneer inside it for the RAM
 * code's call to flash_hp_enter_pe_df_mode, which stays in flash. 0x800 fits exactly, with
 * nothing to spare, so this is 0xA00, leaving room for a newer FSP to add a function. An
 * overflow still fails the link with "region CODE_RAM overflowed".
 *
 * The window is taken off the top of the secure RAM partition, so S_DATA_START and everything
 * derived from it (BOOT_TFM_SHARED_DATA_BASE) are unchanged. The solution is untouched:
 * BSP_PARTITION_RAM_CPU0_S_SIZE still describes the whole secure RAM and is only subdivided
 * here. BL2 derives BL2_DATA_* from the partition directly and is unaffected.
 *
 * Isolation level 1 leaves CONFIG_TFM_ENABLE_MEMORY_PROTECT off, so no MPU region is
 * programmed and PRIVDEFENA's default map permits execution from SRAM. Isolation level 2 or 3
 * needs an executable MPU region for CODE_RAM in tfm_hal_isolation_v8m.c;
 * Image$$ER_CODE_SRAM$$Base/Limit are emitted for that.
 */
#define S_RAM_CODE_SIZE                 (0xA00)

#define S_DATA_START                    (BSP_PARTITION_RAM_CPU0_S_START)
#define S_DATA_SIZE                     (BSP_PARTITION_RAM_CPU0_S_SIZE - S_RAM_CODE_SIZE)
#define S_DATA_LIMIT                    (S_DATA_START + S_DATA_SIZE - 1)

#define S_RAM_CODE_START                (S_DATA_START + S_DATA_SIZE)

/* The linker-script preprocessor expands this into KEEP(*(...)), so it must be a bare ld
 * pattern, not a quoted string. The trailing glob also catches .ram_from_flash.* if a future
 * FSP release splits the section; GCC currently emits exactly one. */
#define S_RAM_CODE_EXTRA_SECTION_NAME   .ram_from_flash*

/* IAR reaches the same two sections through literal ICF text, because a section name cannot
 * survive iccarm's preprocessor as a macro. See ra6m5_fsp_sections.icf and the note at the
 * hook in platform/ext/common/iar/tfm_isolation_s.icf.template. */
#ifdef __ICCARM__
#define S_ICF_PLATFORM_SECTIONS         "ra6m5_fsp_sections.icf"
#endif

/*
 * FSP's do-not-initialise sections, collected into .TFM_NOINIT: NOLOAD, and placed outside
 * __bss_start__..__bss_end__ so that the C runtime does not zero them.
 *
 * Both properties are required. Reset_Handler calls SystemInit() before __PROGRAM_START(), and
 * SystemInit() sets SystemCoreClock and the bsp_clocks state that live here. Folding these
 * sections into .TFM_BSS would zero clock state that is already initialised. FSP's own
 * fsp_gen.ld uses a NOLOAD output section for the same reason.
 *
 * Without this macro, ld orphans .ram_noinit: it creates an output section and, the input
 * being PROGBITS, gives it a load address (110 bytes of flash inside the 2 KB NSC window)
 * holding initialisers that no copy-table entry copies.
 *
 * .noinit is the generic GCC spelling of the same intent. Nothing emits it now, and matching
 * it costs nothing.
 */
#define S_DATA_EXTRA_NOINIT_SECTION_NAME   .ram_noinit* .noinit*

/* RA6M5 has 96 IRQs; the vector table comes from ra_gen/vector_data.c. */
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
 * These take an address. On RA6M5 code flash is based at 0x00000000, so FLASH_AREA_2_OFFSET
 * is also the address.
 */
#define SECONDARY_PARTITION_START       (FLASH_AREA_2_OFFSET)
#define SECONDARY_PARTITION_SIZE        (FLASH_AREA_2_SIZE)

/*
 * Non-secure callable / CMSE veneers.
 *
 * The solution owns this window (FLASH_CPU0_C): 0x800 at 0x11F800, ending exactly on the 32 KB
 * NS boundary at 0x120000. The Renesas Device Partition Manager (RDPM) takes the secure size
 * in KB (1150) and the NSC size in KB (2), not these addresses. Do not derive an IDAU boundary
 * from these macros without reading DESIGN.md §7.1 first.
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
 * ra6m5_layout_checks.c asserts this; its "Main stack" section explains why. */
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
 * RA6M5 option-setting memory: discrete regions, one per register group.
 *
 * The thirteen groups and their start addresses match RA6E1/RA6M4, but BPS/PBPS and their
 * _SEC/_SEL mirrors are 0x10 wide, not 0xC, because the 2 MB code flash has more protectable
 * blocks. Values are from the generated ra6m5_gcc_mcuboot/Debug/memory_regions.ld; do not copy
 * the RA6E1 numbers.
 *
 * WARNING: these must stay thirteen separate MEMORY regions in ra6m5_bl2.ld, each section
 * assigned with '> REGION'. One coalesced PT_LOAD zero-fills the 352 B of gaps, which reach
 * eight defined registers, BPS and PBPS among them (D133), and bricks the part with no known
 * recovery (D002).
 *
 * The fill is invisible in the srec: it is in the program header, and objcopy -O srec emits
 * from sections. Verify with:
 *     arm-none-eabi-readelf -l bin/bl2.axf
 * Expect small separate LOAD segments in 0x0100Axxx, never one spanning 0x1D0.
 * Rationale: DESIGN.md §8.4.
 */
#define OPTION_SETTING_OFS0_START           0x0100A100
#define OPTION_SETTING_OFS0_LENGTH          0x4
#define OPTION_SETTING_DUALSEL_START        0x0100A110
#define OPTION_SETTING_DUALSEL_LENGTH       0x4
#define OPTION_SETTING_OFS1_START           0x0100A180
#define OPTION_SETTING_OFS1_LENGTH          0x4
#define OPTION_SETTING_BANKSEL_START        0x0100A190
#define OPTION_SETTING_BANKSEL_LENGTH       0x4
#define OPTION_SETTING_BPS_START            0x0100A1C0
#define OPTION_SETTING_BPS_LENGTH           0x10
#define OPTION_SETTING_PBPS_START           0x0100A1E0
#define OPTION_SETTING_PBPS_LENGTH          0x10
#define OPTION_SETTING_OFS1_SEC_START       0x0100A200
#define OPTION_SETTING_OFS1_SEC_LENGTH      0x4
#define OPTION_SETTING_BANKSEL_SEC_START    0x0100A210
#define OPTION_SETTING_BANKSEL_SEC_LENGTH   0x4
#define OPTION_SETTING_BPS_SEC_START        0x0100A240
#define OPTION_SETTING_BPS_SEC_LENGTH       0x10
#define OPTION_SETTING_PBPS_SEC_START       0x0100A260
#define OPTION_SETTING_PBPS_SEC_LENGTH      0x10
#define OPTION_SETTING_OFS1_SEL_START       0x0100A280
#define OPTION_SETTING_OFS1_SEL_LENGTH      0x4
#define OPTION_SETTING_BANKSEL_SEL_START    0x0100A290
#define OPTION_SETTING_BANKSEL_SEL_LENGTH   0x4
#define OPTION_SETTING_BPS_SEL_START        0x0100A2C0
#define OPTION_SETTING_BPS_SEL_LENGTH       0x10

#endif /* __REGION_DEFS_H__ */
