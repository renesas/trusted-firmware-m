/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* clang-format off */
#ifndef BSP_LINKER_H
#define BSP_LINKER_H

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/******* Solution Definitions *************/
#define BSP_PARTITION_RAM_CPU0_N_START (RAM_NS_CPU0_N_START)
#define BSP_PARTITION_RAM_CPU0_N_SIZE (RAM_NS_CPU0_N_SIZE)
#define BSP_PARTITION_RAM_CPU1_N_START (RAM_NS_CPU1_N_START)
#define BSP_PARTITION_RAM_CPU1_N_SIZE (RAM_NS_CPU1_N_SIZE)
#define BSP_PARTITION_RAM_CPU0_S_START (RAM_CPU0_S_START)
#define BSP_PARTITION_RAM_CPU0_S_SIZE (RAM_CPU0_S_SIZE)
#define BSP_PARTITION_RAM_CPU0_C_START (RAM_CPU0_C_START)
#define BSP_PARTITION_RAM_CPU0_C_SIZE (RAM_CPU0_C_SIZE)
#define BSP_PARTITION_FLASH_CPU0_N_START (FLASH_NS_CPU0_N_START)
#define BSP_PARTITION_FLASH_CPU0_N_SIZE (FLASH_NS_CPU0_N_SIZE)
/* Secure extension disabled for CPU1, so the secure start address is the same as the non-secure start address. */
#define BSP_PARTITION_FLASH_CPU1_S_START (FLASH_NS_CPU1_N_START)
#define BSP_PARTITION_FLASH_CPU1_N_START (FLASH_NS_CPU1_N_START)
#define BSP_PARTITION_FLASH_CPU1_N_SIZE (FLASH_NS_CPU1_N_SIZE)
#define BSP_PARTITION_FLASH_CPU0_S_START (FLASH_CPU0_S_START)
#define BSP_PARTITION_FLASH_CPU0_S_SIZE (FLASH_CPU0_S_SIZE)
#define BSP_PARTITION_FLASH_CPU0_C_START (FLASH_CPU0_C_START)
#define BSP_PARTITION_FLASH_CPU0_C_SIZE (FLASH_CPU0_C_SIZE)
#define BSP_PARTITION_DATA_FLASH_CPU0_N_START (DATA_FLASH_NS_CPU0_N_START)
#define BSP_PARTITION_DATA_FLASH_CPU0_N_SIZE (DATA_FLASH_NS_CPU0_N_SIZE)
#define BSP_PARTITION_DATA_FLASH_CPU0_S_START (DATA_FLASH_CPU0_S_START)
#define BSP_PARTITION_DATA_FLASH_CPU0_S_SIZE (DATA_FLASH_CPU0_S_SIZE)
#define BSP_PARTITION_SDRAM_CPU0_S_START (SDRAM_CPU0_S_START)
#define BSP_PARTITION_SDRAM_CPU0_S_SIZE (SDRAM_CPU0_S_SIZE)
#define BSP_PARTITION_SDRAM_CPU0_N_START (SDRAM_CPU0_N_START)
#define BSP_PARTITION_SDRAM_CPU0_N_SIZE (SDRAM_CPU0_N_SIZE)
#define BSP_PARTITION_OSPI0_CS0_CPU0_S_START (OSPI0_CS0_CPU0_S_START)
#define BSP_PARTITION_OSPI0_CS0_CPU0_S_SIZE (OSPI0_CS0_CPU0_S_SIZE)
#define BSP_PARTITION_OSPI0_CS0_CPU0_N_START (OSPI0_CS0_CPU0_N_START)
#define BSP_PARTITION_OSPI0_CS0_CPU0_N_SIZE (OSPI0_CS0_CPU0_N_SIZE)
#define BSP_PARTITION_OSPI0_CS1_CPU0_S_START (OSPI0_CS1_CPU0_S_START)
#define BSP_PARTITION_OSPI0_CS1_CPU0_S_SIZE (OSPI0_CS1_CPU0_S_SIZE)
#define BSP_PARTITION_OSPI0_CS1_CPU0_N_START (OSPI0_CS1_CPU0_N_START)
#define BSP_PARTITION_OSPI0_CS1_CPU0_N_SIZE (OSPI0_CS1_CPU0_N_SIZE)
#define BSP_PARTITION_OSPI1_CS0_CPU0_S_START (OSPI1_CS0_CPU0_S_START)
#define BSP_PARTITION_OSPI1_CS0_CPU0_S_SIZE (OSPI1_CS0_CPU0_S_SIZE)
#define BSP_PARTITION_OSPI1_CS0_CPU0_N_START (OSPI1_CS0_CPU0_N_START)
#define BSP_PARTITION_OSPI1_CS0_CPU0_N_SIZE (OSPI1_CS0_CPU0_N_SIZE)
#define BSP_PARTITION_OSPI1_CS1_CPU0_S_START (OSPI1_CS1_CPU0_S_START)
#define BSP_PARTITION_OSPI1_CS1_CPU0_S_SIZE (OSPI1_CS1_CPU0_S_SIZE)
#define BSP_PARTITION_OSPI1_CS1_CPU0_N_START (OSPI1_CS1_CPU0_N_START)
#define BSP_PARTITION_OSPI1_CS1_CPU0_N_SIZE (OSPI1_CS1_CPU0_N_SIZE)
#define BSP_PARTITION_ITCM_CPU0_N_START (ITCM_NS_CPU0_N_START)
#define BSP_PARTITION_ITCM_CPU0_N_SIZE (ITCM_NS_CPU0_N_SIZE)
#define BSP_PARTITION_ITCM_CPU0_S_START (ITCM_CPU0_S_START)
#define BSP_PARTITION_ITCM_CPU0_S_SIZE (ITCM_CPU0_S_SIZE)
#define BSP_PARTITION_DTCM_CPU0_N_START (DTCM_NS_CPU0_N_START)
#define BSP_PARTITION_DTCM_CPU0_N_SIZE (DTCM_NS_CPU0_N_SIZE)
#define BSP_PARTITION_DTCM_CPU0_S_START (DTCM_CPU0_S_START)
#define BSP_PARTITION_DTCM_CPU0_S_SIZE (DTCM_CPU0_S_SIZE)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/* These symbols are used for sau/idau configuration in a secure project */
extern uint32_t const *const gp_ddsc_DATA_FLASH_START;
extern uint32_t const *const gp_ddsc_DATA_FLASH_END;
extern uint32_t const *const gp_ddsc_ITCM_START;
extern uint32_t const *const gp_ddsc_ITCM_END;
extern uint32_t const *const gp_ddsc_DTCM_START;
extern uint32_t const *const gp_ddsc_DTCM_END;
extern uint32_t const *const gp_ddsc_FLASH_START;
extern uint32_t const *const gp_ddsc_FLASH_END;
extern uint32_t const *const gp_ddsc_FLASH_NSC;
extern uint32_t const *const gp_ddsc_RAM_START;
extern uint32_t const *const gp_ddsc_RAM_END;
extern uint32_t const *const gp_ddsc_RAM_NSC;

/***********************************************************************************************************************
 * Exported global functions (to be accessed by other files)
 **********************************************************************************************************************/
#endif   /* BSP_LINKER_H */
#ifdef BSP_LINKER_C
/***********************************************************************************************************************
 * Objects allocated by bsp_linker.c
 **********************************************************************************************************************/
/* DDSC symbol definitions */
extern const uint32_t __ddsc_ITCM_START;
BSP_DONT_REMOVE uint32_t const * const gp_ddsc_ITCM_START = &__ddsc_ITCM_START;
extern const uint32_t __ddsc_ITCM_END;
BSP_DONT_REMOVE uint32_t const * const gp_ddsc_ITCM_END = &__ddsc_ITCM_END;
extern const uint32_t __ddsc_DTCM_START;
BSP_DONT_REMOVE uint32_t const * const gp_ddsc_DTCM_START = &__ddsc_DTCM_START;
extern const uint32_t __ddsc_DTCM_END;
BSP_DONT_REMOVE uint32_t const * const gp_ddsc_DTCM_END = &__ddsc_DTCM_END;

#endif   /* BSP_LINKER_C */
/* clang-format on */
