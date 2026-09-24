/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __REGION_DEFS_H__
#define __REGION_DEFS_H__

#include "flash_layout.h"     /* pulls in bsp_linker_info.h */

/*
 * RA8M2 memory regions - DERIVED from the e2 solution, see flash_layout.h.
 * Nothing here is hand-maintained; change the partitions in solution.xml, rebuild in
 * e2, and these follow.
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
 * MCUboot header/trailer, straight from the solution's partitioning.
 *
 * These also reach the build as -D from CMake, and THAT copy is what imgtool signs with.
 * They must agree: TF-M defaults both to 0x400, and with a 0x400 header imgtool places the
 * payload 0x400 into the slot while the image is linked for slot+0x200 - a signed image
 * offset by 0x200, which does not boot and produces no other symptom. config.cmake reads
 * the real values out of bsp_linker_info.h, but only on a CLEAN CMake cache, so check
 * rather than silently redefine.
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
 * Secure image. FLASH_CPU0_S is the code, FLASH_CPU0_C the non-secure-callable
 * veneers at the end of it - the solution already places NSC at end-of-secure, which
 * is the layout the ra6m4 port had to force with linker macros.
 */
#define S_CODE_START                    (BSP_PARTITION_FLASH_CPU0_S_START)
#define S_CODE_SIZE                     (BSP_PARTITION_FLASH_CPU0_S_SIZE + \
                                         BSP_PARTITION_FLASH_CPU0_C_SIZE)
#define S_CODE_LIMIT                    (S_CODE_START + S_CODE_SIZE - 1)

/*
 * Secure RAM.
 *
 * NO code-from-RAM window on this part, unlike RA6E1 and RA6M5. Those needed one because
 * r_flash_hp carries PLACE_IN_RAM_SECTION on its code-flash program/erase routines: the FCU
 * makes the ENTIRE code flash unreadable while a P/E is in progress, so running them from
 * flash is a prefetch abort mid-operation with the FCU left in P/E mode.
 *
 * MRAM does not work that way, and FSP says so by construction:
 *
 *   - PLACE_IN_RAM_SECTION appears in NO FSP source in this project. r_mram.h DEFINES the
 *     macro - copied verbatim from r_flash_hp.h, comment and all, including a reference to
 *     a "Code Flash programming is enabled" option that r_mram_cfg.h does not have - but
 *     r_mram.c never applies it. RA6M5's r_flash_hp.c applies it 17 times.
 *   - In the e2-built secure image the linked block is EMPTY: "__ram_from_flash 0x2200'0000
 *     0x0 <Block>" in the map. Nothing is placed there because nothing asks to be.
 *
 * So FSP's own MRAM driver executes from MRAM while programming MRAM, which it could not do
 * if the array went unreadable. That is inference from FSP's behaviour rather than a quoted
 * datasheet guarantee, so it is worth knowing how to undo: define S_RAM_CODE_SIZE and
 * S_RAM_CODE_START again (taking the space off the TOP of the secure RAM partition, as the
 * RA6M5 port does) and TF-M's generated linker re-emits .ER_CODE_SRAM with its copy-table
 * entry. The mechanism is entirely gated on S_RAM_CODE_START being defined.
 *
 * WHAT THIS ALSO RETIRES. DECISIONS D029/D048 - ILINK declining to copy-init
 * .ram_from_flash at isolation 3, leaving the P/E routines in flash while isolation 1
 * relocated them correctly - cannot arise here, because there is nothing to copy.
 *
 * THREE FILES HAVE TO AGREE, and all three now say nothing:
 *   - this file             - no S_RAM_CODE_START, so TF-M's template creates no ER_CODE_SRAM
 *   - ra8m2_fsp_sections.icf - places no .ram_from_flash (naming it would target a block that
 *                              does not exist)
 *   - tfm_hal_platform.c    - no ra_ram_code_init() helper and no call
 * They were briefly left in place as inert no-ops; that is worse than removing them, because
 * __section_begin() on a section nothing creates is a link-time question rather than a
 * no-op, and a reader cannot tell a deliberate stub from an oversight. The restore recipe is
 * at the top of ra8m2_fsp_sections.icf.
 *
 * SYMPTOM IF THIS IS WRONG: a hang, not a fault, on the first Protected Storage or ITS
 * write - those live in DF_EMULATION, which is MRAM (flash_layout.h), so unlike RA6M5 the
 * secure image programs MRAM on a normal code path rather than only in an armed-but-unused
 * one. If that happens, restore the window first.
 */
