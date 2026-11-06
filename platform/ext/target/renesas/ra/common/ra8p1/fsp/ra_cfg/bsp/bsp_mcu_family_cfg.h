/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BSP_MCU_FAMILY_CFG_H_
#define BSP_MCU_FAMILY_CFG_H_

#include <bsp_mcu_info.h>
#include <bsp_override.h>
#include <r_adc_device_types.h>

#include "bsp_clock_cfg.h"
#include "bsp_mcu_device_cfg.h"
#include "bsp_mcu_device_pn_cfg.h"
#define BSP_MCU_GROUP_RA8P1    (1)
#define BSP_MCU_GROUP_RA8_GEN2 (1)

#define CPU0 (0)
#define CPU1 (1)

#if (_RA_CORE == CPU1)
#define BSP_CFG_CPU_CORE         (1)
#define BSP_CFG_SKIP_INIT        (1)
#define BSP_SECONDARY_CORE_BUILD (1)
#else
#define BSP_CFG_CPU_CORE (0)
#endif

#define BSP_LOCO_HZ      (32768)
#define BSP_MOCO_HZ      (8000000)
#define BSP_SUB_CLOCK_HZ (0)
#if BSP_CFG_HOCO_FREQUENCY == 0
#define BSP_HOCO_HZ (16000000)
#elif BSP_CFG_HOCO_FREQUENCY == 1
#define BSP_HOCO_HZ (18000000)
#elif BSP_CFG_HOCO_FREQUENCY == 2
#define BSP_HOCO_HZ (20000000)
#elif BSP_CFG_HOCO_FREQUENCY == 4
#define BSP_HOCO_HZ (32000000)
#elif BSP_CFG_HOCO_FREQUENCY == 7
#define BSP_HOCO_HZ (48000000)
#else
#error "Invalid HOCO frequency chosen (BSP_CFG_HOCO_FREQUENCY) in bsp_clock_cfg.h"
#endif

#define BSP_CFG_FLL_ENABLE (0)

#define BSP_CFG_CLOCK_SETTLING_DELAY_ENABLE  (1)
#define BSP_CFG_SLEEP_MODE_DELAY_ENABLE      (1)
#define BSP_CFG_MSTP_CHANGE_DELAY_ENABLE     (1)
#define BSP_CFG_RTOS_SLEEP_MODE_DELAY_ENABLE (1)
#define BSP_CFG_CLOCK_SETTLING_DELAY_US      (30)

#if defined(BSP_PACKAGE_HLQFP)
#define BSP_MAX_CLOCK_CHANGE_THRESHOLD (300000000U)
#else
#define BSP_MAX_CLOCK_CHANGE_THRESHOLD (500000000U)
#endif

#define BSP_CORTEX_VECTOR_TABLE_ENTRIES (16U)
#define BSP_VECTOR_TABLE_MAX_ENTRIES    (112U)
#define BSP_CFG_INLINE_IRQ_FUNCTIONS    (1)

#if defined(_RA_TZ_SECURE)
#define BSP_TZ_SECURE_BUILD    (1)
#define BSP_TZ_NONSECURE_BUILD (0)
#define BSP_REMOTE_NONSECURE_BUILD (0)
#elif defined(_RA_TZ_NONSECURE)
#define BSP_TZ_SECURE_BUILD    (0)
#define BSP_TZ_NONSECURE_BUILD (1)
#define BSP_REMOTE_NONSECURE_BUILD (0)
#elif defined(_RA_REMOTE_NONSECURE)
#define BSP_TZ_SECURE_BUILD (0)
#define BSP_TZ_NONSECURE_BUILD (0)
#define BSP_REMOTE_NONSECURE_BUILD (1)
#else
#define BSP_TZ_SECURE_BUILD    (0)
#define BSP_TZ_NONSECURE_BUILD (0)
#define BSP_REMOTE_NONSECURE_BUILD (0)
#endif

/* TrustZone Settings */
#define BSP_TZ_CFG_INIT_SECURE_ONLY   (BSP_CFG_CLOCKS_SECURE || (!BSP_CFG_CLOCKS_OVERRIDE))
#define BSP_TZ_CFG_SKIP_INIT          (BSP_TZ_NONSECURE_BUILD && BSP_TZ_CFG_INIT_SECURE_ONLY)
#define BSP_TZ_CFG_EXCEPTION_RESPONSE (0)

