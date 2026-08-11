/*
 * Copyright (c) 2025 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __REGION_DEFS_H__
#define __REGION_DEFS_H__

#include "flash_layout.h"

/* Memory layout for Renesas RA6M4
 *
 * Flash: 1MB at 0x0000_0000
 * SRAM:  256KB at 0x2000_0000
 *
 * TrustZone Memory Partitioning:
 * - Secure Flash:     128KB BL2 + 384KB S image
 * - Non-Secure Flash: 512KB NS image
 * - Secure SRAM:      128KB
 * - Non-Secure SRAM:  128KB
 */

/* Flash memory */
#define FLASH_BASE_ADDRESS              0x00000000

#define S_ROM_ALIAS_BASE                (FLASH_BASE_ADDRESS + FLASH_AREA_0_OFFSET)
#define NS_ROM_ALIAS_BASE               (FLASH_BASE_ADDRESS + FLASH_AREA_1_OFFSET)

/* BL2 (MCUboot) header and trailer sizes - defined early as they're used in code region calculations */
#ifndef BL2_HEADER_SIZE
#define BL2_HEADER_SIZE                 0x400       /* 1KB */
#endif
#ifndef BL2_TRAILER_SIZE
#define BL2_TRAILER_SIZE                0x800       /* 2KB */
#endif

/* SRAM memory */
#define SRAM_BASE                       0x20000000
#define TOTAL_RAM_SIZE                  0x00040000  /* 256KB */


/* Vector table sizes */
#define S_CODE_VECTOR_TABLE_SIZE        0x800       /* 2048 bytes for secure vector table (496 entries) */

/* Secure and Non-Secure RAM split (128KB each) */
#define S_RAM_SIZE                      0x00020000  /* 128KB */
#define NS_RAM_SIZE                     0x00020000  /* 128KB */
/* Stack sizes */
#define S_MSP_STACK_SIZE                0x0800      /* 2KB Main Stack for Secure */
#define S_PSP_STACK_SIZE                0x0800      /* 2KB Process Stack for Secure */
#define NS_MSP_STACK_SIZE               0x0400      /* 1KB Main Stack for Non-Secure */
#define NS_PSP_STACK_SIZE               0x0C00      /* 3KB Process Stack for Non-Secure */
#define S_HEAP_SIZE                     0x0200      /* 512 bytes Heap for Secure */
#define NS_HEAP_SIZE                    0x1000      /* 4KB Heap for Non-Secure */
#define NS_STACK_SIZE                   0x0400      /* 1KB for NS (used by NSPE) */



#define S_RAM_ALIAS_BASE                SRAM_BASE
#define NS_RAM_ALIAS_BASE               (SRAM_BASE + S_RAM_SIZE)

/* Secure regions */
/* When BL2 is enabled, MCUboot header (BL2_HEADER_SIZE) precedes the image.
 * S_CODE_START must account for this header offset so the linker places
 * code at the correct execution address after MCUboot loads the image. */
#ifdef BL2
#define S_CODE_START                    (S_ROM_ALIAS_BASE + BL2_HEADER_SIZE)
#define S_CODE_SIZE                     (FLASH_S_PARTITION_SIZE - BL2_HEADER_SIZE - BL2_TRAILER_SIZE)
#else
#define S_CODE_START                    S_ROM_ALIAS_BASE
#define S_CODE_SIZE                     FLASH_S_PARTITION_SIZE
#endif
#define S_CODE_LIMIT                    (S_CODE_START + S_CODE_SIZE - 1)

#define S_DATA_START                    S_RAM_ALIAS_BASE
#define S_DATA_SIZE                     S_RAM_SIZE
#define S_DATA_LIMIT                    (S_DATA_START + S_DATA_SIZE - 1)

