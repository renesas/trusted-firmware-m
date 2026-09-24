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

/*
 * Deliberately EMPTY - FSP programs the SAU, not TF-M.
 *
 * The earlier comment here claimed "SAU configuration is handled by TF-M's common ARMv8-M
 * isolation framework". That is not what happens. TF-M's common code calls this hook and
 * expects the PLATFORM to program the SAU; it has no generic implementation. On this port the
 * work is done by FSP's R_BSP_SecurityInit() (ra/fsp/src/bsp/mcu/all/bsp_security.c), which
 * BSP init calls before TF-M's platform init runs, and which derives every boundary from the
 * generated partition macros - the same solution.xml partitioning flash_layout.h and
 * region_defs.h are built from. So there is exactly one description of the memory map and
 * TF-M consumes it rather than restating it.
 *
 * Verify rather than trust this comment: R_BSP_SecurityInit must be present in the linked
 * secure image.
 *     arm-none-eabi-nm bin/tfm_s.axf | grep R_BSP_SecurityInit
 * If it is ever absent, nothing programs the SAU and the non-secure image will fault on its
 * first access - at which point this hook is where the replacement belongs.
 *
 * Related: ra8m2_ddsc.c supplies the four gp_ddsc_{I,D}TCM_* symbols that the same function
 * needs for M85 TCM attribution, because the port excludes FSP's bsp_linker.c.
 *
 * IDAU: RA8M2 has no IDAU in the Armv8-M sense. Attribution is the SAU (8 regions, per the
 * device rzone) plus the Renesas security-attribution registers - PSCU, and TZF/MSAOAD for
 * operation-after-detection - all of which bsp_security.c programs from the solution.
 */
void sau_and_idau_cfg(void)
{
}

enum tfm_plat_err_t mpc_init_cfg(void)
{
    /* No MPC on this part. Memory attribution is the SAU (see sau_and_idau_cfg above) and,
     * at isolation 2/3, the MPU via TF-M's own tfm_hal_isolation_v8m.c. */
    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t ppc_init_cfg(void)
{
    /* No PPC on this part - peripheral attribution is the Renesas PSAR/PSCU registers, which
     * bsp_security.c programs from the solution's peripheral slot assignments. See
     * target_cfg.h. */
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
 * If a future partition genuinely needs per-peripheral attribution, this is the wrong place
 * for it: that attribution lives in PSAR/PSCU and belongs in the solution, not in a TF-M-side
 * emulation of a controller the part does not have.
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