/* TODO: check these configurations */
/* CMSIS TrustZone Settings */
#define SCB_CSR_AIRCR_INIT         (1)
#define SCB_AIRCR_BFHFNMINS_VAL    (0)
#define SCB_AIRCR_SYSRESETREQS_VAL (1)
#define SCB_AIRCR_PRIS_VAL         (0)
#define TZ_FPU_NS_USAGE            (1)
#ifndef SCB_NSACR_CP10_11_VAL
#define SCB_NSACR_CP10_11_VAL (3U)
#endif

/* TODO: check these configurations */
#ifndef FPU_FPCCR_TS_VAL
#define FPU_FPCCR_TS_VAL (1U)
#endif
#define FPU_FPCCR_CLRONRETS_VAL (1)

/* TODO: check these configurations */
#ifndef FPU_FPCCR_CLRONRET_VAL
#define FPU_FPCCR_CLRONRET_VAL (1)
#endif

/* Type 1 Peripheral Security Attribution */

/* Peripheral Security Attribution Register (PSAR) Settings */
#ifndef BSP_TZ_CFG_PSARB
#define BSP_TZ_CFG_PSARB                                                                           \
	((((RENESAS_RA_PSAR_NS_I3C > 0) ? 1U : 0U) << 4) /* I3C */ |                               \
	 (((RENESAS_RA_PSAR_NS_IIC2 > 0) ? 1U : 0U) << 7) /* IIC2 */ |                             \
	 (((RENESAS_RA_PSAR_NS_IIC1 > 0) ? 1U : 0U) << 8) /* IIC1 */ |                             \
	 (((RENESAS_RA_PSAR_NS_IIC0 > 0) ? 1U : 0U) << 9) /* IIC0 */ |                             \
	 (((RENESAS_RA_PSAR_NS_USBFS > 0) ? 1U : 0U) << 11) /* USBFS */ |                          \
	 (((RENESAS_RA_PSAR_NS_USBHS > 0) ? 1U : 0U) << 12) /* USBHS */ |                          \
	 (((RENESAS_RA_PSAR_NS_OSPI0 > 0) ? 1U : 0U) << 16) /* OSPI0 */ |                          \
	 (((RENESAS_RA_PSAR_NS_OSPI1 > 0) ? 1U : 0U) << 17) /* OSPI1 */ |                          \
	 (((RENESAS_RA_PSAR_NS_RSPI1 > 0) ? 1U : 0U) << 18) /* SPI1 */ |                           \
	 (((RENESAS_RA_PSAR_NS_RSPI0 > 0) ? 1U : 0U) << 19) /* SPI0 */ |                           \
	 (((RENESAS_RA_PSAR_NS_SCI9 > 0) ? 1U : 0U) << 22) /* SCI9 */ |                            \
	 (((RENESAS_RA_PSAR_NS_SCI8 > 0) ? 1U : 0U) << 23) /* SCI8 */ |                            \
	 (((RENESAS_RA_PSAR_NS_SCI7 > 0) ? 1U : 0U) << 24) /* SCI7 */ |                            \
	 (((RENESAS_RA_PSAR_NS_SCI6 > 0) ? 1U : 0U) << 25) /* SCI6 */ |                            \
	 (((RENESAS_RA_PSAR_NS_SCI5 > 0) ? 1U : 0U) << 26) /* SCI5 */ |                            \
	 (((RENESAS_RA_PSAR_NS_SCI4 > 0) ? 1U : 0U) << 27) /* SCI4 */ |                            \
	 (((RENESAS_RA_PSAR_NS_SCI3 > 0) ? 1U : 0U) << 28) /* SCI3 */ |                            \
	 (((RENESAS_RA_PSAR_NS_SCI2 > 0) ? 1U : 0U) << 29) /* SCI2 */ |                            \
	 (((RENESAS_RA_PSAR_NS_SCI1 > 0) ? 1U : 0U) << 30) /* SCI1 */ |                            \
	 (((RENESAS_RA_PSAR_NS_SCI0 > 0) ? 1U : 0U) << 31) /* SCI0 */)