/* Non-Secure regions */
/* Same as secure: when BL2 is enabled, account for MCUboot header offset */
#ifdef BL2
#define NS_CODE_START                   (NS_ROM_ALIAS_BASE + BL2_HEADER_SIZE)
#define NS_CODE_SIZE                    (FLASH_NS_PARTITION_SIZE - BL2_HEADER_SIZE - BL2_TRAILER_SIZE)
#else
#define NS_CODE_START                   NS_ROM_ALIAS_BASE
#define NS_CODE_SIZE                    FLASH_NS_PARTITION_SIZE
#endif
#define NS_PARTITION_START              NS_ROM_ALIAS_BASE  /* Start of partition (including header) */
#define NS_CODE_LIMIT                   (NS_CODE_START + NS_CODE_SIZE - 1)

#define NS_DATA_START                   NS_RAM_ALIAS_BASE
#define NS_DATA_SIZE                    NS_RAM_SIZE
#define NS_DATA_LIMIT                   (NS_DATA_START + NS_DATA_SIZE - 1)

/* Place the CMSE veneers (.gnu.sgstubs) at the END of the secure code region so
 * the NSC region sits at the Secure->Non-Secure boundary. RA6M4 attributes code
 * flash as contiguous [Secure][NSC][Non-Secure] (programmed via RFP), which
 * requires the veneers at that boundary - not TF-M's default beginning placement.
 * This uses TF-M's own generated linker (via the macro below), the same upstream
 * pattern as the nordic_nrf / lairdconnectivity platforms - no custom linker copy
 * to maintain across TF-M versions. */
#define TFM_LINKER_VENEERS_LOCATION_END

/* Non-Secure Callable (NSC) region - at end of secure code */
#define CMSE_VENEER_REGION_SIZE         0x400       /* 1KB */
#define CMSE_VENEER_REGION_START        (S_CODE_START + S_CODE_SIZE - CMSE_VENEER_REGION_SIZE)

/* Pin the veneer section at a FIXED, slot-boundary address (not end-of-code) so
 * the NSC region is STABLE across firmware updates - the RA6M4 hardware TZ/NSC
 * boundary is programmed once (provisioning) and must not shift when the secure
 * image size changes. TFM_LINKER_VENEERS_START is #ifndef-overridable in TF-M's
 * generated linker (tfm_isolation_s.ld.template), so this stays on TF-M's own
 * linker - no platform linker copy to maintain. */
#define TFM_LINKER_VENEERS_START        CMSE_VENEER_REGION_START

/* Secondary partition (for MCUBoot firmware update) */
/* Note: RA6M4 has only 1MB flash - not enough for true A/B partitioning */
/* These are placeholders for MCUBoot compatibility */
#define SECONDARY_PARTITION_START       0x0         /* Placeholder - no secondary partition */
#define SECONDARY_PARTITION_SIZE        0x0         /* Placeholder - no secondary partition */
/* Shared boot measurement data (for MCUBoot to TF-M) */
/* Allocate small RAM area for boot measurements */
#define SHARED_BOOT_MEASUREMENT_BASE    (S_RAM_ALIAS_BASE)
#define SHARED_BOOT_MEASUREMENT_SIZE    0x100       /* 256 bytes */
#define BOOT_TFM_SHARED_DATA_BASE       (SHARED_BOOT_MEASUREMENT_BASE + SHARED_BOOT_MEASUREMENT_SIZE)
#define BOOT_TFM_SHARED_DATA_SIZE       0x400       /* 1KB for boot shared data */

/* Boot seed for attestation (generated by BL2/MCUboot) */
#define BOOT_TFM_SHARED_SEED_BASE       (BOOT_TFM_SHARED_DATA_BASE + BOOT_TFM_SHARED_DATA_SIZE)
#define BOOT_TFM_SHARED_SEED_SIZE       0x20        /* 32 bytes for boot seed */

/* BL2 (MCUBoot) memory regions */
#define BL2_HEAP_SIZE                   0x1000      /* 4KB heap for BL2 */
#define BL2_MSP_STACK_SIZE              0x800       /* 2KB main stack for BL2 */
#define BL2_PSP_STACK_SIZE              0x800       /* 2KB process stack for BL2 */

