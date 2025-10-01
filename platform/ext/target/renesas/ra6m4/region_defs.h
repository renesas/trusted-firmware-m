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

/* SRAM memory */
#define SRAM_BASE                       0x20000000
#define TOTAL_RAM_SIZE                  0x00040000  /* 256KB */

/* Secure and Non-Secure RAM split (128KB each) */
#define S_RAM_SIZE                      0x00020000  /* 128KB */
#define NS_RAM_SIZE                     0x00020000  /* 128KB */

#define S_RAM_ALIAS_BASE                SRAM_BASE
#define NS_RAM_ALIAS_BASE               (SRAM_BASE + S_RAM_SIZE)

/* Secure regions */
#define S_CODE_START                    S_ROM_ALIAS_BASE
#define S_CODE_SIZE                     FLASH_S_PARTITION_SIZE
#define S_CODE_LIMIT                    (S_CODE_START + S_CODE_SIZE - 1)

#define S_DATA_START                    S_RAM_ALIAS_BASE
#define S_DATA_SIZE                     S_RAM_SIZE
#define S_DATA_LIMIT                    (S_DATA_START + S_DATA_SIZE - 1)

/* Non-Secure regions */
#define NS_CODE_START                   NS_ROM_ALIAS_BASE
#define NS_CODE_SIZE                    FLASH_NS_PARTITION_SIZE
#define NS_CODE_LIMIT                   (NS_CODE_START + NS_CODE_SIZE - 1)

#define NS_DATA_START                   NS_RAM_ALIAS_BASE
#define NS_DATA_SIZE                    NS_RAM_SIZE
#define NS_DATA_LIMIT                   (NS_DATA_START + NS_DATA_SIZE - 1)

/* Non-Secure Callable (NSC) region */
#define CMSE_VENEER_REGION_SIZE         0x400       /* 1KB */

/* Bootloader regions */
#define BL2_HEADER_SIZE                 0x400       /* 1KB */
#define BL2_TRAILER_SIZE                0x800       /* 2KB */

/* MPU region alignment requirements for ARMv8-M */
#define MPU_REGION_ALIGNMENT            0x20        /* 32 bytes minimum */

/* Platform peripherals */
#define PERIPHERAL_BASE                 0x40000000
#define PERIPHERAL_SIZE                 0x10000000

/* Peripheral access from secure world */
#define S_PERIPHERAL_ALIAS_BASE         PERIPHERAL_BASE
#define NS_PERIPHERAL_ALIAS_BASE        (PERIPHERAL_BASE + 0x10000000)

/* NVIC Configuration */
#define NVIC_USER_IRQ_OFFSET            16
#define NVIC_USER_IRQ_NUMBER            480

#endif /* __REGION_DEFS_H__ */