#endif
#ifndef BSP_TZ_CFG_PSARC
#define BSP_TZ_CFG_PSARC                                                                           \
	((((RENESAS_RA_PSAR_NS_CAC > 0) ? 1U : 0U) << 0) /* CAC */ |                               \
	 (((RENESAS_RA_PSAR_NS_CRC > 0) ? 1U : 0U) << 1) /* CRC */ |                               \
	 (((RENESAS_RA_PSAR_NS_SSIE1 > 0) ? 1U : 0U) << 7) /* SSIE1 */ |                           \
	 (((RENESAS_RA_PSAR_NS_SSIE0 > 0) ? 1U : 0U) << 8) /* SSIE0 */ |                           \
	 (((RENESAS_RA_PSAR_NS_SDHI1 > 0) ? 1U : 0U) << 11) /* SDHI1 */ |                          \
	 (((RENESAS_RA_PSAR_NS_SDHI0 > 0) ? 1U : 0U) << 12) /* SDHI0 */ |                          \
	 (((RENESAS_RA_PSAR_NS_DOC > 0) ? 1U : 0U) << 13) /* DOC */ |                              \
	 (((RENESAS_RA_PSAR_NS_GRAPHICS > 0) ? 1U : 0U) << 15) /* GLCDC/MIPI-DSI/MIPI-CSI/DRW */ | \
	 (((RENESAS_RA_PSAR_NS_CEU > 0) ? 1U : 0U) << 16) /* CEU */ |                              \
	 (((RENESAS_RA_PSAR_NS_PDMIF > 0) ? 1U : 0U) << 24) /* PDM */ |                            \
	 (((RENESAS_RA_PSAR_NS_CANFD1 > 0) ? 1U : 0U) << 26) /* CANFD1 */ |                        \
	 (((RENESAS_RA_PSAR_NS_CANFD0 > 0) ? 1U : 0U) << 27) /* CANFD0 */ |                        \
	 (((RENESAS_RA_PSAR_NS_ETHPHYCLK > 0) ? 1U : 0U) << 28) /* ETHPHYCLK */ |                  \
	 (((RENESAS_RA_PSAR_NS_ESWM > 0) ? 1U : 0U) << 30) /* ESWM */ |                            \
	 (((RENESAS_RA_PSAR_NS_RSIPE50D > 0) ? 1U : 0U) << 31) /* RSIP-E50D */)
#endif
#ifndef BSP_TZ_CFG_PSARD
#define BSP_TZ_CFG_PSARD                                                                           \
	((((RENESAS_RA_PSAR_NS_AGT1 > 0) ? 1U : 0U) << 4) /* AGT1 */ |                             \
	 (((RENESAS_RA_PSAR_NS_AGT0 > 0) ? 1U : 0U) << 5) /* AGT0 */ |                             \
	 (((RENESAS_RA_PSAR_NS_PDG > 0) ? 1U : 0U) << 6) /* PDG */ |                               \
	 (((RENESAS_RA_PSAR_NS_PGI3 > 0) ? 1U : 0U) << 11) /* PGI3 */ |                            \
	 (((RENESAS_RA_PSAR_NS_PGI2 > 0) ? 1U : 0U) << 12) /* PGI2 */ |                            \
	 (((RENESAS_RA_PSAR_NS_PGI1 > 0) ? 1U : 0U) << 13) /* PGI1 */ |                            \
	 (((RENESAS_RA_PSAR_NS_PGI0 > 0) ? 1U : 0U) << 14) /* PGI0 */ |                            \
	 (((RENESAS_RA_PSAR_NS_DAC121 > 0) ? 1U : 0U) << 19) /* DAC121 */ |                        \
	 (((RENESAS_RA_PSAR_NS_DAC120 > 0) ? 1U : 0U) << 20) /* DAC120 */ |                        \
	 (((RENESAS_RA_PSAR_NS_ADC16H > 0) ? 1U : 0U) << 21) /* ADC_B */ |                         \
	 (((RENESAS_RA_PSAR_NS_TSN > 0) ? 1U : 0U) << 22) /* TSN */ |                              \
	 (((RENESAS_RA_PSAR_NS_ACMPHS3 > 0) ? 1U : 0U) << 25) /* ACMPHS3 */ |                      \
	 (((RENESAS_RA_PSAR_NS_ACMPHS2 > 0) ? 1U : 0U) << 26) /* ACMPHS2 */ |                      \
	 (((RENESAS_RA_PSAR_NS_ACMPHS1 > 0) ? 1U : 0U) << 27) /* ACMPHS1 */ |                      \
	 (((RENESAS_RA_PSAR_NS_ACMPHS0 > 0) ? 1U : 0U) << 28) /* ACMPHS0 */)