#define S_DATA_START                    (BSP_PARTITION_RAM_CPU0_S_START)
#define S_DATA_SIZE                     (BSP_PARTITION_RAM_CPU0_S_SIZE)
#define S_DATA_LIMIT                    (S_DATA_START + S_DATA_SIZE - 1)

/* IAR reaches the platform's vendor sections through literal ICF text: a section name
 * cannot survive iccarm's preprocessor as a macro. See ra8m2_fsp_sections.icf, and the
 * note at the hook in platform/ext/common/iar/tfm_isolation_s.icf.template. */
#ifdef __ICCARM__
#define S_ICF_PLATFORM_SECTIONS         "ra8m2_fsp_sections.icf"
#endif

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
 * Secure secondary (staging) slot. TF-M's generated secure linker emits the
 * LR_SECONDARY_PARTITION load-region symbol from these, and target_cfg.c publishes them
 * in memory_regions. ra6m4 had to stub these to 0 - the solution gives us the real slot,
 * so derive it (FLASH_AREA_2 = image 0 secondary = secure).
 *
 * ADDRESS, NOT OFFSET - and on this part those differ.
 *
 * This is consumed as an address in three places: "place at address
 * SECONDARY_PARTITION_START" in the IAR ICF, "Load$$LR$$LR_SECONDARY_PARTITION$$Base ="
 * in the GNU script, and .secondary_partition_base in target_cfg.c. It therefore takes
 * the partition's START directly, NOT FLASH_AREA_2_OFFSET.
 *
 * FLASH_AREA_*_OFFSET is what MCUboot wants: a flat offset into the flash device, with the
 * security alias base subtracted (flash_layout.h, TFM_MRAM_S_OFF / TFM_MRAM_NS_OFF). MRAM
 * is based at 0x02000000, so the offset 0x68000 and the address 0x02068000 are 32 MB apart.
 * On RA6E1 and RA6M5 code flash is based at 0x00000000 and the two coincide, which is why
 * the port this was derived from could use FLASH_AREA_2_OFFSET here and be correct.
 *
 * Sizes are alias-independent, so FLASH_AREA_2_SIZE is used as-is.
 */
#define SECONDARY_PARTITION_START       (BSP_PARTITION___BL_0_S_H_START)
#define SECONDARY_PARTITION_SIZE        (FLASH_AREA_2_SIZE)

/*
 * Non-secure callable / CMSE veneers.
 *
 * The solution owns this window (FLASH_CPU0_C), 0x800 at 0x11F800, ending exactly on the
 * 32 KB NS boundary at 0x120000. The Partition Manager takes the SECURE size in KB (1150)
 * and the NSC size in KB (2) - it does not take these addresses. Do NOT derive an IDAU
 * boundary from these macros without reading DESIGN.md 7.1 first.
 */
#define CMSE_VENEER_REGION_START        (BSP_PARTITION_FLASH_CPU0_C_START)
#define CMSE_VENEER_REGION_SIZE         (BSP_PARTITION_FLASH_CPU0_C_SIZE)
#define CMSE_VENEER_REGION_LIMIT        (CMSE_VENEER_REGION_START + \
                                         CMSE_VENEER_REGION_SIZE - 1)

/* Pin the veneers where the solution says, on TF-M's own generated linker. */
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
 * BL2. It is the bootloader: it lives at the base of flash. Deriving this from the
 * secure image base put the ra6m4 BL2 in the secure slot, so nothing was at the reset
 * vector and the device never ran it (DESIGN.md 8.3).
 *
 * BL2 owns all secure RAM while it runs - the solution's RAM_BL partition
 * (BSP_PARTITION_RAM_BL_CPU0_S_SIZE) is its own bootloader's budget and is smaller
 * than TF-M's BL2 needs, so use the full secure RAM partition instead.
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
 * BL2 writes the measurement TLVs through MCUBOOT_SHARED_DATA_BASE (aliased to these
 * in bl2/ext/mcuboot/include/flash_map/flash_map.h) and the SPM reads them back at
 * SHARED_BOOT_MEASUREMENT_BASE in tfm_boot_data.c. Both must name the SAME region, so
 * alias them onto the shared data area - the upstream convention (rp2350, psoc64,
 * the stm32 family). ra6m4 instead carved a separate 0x100 block ahead of the shared
 * data, which left BOOT_TFM_SHARED_DATA_* feeding only the overlap check.
 */
