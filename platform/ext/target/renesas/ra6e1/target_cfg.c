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
/* Deliberately NOT hal_data.h: nothing here uses it, and once the Crypto stack is in the
 * e2 project the generated ra_gen/common_data.h it pulls in includes FSP's own
 * psa/crypto.h, which would shadow TF-M's for everything in platform_s. */

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
    /* RA6 has no PPC - peripheral protection is done via SAU/IDAU and the Renesas PSAR
     * registers, which FSP programs from the solution. See target_cfg.h. */
    return TFM_PLAT_ERR_SUCCESS;
}

/*
 * No-ops, present only so the shared isolation HAL links.
 *
 * tfm_hal_bind_boundary() guards these calls with
 * "if (periph_ppc_bank != PPC_SP_DO_NOT_CONFIGURE)", and every platform_data_t on this
 * port sets exactly that sentinel, so none is ever reached at run time. The call sites are
 * still COMPILED, so the symbols must resolve.
 *
 * If a future RA6 partition genuinely needs per-peripheral attribution, this is the wrong
 * place for it: that attribution lives in PSAR and belongs in the solution, not in a
 * TF-M-side emulation of a controller the part does not have.
 */
void ppc_configure_to_secure(ppc_bank_t bank, uint32_t pos)
{
    (void)bank;
    (void)pos;
}

void ppc_clr_secure_unpriv(ppc_bank_t bank, uint32_t pos)
{
    (void)bank;
    (void)pos;
}

void ppc_en_secure_unpriv(ppc_bank_t bank, uint32_t pos)
{
    (void)bank;
    (void)pos;
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