#endif
#ifndef BSP_TZ_CFG_PSARE
#define BSP_TZ_CFG_PSARE                                                                           \
	((((RENESAS_RA_PSAR_NS_WDT1 > 0) ? 1U : 0U) << 0) /* WDT1 */ |                             \
	 (((RENESAS_RA_PSAR_NS_WDT0 > 0) ? 1U : 0U) << 1) /* WDT0 */ |                             \
	 (((RENESAS_RA_PSAR_NS_IWDT > 0) ? 1U : 0U) << 2) /* IWDT */ |                             \
	 (((RENESAS_RA_PSAR_NS_RTC > 0) ? 1U : 0U) << 3) /* RTC */ |                               \
	 (((RENESAS_RA_PSAR_NS_ULPT1 > 0) ? 1U : 0U) << 8) /* ULPT1 */ |                           \
	 (((RENESAS_RA_PSAR_NS_ULPT0 > 0) ? 1U : 0U) << 9) /* ULPT0 */ |                           \
	 (((RENESAS_RA_PSAR_NS_GPT_COMMON > 0) ? 1U : 0U) << 15) /* GPT COMMON */ |                \
	 (((RENESAS_RA_PSAR_NS_GPT13 > 0) ? 1U : 0U) << 18) /* GPT13 */ |                          \
	 (((RENESAS_RA_PSAR_NS_GPT12 > 0) ? 1U : 0U) << 19) /* GPT12 */ |                          \
	 (((RENESAS_RA_PSAR_NS_GPT11 > 0) ? 1U : 0U) << 20) /* GPT11 */ |                          \
	 (((RENESAS_RA_PSAR_NS_GPT10 > 0) ? 1U : 0U) << 21) /* GPT10 */ |                          \
	 (((RENESAS_RA_PSAR_NS_GPT4 > 0) ? 1U : 0U) << 27) /* GPT4 */ |                            \
	 (((RENESAS_RA_PSAR_NS_GPT3 > 0) ? 1U : 0U) << 28) /* GPT3 */ |                            \
	 (((RENESAS_RA_PSAR_NS_GPT2 > 0) ? 1U : 0U) << 29) /* GPT2 */ |                            \
	 (((RENESAS_RA_PSAR_NS_GPT1 > 0) ? 1U : 0U) << 30) /* GPT1 */ |                            \
	 (((RENESAS_RA_PSAR_NS_GPT0 > 0) ? 1U : 0U) << 31) /* GPT0 */)
#endif
#ifndef BSP_TZ_CFG_MSSAR
#define BSP_TZ_CFG_MSSAR                                                                           \
	((((RENESAS_RA_MSSAR_NS_SRAM0 > 0) ? 1U : 0) << 0) /* SRAM0 */ |                           \
	 (((RENESAS_RA_MSSAR_NS_SRAM1 > 0) ? 1U : 0U) << 1) /* SRAM1 */ |                          \
	 (((RENESAS_RA_MSSAR_NS_SRAM2 > 0) ? 1U : 0U) << 2) /* SRAM2 */ |                          \
	 (((RENESAS_RA_MSSAR_NS_SRAM3 > 0) ? 1U : 0U) << 3) /* SRAM3 */ |                          \
	 (((RENESAS_RA_MSSAR_NS_NPU > 0) ? 1U : 0U) << 16) /* NPU */ |                             \
	 (((RENESAS_RA_MSSAR_NS_DMAC0_DTC0 > 0) ? 1U : 0U) << 22) /* DTC0_DMAC0 */ |               \
	 (((RENESAS_RA_MSSAR_NS_DMAC1_DTC1 > 0) ? 1U : 0U) << 23) /* DTC1_DMAC1 */ |               \
	 (((RENESAS_RA_MSSAR_NS_ELC > 0) ? 1U : 0U) << 31) /* ELC */)
#endif

/* Type 2 Peripheral Security Attribution */

/* Security attribution for RSTSRn registers. */
#ifndef BSP_TZ_CFG_RSTSAR
#define BSP_TZ_CFG_RSTSAR (0x0000000FU)
#endif

/* Security attribution for registers of LVD channels. */
#ifndef BSP_TZ_CFG_LVDSAR
/* The LVD driver needs to access both channels. This means that the security
 * attribution for both channels must be the same.
 */
#if (RA_NOT_DEFINED > 0) || (RA_NOT_DEFINED > 0)
#define BSP_TZ_CFG_LVDSAR (0U)
#else
#define BSP_TZ_CFG_LVDSAR (3U)
#endif
#endif

