/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 * Copyright (c) 2018-2020 Arm Limited
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "target_cfg.h"
#include "common_target_cfg.h"
#include "region_defs.h"
#include "tfm_plat_defs.h"
#include "cmsis.h"
#include "bsp_api.h"
#include "hal_data.h"

/* To write into AIRCR register, 0x5FA value must be write to the VECTKEY field,
 * otherwise the processor ignores the write.
 */
#define SCB_AIRCR_WRITE_MASK ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos))

/* Memory region limits - initialized with region_defs.h values */
const struct memory_region_limits memory_regions = {
    .non_secure_code_start = NS_CODE_START,
    .non_secure_partition_base = NS_CODE_START,
    .non_secure_partition_limit = NS_CODE_LIMIT,
    .veneer_base = CMSE_VENEER_REGION_START,
    .veneer_limit = CMSE_VENEER_REGION_START + CMSE_VENEER_REGION_SIZE - 1,
#ifdef BL2
    .secondary_partition_base = SECONDARY_PARTITION_START,
    .secondary_partition_limit = SECONDARY_PARTITION_START + SECONDARY_PARTITION_SIZE - 1,
#endif
};

void sau_and_idau_cfg(void)
{
    /* SAU configuration is handled by TF-M's common ARMv8-M isolation framework */
    /* The framework will configure SAU based on region_defs.h and memory_regions */

    /* RA6M4 doesn't have IDAU - security attribution is done entirely via SAU */
}

enum tfm_plat_err_t mpc_init_cfg(void)
{
    /* RA6M4 does not have MPC - memory protection is done via SAU/MPU */
    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t ppc_init_cfg(void)
{
    /* RA6M4 does not have PPC - peripheral protection is done via SAU */
    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t enable_fault_handlers(void)
{
    /* Explicitly set secure fault priority to the highest */
    NVIC_SetPriority(SecureFault_IRQn, 0);

    /* Enables BUS, MEM, USG and Secure faults */
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk
                  | SCB_SHCSR_BUSFAULTENA_Msk
                  | SCB_SHCSR_MEMFAULTENA_Msk
                  | SCB_SHCSR_SECUREFAULTENA_Msk;
    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t system_reset_cfg(void)
{
    uint32_t reg_value = SCB->AIRCR;

    /* Clear SCB_AIRCR_VECTKEY value */
    reg_value &= ~(uint32_t)(SCB_AIRCR_VECTKEY_Msk);

    /* Enable system reset request only to the secure world */
    reg_value |= (uint32_t)(SCB_AIRCR_WRITE_MASK | SCB_AIRCR_SYSRESETREQS_Msk);

    SCB->AIRCR = reg_value;

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t init_debug(void)
{
    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt target state to NS configuration ----------*/
enum tfm_plat_err_t nvic_interrupt_target_state_cfg(void)
{
    /* Interrupt target state is set in bsp_irq_cfg() in bsp_irq.c
     * FSP's BSP initialization handles the NVIC_ITNS configuration for
     * routing interrupts to secure or non-secure state based on the
     * interrupt configuration in the RASC-generated code. */
    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt enabling for S peripherals ----------------*/
enum tfm_plat_err_t nvic_interrupt_enable(void)
{
    /* Platform-specific secure interrupts can be enabled here
     * Example:
     *   NVIC_EnableIRQ(UART0_S_IRQ);
     *   NVIC_EnableIRQ(TIMER0_S_IRQ);
     */
    return TFM_PLAT_ERR_SUCCESS;
}