/* BL2 is the bootloader: it lives at the base of flash (FLASH_AREA_BL2_OFFSET
 * == 0), NOT at the secure image base. Using S_ROM_ALIAS_BASE here (0x20000)
 * linked BL2 into the SECURE slot, so the reset vector at 0x0 was empty and
 * tfm_s_signed.bin was later written on top of the bootloader. */
#define BL2_CODE_START                  (FLASH_BASE_ADDRESS + FLASH_AREA_BL2_OFFSET)
#define BL2_CODE_SIZE                   FLASH_AREA_BL2_SIZE
#define BL2_CODE_LIMIT                  (BL2_CODE_START + BL2_CODE_SIZE - 1)

#define BL2_DATA_START                  S_RAM_ALIAS_BASE
#define BL2_DATA_SIZE                   TOTAL_RAM_SIZE
#define BL2_DATA_LIMIT                  (BL2_DATA_START + BL2_DATA_SIZE - 1)

/* RA6M4 option-setting memory (OFS) - DISCRETE regions, one per register group.
 *
 * ⚠ These MUST stay thirteen separate regions, and each .option_setting_* output
 * section in ra6m4_bl2.ld MUST be assigned to its own region with '> REGION'.
 * They are NOT a contiguous block and must never be described as one.
 *
 * The option-setting map is sparse: the thirteen groups below hold 92 bytes of
 * real data spread across a 460-byte span (0x0100A100..0x0100A2CC). The 368
 * bytes in between are OTHER FCU configuration - including the Flash Access
 * Window / FSPR permanence word - and are NOT ours to write.
 *
 * If these sections are emitted with plain addressed section statements and no
 * '> REGION' assignment, GNU ld coalesces them into ONE PT_LOAD segment spanning
 * the whole 460 bytes and zero-fills the gaps. A debugger (J-Link/Ozone) loading
 * the ELF writes that entire span, clearing FSPR -> the part is PERMANENTLY
 * bricked. This destroyed two EK-RA6M4 boards on 2026-07-21. Note the fill lives
 * in the program header, not in any section, so `objcopy -O srec` does NOT show
 * it - an srec diff will not catch a regression here. Verify with:
 *     arm-none-eabi-readelf -l bin/bl2.axf   # expect 13 separate LOAD segments,
 *                                            # FileSiz 4 or 12, no 0x1CC span
 *
 * Values are the RA6M4 option-setting map, identical to the RASC-generated
 * memory_regions.ld (FSP_Project_ra6m4_bl2/memory_regions.ld:14-39). A port to
 * another RA device (e.g. RA6E1) must re-derive this list from that device's
 * generated memory_regions.ld - the groups present and their addresses vary.
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

/* MPU region alignment requirements for ARMv8-M */
#define MPU_REGION_ALIGNMENT            0x20        /* 32 bytes minimum */

/* Platform peripherals */
#define PERIPHERAL_BASE                 0x40000000
#define PERIPHERAL_SIZE                 0x10000000

/* Peripheral access from secure world */
#define S_PERIPHERAL_ALIAS_BASE         PERIPHERAL_BASE
#define NS_PERIPHERAL_ALIAS_BASE        (PERIPHERAL_BASE + 0x10000000)

/* Peripheral limits for SAU configuration */
#define PERIPHERALS_BASE_S              S_PERIPHERAL_ALIAS_BASE
#define PERIPHERALS_LIMIT_S             (S_PERIPHERAL_ALIAS_BASE + PERIPHERAL_SIZE - 1)
#define PERIPHERALS_BASE_NS             NS_PERIPHERAL_ALIAS_BASE
#define PERIPHERALS_LIMIT_NS            (NS_PERIPHERAL_ALIAS_BASE + PERIPHERAL_SIZE - 1)

/* NVIC Configuration */
#define NVIC_USER_IRQ_OFFSET            16
#define NVIC_USER_IRQ_NUMBER            480

#endif /* __REGION_DEFS_H__ */