/* Security attribution for LPM registers.
 * - OPCCR based on clock security.
 * - Set remaining registers based on LPM security.
 */
#ifndef BSP_TZ_CFG_LPMSAR
#define BSP_TZ_CFG_LPMSAR                                                                          \
	((RA_NOT_DEFINED > 0) ? BSP_CFG_CLOCKS_SECURE == 0                                         \
			      : (0x002E0116U | (BSP_CFG_CLOCKS_SECURE == 0)))
#endif
/* Deep Standby Interrupt Factor Security Attribution Register. */
#ifndef BSP_TZ_CFG_DPFSAR
#define BSP_TZ_CFG_DPFSAR ((RA_NOT_DEFINED > 0) ? 0U : 0xEF1FFFFFU)
#endif
/* Deep Standby Interrupt Factor Security Attribution Register 1. */
#ifndef BSP_TZ_CFG_DPFSAR1
#define BSP_TZ_CFG_DPFSAR1 ((RA_NOT_DEFINED > 0) ? 0U : 0x0000FFFFU)
#endif
/* RAM Standby Control Security Attribution Register. */
#ifndef BSP_TZ_CFG_RSCSAR
#define BSP_TZ_CFG_RSCSAR ((RA_NOT_DEFINED > 0) ? 0U : 0x00037FFFU)
#endif
/* Power Gating Control Security Attribution Register */
#ifndef BSP_TZ_CFG_PGCSAR
#define BSP_TZ_CFG_PGCSAR 0x000000FFU
#endif

/* Security attribution for CGC registers. */
#ifndef BSP_TZ_CFG_CGFSAR
#if BSP_CFG_CLOCKS_SECURE
/* Protect all CGC registers from Non-secure write access, except OCTACLK
 * (OCTACKCR/OCTACKDIVCR, NONSEC17), which the Non-secure OSPI_B driver
 * reprograms at runtime.
 * TODO: Make OCTACLK security attribution configurable instead of fixed.
 */
#define BSP_TZ_CFG_CGFSAR (1U << 17 | 0x00000000U)
#else
/* Allow Secure and Non-secure write access. */
#define BSP_TZ_CFG_CGFSAR (0x1F7F7FFDU)
#endif
#endif

/* Security attribution for Battery Backup registers. */
#ifndef BSP_TZ_CFG_BBFSAR
#define BSP_TZ_CFG_BBFSAR (0x7FU)
#endif

/* Security attribution for Battery Backup registers (VBTBKRn). */
#ifndef BSP_TZ_CFG_VBRSABAR
#define BSP_TZ_CFG_VBRSABAR (0xED00)
#endif

/* Security attribution for registers for IRQ channels. */
#ifndef BSP_TZ_CFG_ICUSARA
#define BSP_TZ_CFG_ICUSARA                                                                         \
	((((RENESAS_RA_ICUSARA_NS_EXT_IRQ0 > 0) ? 1U : 0U) << 0U) /* External IRQ0 */ |            \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ1 > 0) ? 1U : 0U) << 1U) /* External IRQ1 */ |            \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ2 > 0) ? 1U : 0U) << 2U) /* External IRQ2 */ |            \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ3 > 0) ? 1U : 0U) << 3U) /* External IRQ3 */ |            \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ4 > 0) ? 1U : 0U) << 4U) /* External IRQ4 */ |            \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ5 > 0) ? 1U : 0U) << 5U) /* External IRQ5 */ |            \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ6 > 0) ? 1U : 0U) << 6U) /* External IRQ6 */ |            \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ7 > 0) ? 1U : 0U) << 7U) /* External IRQ7 */ |            \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ8 > 0) ? 1U : 0U) << 8U) /* External IRQ8 */ |            \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ9 > 0) ? 1U : 0U) << 9U) /* External IRQ9 */ |            \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ10 > 0) ? 1U : 0U) << 10U) /* External IRQ10 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ11 > 0) ? 1U : 0U) << 11U) /* External IRQ11 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ12 > 0) ? 1U : 0U) << 12U) /* External IRQ12 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ13 > 0) ? 1U : 0U) << 13U) /* External IRQ13 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ14 > 0) ? 1U : 0U) << 14U) /* External IRQ14 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ15 > 0) ? 1U : 0U) << 15U) /* External IRQ15 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ16 > 0) ? 1U : 0U) << 16U) /* External IRQ16 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ17 > 0) ? 1U : 0U) << 17U) /* External IRQ17 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ18 > 0) ? 1U : 0U) << 18U) /* External IRQ18 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ19 > 0) ? 1U : 0U) << 19U) /* External IRQ19 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ20 > 0) ? 1U : 0U) << 20U) /* External IRQ20 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ21 > 0) ? 1U : 0U) << 21U) /* External IRQ21 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ22 > 0) ? 1U : 0U) << 22U) /* External IRQ22 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ23 > 0) ? 1U : 0U) << 23U) /* External IRQ23 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ24 > 0) ? 1U : 0U) << 24U) /* External IRQ24 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ25 > 0) ? 1U : 0U) << 25U) /* External IRQ25 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ26 > 0) ? 1U : 0U) << 26U) /* External IRQ26 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ27 > 0) ? 1U : 0U) << 27U) /* External IRQ27 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ28 > 0) ? 1U : 0U) << 28U) /* External IRQ28 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ29 > 0) ? 1U : 0U) << 29U) /* External IRQ29 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ30 > 0) ? 1U : 0U) << 30U) /* External IRQ30 */ |         \
	 (((RENESAS_RA_ICUSARA_NS_EXT_IRQ31 > 0) ? 1U : 0U) << 31U) /* External IRQ31 */)