#define SHARED_BOOT_MEASUREMENT_BASE    (BOOT_TFM_SHARED_DATA_BASE)
#define SHARED_BOOT_MEASUREMENT_SIZE    (BOOT_TFM_SHARED_DATA_SIZE)

#define S_HEAP_SIZE                     (0x800)

/* Must be BSP_CFG_STACK_MAIN_BYTES + STACKSEAL_SIZE (0x1000 + 8 for this solution).
 *
 * The generated scatter file lays .msp_stack out as S_MSP_STACK_SIZE - STACKSEAL_SIZE
 * and reserves the eight bytes above it for __StackSeal. bsp_init_stub.c aliases FSP's
 * g_main_stack onto that stack, and FSP's SystemInit() writes its stack seal at
 * &g_main_stack[BSP_CFG_STACK_MAIN_BYTES] - which therefore has to land exactly on
 * __StackSeal rather than in live data above it.
 *
 * ra8m2_layout_checks.c asserts the relationship, so a regenerated project that changes
 * the FSP main stack size fails the build instead of scribbling past the stack. */
#define S_MSP_STACK_SIZE                (0x1008)
#define S_PSP_STACK_SIZE                (0x800)
#define NS_HEAP_SIZE                    (0x1000)
#define NS_STACK_SIZE                   (0x1000)

/* Peripherals - whole space, attribution is by IDAU/SAU not by these. */
#define PERIPHERAL_BASE                 (0x40000000)
#define PERIPHERAL_SIZE                 (0x10000000)
#define PERIPHERALS_BASE_S              (PERIPHERAL_BASE)
#define PERIPHERALS_LIMIT_S             (PERIPHERAL_BASE + PERIPHERAL_SIZE - 1)
#define PERIPHERALS_BASE_NS             (PERIPHERAL_BASE)
#define PERIPHERALS_LIMIT_NS            (PERIPHERAL_BASE + PERIPHERAL_SIZE - 1)

/*
 * RA8M2 option-setting memory - DISCRETE regions, one per register group.
 *
 * THE ADDRESSES ARE NOT HERE, AND MUST NOT BE PUT BACK. They come from
 * option_settings.h, which the platform CMakeLists generates at configure time from the
 * solution's own memory_regions.icf. Included below.
 *
 * WHY. They were hand-written here until 2026-09-24, and because this port was seeded from
 * RA6M5 the values were RA6M5's: .option_setting_ofs0 linked at 0x0100A100 when RA8M2's
 * OFS0 is at 0x02c9f040. The image wrote option words to an address that is not this
 * device's option memory, and the real OFS and block-protect registers were never written
 * at all. The comment that sat here even asserted "the same start addresses as RA6E1/RA6M4"
 * and "2 MB of code flash", both false. Caught by reading bin/bl2.map, before flashing.
 *
 * RA8M2's set is NOT RA6's: 26 groups against 13. OFS2, OFS3 (+_SEC/_SEL) and SAS are new;
 * there is a separate OTP block (FSBLCTRL0-2, SAMR, SACC00-13, PBPS +_SEC, ZHUK); and
 * DUALSEL, BANKSEL and the non-OTP PBPS do not exist here. BPS is 0x80, not 0x10.
 *
 * WHAT HAS NOT CHANGED - the placement rule, which is the actual brick hazard. These MUST
 * stay SEPARATE regions in ra8m2_bl2.ld / .icf, each section assigned with '> REGION'.
 * Emitting them as one coalesced PT_LOAD zero-fills the FCU configuration in the gaps,
 * including the block-protect permanence word, and permanently bricks the part. That
 * destroyed two EK-RA6M4 boards on 2026-07-21 (DECISIONS D002).
 *
 * The fill is INVISIBLE in the srec - it lives in the program header, and an srec is
 * emitted from sections. Verify the segments, not the sections:
 *     arm-none-eabi-readelf -l bin/bl2.axf
 * Expect small separate LOAD segments in 0x02c9fxxx and 0x02e07xxx/0x02e17xxx, never one
 * spanning them. Full rationale: DESIGN.md 8.4.
 */
#include "option_settings.h"

#endif /* __REGION_DEFS_H__ */
