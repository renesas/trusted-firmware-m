/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __REGION_DEFS_H__
#define __REGION_DEFS_H__

#include "flash_layout.h"     /* pulls in bsp_linker_info.h */

/*
 * RA6E1 memory regions - DERIVED from the e2 solution, see flash_layout.h.
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

#define S_DATA_START                    (BSP_PARTITION_RAM_CPU0_S_START)
#define S_DATA_SIZE                     (BSP_PARTITION_RAM_CPU0_S_SIZE)
#define S_DATA_LIMIT                    (S_DATA_START + S_DATA_SIZE - 1)

/* RA6E1 has 96 IRQs; the vector table comes from ra_gen/vector_data.c. */
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
 */
#define SECONDARY_PARTITION_START       (FLASH_AREA_2_OFFSET)
#define SECONDARY_PARTITION_SIZE        (FLASH_AREA_2_SIZE)

/*
 * Non-secure callable / CMSE veneers.
 *
 * The solution owns this window (FLASH_CPU0_C), and the hardware NSC region runs from
 * here to the next 32 KB boundary - so the Partition Manager value is 1 KB even though
 * the partition declares 0x300. Do NOT derive an IDAU boundary from these macros
 * without reading DESIGN.md 7.1 first.
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
 * ra6e1_layout_checks.c asserts the relationship, so a regenerated project that changes
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
 * RA6E1 option-setting memory - DISCRETE regions, one per register group.
 *
 * Byte-identical to RA6M4. These MUST stay thirteen separate MEMORY regions in
 * ra6e1_bl2.ld, each section assigned with '> REGION'. Emitting them as one
 * coalesced PT_LOAD zero-fills the 368 bytes of FCU config in the gaps - including
 * the FSPR permanence word - and permanently bricks the part. That destroyed two
 * EK-RA6M4 boards on 2026-07-21.
 *
 * The fill is invisible in the srec (it lives in the program header, and objcopy -O
 * srec emits from sections). Verify with:
 *     arm-none-eabi-readelf -l bin/bl2.axf
 * Expect small separate LOAD segments in 0x0100Axxx, never one spanning 0x1CC.
 * Full rationale: DESIGN.md 8.4.
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
#define OPTION_SETTING_BPS_LENGTH           0xC
#define OPTION_SETTING_PBPS_START           0x0100A1E0
#define OPTION_SETTING_PBPS_LENGTH          0xC
#define OPTION_SETTING_OFS1_SEC_START       0x0100A200
#define OPTION_SETTING_OFS1_SEC_LENGTH      0x4
#define OPTION_SETTING_BANKSEL_SEC_START    0x0100A210
#define OPTION_SETTING_BANKSEL_SEC_LENGTH   0x4
#define OPTION_SETTING_BPS_SEC_START        0x0100A240
#define OPTION_SETTING_BPS_SEC_LENGTH       0xC
#define OPTION_SETTING_PBPS_SEC_START       0x0100A260
#define OPTION_SETTING_PBPS_SEC_LENGTH      0xC
#define OPTION_SETTING_OFS1_SEL_START       0x0100A280
#define OPTION_SETTING_OFS1_SEL_LENGTH      0x4
#define OPTION_SETTING_BANKSEL_SEL_START    0x0100A290
#define OPTION_SETTING_BANKSEL_SEL_LENGTH   0x4
#define OPTION_SETTING_BPS_SEL_START        0x0100A2C0
#define OPTION_SETTING_BPS_SEL_LENGTH       0xC

#endif /* __REGION_DEFS_H__ */