#endif

/* TODO: check this configuration */
/* Security attribution for NMI registers. */
#ifndef BSP_TZ_CFG_ICUSARB
#define BSP_TZ_CFG_ICUSARB                                                                         \
	(0 ? 7U : 0U) /* Should follow the secure attribution set in AIRCR.BFHFNMINS. */
#endif

/* Security attribution for registers for DMAC channels */
#ifndef BSP_TZ_CFG_DMACCHSAR
#define BSP_TZ_CFG_DMACCHSAR                                                                       \
	((((RENESAS_RA_DMACCHSAR_NS_DMAC0 > 0) ? 1U : 0U) << 0U) /* DMACx Channel 0 */ |           \
	 (((RENESAS_RA_DMACCHSAR_NS_DMAC1 > 0) ? 1U : 0U) << 1U) /* DMACx Channel 1 */ |           \
	 (((RENESAS_RA_DMACCHSAR_NS_DMAC2 > 0) ? 1U : 0U) << 2U) /* DMACx Channel 2 */ |           \
	 (((RENESAS_RA_DMACCHSAR_NS_DMAC3 > 0) ? 1U : 0U) << 3U) /* DMACx Channel 3 */ |           \
	 (((RENESAS_RA_DMACCHSAR_NS_DMAC4 > 0) ? 1U : 0U) << 4U) /* DMACx Channel 4 */ |           \
	 (((RENESAS_RA_DMACCHSAR_NS_DMAC5 > 0) ? 1U : 0U) << 5U) /* DMACx Channel 5 */ |           \
	 (((RENESAS_RA_DMACCHSAR_NS_DMAC6 > 0) ? 1U : 0U) << 6U) /* DMACx Channel 6 */ |           \
	 (((RENESAS_RA_DMACCHSAR_NS_DMAC7 > 0) ? 1U : 0U) << 7U) /* DMACx Channel 7 */)
#endif

/* Security attribution registers for WUPEN0. */
#ifndef BSP_TZ_CFG_ICUSARE
#define BSP_TZ_CFG_ICUSARE ((RA_NOT_DEFINED > 0) ? 0U : 0xFF1D0000U)
#endif

/* Security attribution registers for WUPEN1. */
#ifndef BSP_TZ_CFG_ICUSARF
#define BSP_TZ_CFG_ICUSARF ((RA_NOT_DEFINED > 0) ? 0U : 0x0000FF88U)
#endif

/* Trusted Event Route Control Register for IELSR, DMAC.DELSR and ELC.ELSR. Note
 * that currently Trusted Event Route Control is not supported.
 */
#ifndef BSP_TZ_CFG_TEVTRCR
#define BSP_TZ_CFG_TEVTRCR (0)
#endif

/* Security attribution register for ELCR, ELSEGR0, ELSEGR1, ELSEGR2, ELSEGR3
 * Security Attribution.
 */
#ifndef BSP_TZ_CFG_ELCSARA
#define BSP_TZ_CFG_ELCSARA (0x0000001FU)
#endif

/* Set DTCSTSAR if the Secure program uses the DTC. */
#if RA_NOT_DEFINED == RA_NOT_DEFINED
#define BSP_TZ_CFG_DTC_USED (0U)
#else
#define BSP_TZ_CFG_DTC_USED (1U)
#endif

/* Security attribution for IPC */
#ifndef BSP_TZ_CFG_IPCSAR
#define BSP_TZ_CFG_IPCSAR                                                                          \
	((((RENESAS_RA_IPCSAR_NS_SAIPCSEM0 > 0) ? 1U : 0U) << 0U) | /* SAIPCSEM0 */                \
	 (((RENESAS_RA_IPCSAR_NS_SAIPCSEM1 > 0) ? 1U : 0U) << 1U) | /* SAIPCSEM1 */                \
	 (((RENESAS_RA_IPCSAR_NS_SAIPCNMI0 > 0) ? 1U : 0U) << 8U) | /* SAIPCNMI0 */                \
	 (((RENESAS_RA_IPCSAR_NS_SAIPCNMI1 > 0) ? 1U : 0U) << 9U) | /* SAIPCNMI1 */                \
	 (((RENESAS_RA_IPCSAR_NS_SAIPCIR0 > 0) ? 1U : 0U) << 16U) | /* IPC 00 */                   \
	 (((RENESAS_RA_IPCSAR_NS_SAIPCIR1 > 0) ? 1U : 0U) << 17U) | /* IPC 01 */                   \
	 (((RENESAS_RA_IPCSAR_NS_SAIPCIR2 > 0) ? 1U : 0U) << 18U) | /* IPC 10 */                   \
	 (((RENESAS_RA_IPCSAR_NS_SAIPCIR3 > 0) ? 1U : 0U) << 19U) /* IPC 11 */)
#endif

/* Security attribution of MRAM registers. */
#ifndef BSP_TZ_CFG_MSAR
#define BSP_TZ_CFG_MSAR                                                                            \
	(((BSP_CFG_CLOCKS_SECURE == 0) ? (1U << 1) : 0U) | /* MREFREQ */                           \
	 ((BSP_CFG_CLOCKS_SECURE == 0) ? (1U << 3) : 0U) | /* MRCFREQ */                           \
	 ((BSP_CFG_CLOCKS_SECURE == 0) ? (1U << 4) : 0U) /* MRCPFB */)
#endif

/* Security attribution for SRAM registers. */
#ifndef BSP_TZ_CFG_SRAMSAR
/* If the CGC registers are only accessible in Secure mode, than there is no
 * reason for Non Secure applications to access SRAM0WTEN and therefore there is
 * no reason to access PRCR2.
 */
#define BSP_TZ_CFG_SRAMSAR                                                                         \
	((((RENESAS_RA_SRAMSAR_NS_SRAMSA0 > 0) ? 1U : 0U) << 0U) | /* SRAMSA0 */                   \
	 (((RENESAS_RA_SRAMSAR_NS_SRAMSA1 > 0) ? 1U : 0U) << 1U) | /* SRAMSA1 */                   \
	 (((RENESAS_RA_SRAMSAR_NS_SRAMSA2 > 0) ? 1U : 0U) << 2U) | /* SRAMSA2 */                   \
	 (((RENESAS_RA_SRAMSAR_NS_SRAMSA3 > 0) ? 1U : 0U) << 3U) | /* SRAMSA3 */                   \
	 ((BSP_CFG_CLOCKS_SECURE == 0) ? (1U << 8U) : 0U) /* SRAMWTSA */)
#endif

/* SRAM ECC Region Security Attribution Register. */
#ifndef BSP_TZ_CFG_SRAMESAR
#define BSP_TZ_CFG_SRAMESAR (0x1U)
#endif

/* Security attribution for the DMAC Bus Master MPU settings. */
#ifndef BSP_TZ_CFG_MMPUSARA
/* The DMAC Bus Master MPU settings should align with the DMAC channel settings.
 */
#define BSP_TZ_CFG_MMPUSARA (BSP_TZ_CFG_DMACCHSAR)
#endif

/* Security Attribution Register A for BUS Control registers. */
#ifndef BSP_TZ_CFG_BUSSARA
#define BSP_TZ_CFG_BUSSARA (1U)
#endif
/* Security Attribution Register B for BUS Control registers. */
#ifndef BSP_TZ_CFG_BUSSARB
#define BSP_TZ_CFG_BUSSARB (1U)
#endif
/* Security Attribution Register C for BUS Control registers. */
#ifndef BSP_TZ_CFG_BUSSARC
#define BSP_TZ_CFG_BUSSARC (1U)
#endif

/* Enable Uninitialized Non-Secure Application Fallback. */
#ifndef BSP_TZ_CFG_NON_SECURE_APPLICATION_FALLBACK
#define BSP_TZ_CFG_NON_SECURE_APPLICATION_FALLBACK (1U)
#endif

/* Security attribution for Cache registers. */
#ifndef BSP_TZ_CFG_CACHESAR
/* Currently, the BL2 is not supported -> configure CACHE and FLAHS at NSPE */
#define BSP_TZ_CFG_CACHESAR (0x00000005U)
#endif

/* Used to create IELS values for the interrupt initialization table
 * g_interrupt_event_link_select.
 */
#define BSP_PRV_IELS_ENUM(vector) (ELC_##vector)

#ifndef BSP_CLOCK_CFG_MAIN_OSC_WAIT
#define BSP_CLOCK_CFG_MAIN_OSC_WAIT (9)
#endif

#ifndef BSP_CFG_DCACHE_ENABLED
#if (0U == BSP_CFG_CPU_CORE)
#define BSP_CFG_DCACHE_ENABLED (0)
#else
#define BSP_CFG_DCACHE_ENABLED (0)
#endif
#endif

#ifndef BSP_CFG_DCACHE_FORCE_WRITETHROUGH
#if (0U == BSP_CFG_CPU_CORE)
#define BSP_CFG_DCACHE_FORCE_WRITETHROUGH (0)
#else
#define BSP_CFG_DCACHE_FORCE_WRITETHROUGH (0)
#endif
#endif

#ifndef BSP_CFG_IOPORT_VOLTAGE_MODE_VCC
#define BSP_CFG_IOPORT_VOLTAGE_MODE_VCC (0)
#endif

#ifndef BSP_CFG_IOPORT_VOLTAGE_MODE_VCC2
#define BSP_CFG_IOPORT_VOLTAGE_MODE_VCC2 (0)
#endif

#ifndef BSP_CFG_SDRAM_ENABLED
#define BSP_CFG_SDRAM_ENABLED (0)
#endif

#ifndef BSP_CFG_SDRAM_TRAS
#define BSP_CFG_SDRAM_TRAS (6)
#endif

#ifndef BSP_CFG_SDRAM_TRCD
#define BSP_CFG_SDRAM_TRCD (3)
#endif

#ifndef BSP_CFG_SDRAM_TRP
#define BSP_CFG_SDRAM_TRP (3)
#endif

#ifndef BSP_CFG_SDRAM_TWR
#define BSP_CFG_SDRAM_TWR (2)
#endif

#ifndef BSP_CFG_SDRAM_TCL
#define BSP_CFG_SDRAM_TCL (3)
#endif

#ifndef BSP_CFG_SDRAM_TRFC
#define BSP_CFG_SDRAM_TRFC (937)
#endif

#ifndef BSP_CFG_SDRAM_TREFW
#define BSP_CFG_SDRAM_TREFW (8)
#endif

#ifndef BSP_CFG_SDRAM_INIT_ARFI
#define BSP_CFG_SDRAM_INIT_ARFI (10)
#endif

#ifndef BSP_CFG_SDRAM_INIT_ARFC
#define BSP_CFG_SDRAM_INIT_ARFC (8)
#endif

#ifndef BSP_CFG_SDRAM_INIT_PRC
#define BSP_CFG_SDRAM_INIT_PRC (3)
#endif

#ifndef BSP_CFG_SDRAM_MULTIPLEX_ADDR_SHIFT
#define BSP_CFG_SDRAM_MULTIPLEX_ADDR_SHIFT (1)
#endif

#ifndef BSP_CFG_SDRAM_ENDIAN_MODE
#define BSP_CFG_SDRAM_ENDIAN_MODE (0)
#endif

#ifndef BSP_CFG_SDRAM_ACCESS_MODE
#define BSP_CFG_SDRAM_ACCESS_MODE (1)
#endif

#ifndef BSP_CFG_SDRAM_BUS_WIDTH
#define BSP_CFG_SDRAM_BUS_WIDTH (1)
#endif

#ifndef BSP_CFG_OSPI_B_STARTUP_ENABLED
#define BSP_CFG_OSPI_B_STARTUP_ENABLED (0)
#endif

#endif /* BSP_MCU_FAMILY_CFG_H_ */
