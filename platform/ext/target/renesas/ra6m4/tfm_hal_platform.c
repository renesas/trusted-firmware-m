/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tfm_hal_platform.h"
#include "target_cfg.h"
#include "region_defs.h"
#include "bsp_api.h"
#include "cmsis.h"
#include "tfm_platform_system.h"
#include "fih.h"

/* Linker script provides __StackSeal. TF-M SPM code expects __STACK_SEAL.
 * We provide __STACK_SEAL as a weak symbol in the same section. */
extern uint64_t __StackSeal;

/* Weak definition placed in msp_stack_seal_res section - same as __StackSeal */
__attribute__((weak, section(".msp_stack_seal_res")))
uint64_t __STACK_SEAL = 0xFEF5EDA5FEF5EDA5ULL;

FIH_RET_TYPE(enum tfm_hal_status_t) tfm_hal_platform_init(void)
{
    /* FSP BSP clock initialization (bsp_clock_init) is called automatically from
     * SystemInit() in system.c during Reset_Handler, before main() is entered.
     * This configures the system to 200 MHz from the 24 MHz XTAL:
     *   - XTAL: 24 MHz
     *   - PLL: (24 MHz / 3) * 25 = 200 MHz
     *   - ICLK (system clock): 200 MHz
     *
     * No additional clock *configuration* is needed here - BUT the cached
     * SystemCoreClock value must be recomputed: SystemInit() runs before
     * __PROGRAM_START() does the C-runtime init, and with the RASC config's
     * BSP_CFG_EARLY_INIT=0 the SystemCoreClock variable lives in plain .bss,
     * so it is zeroed immediately after SystemInit set it. Any FSP driver that
     * derives a peripheral clock via R_FSP_SystemClockHzGet() (== SystemCoreClock
     * >> divider) would then see 0 - e.g. R_FLASH_HP_Open() rejects it with
     * FSP_ERR_FCLK (FCLK below the 4 MHz minimum). g_clock_freq[] lives in
     * .ram_noinit and survives, so SystemCoreClockUpdate() restores the real
     * value. Idempotent; also done defensively in Driver_Flash.c.
     */
    SystemCoreClockUpdate();

    /* Note: target_cfg.h functions are called by TF-M framework */
    FIH_RET(fih_int_encode(TFM_HAL_SUCCESS));
}

void tfm_hal_system_reset(void)
{
    NVIC_SystemReset();
    while(1);
}

void tfm_hal_system_halt(void)
{
    __disable_irq();
    while(1) {
        __WFI();
    }
}

/* Non-Secure context initialization functions */
uint32_t tfm_hal_get_ns_VTOR(void)
{
    /* Non-secure vector table is at the start of non-secure code */
    return NS_CODE_START;
}

uint32_t tfm_hal_get_ns_MSP(void)
{
    /* Read the initial stack pointer from NS vector table */
    return *((uint32_t *)NS_CODE_START);
}

uint32_t tfm_hal_get_ns_entry_point(void)
{
    /* Read the reset handler address from NS vector table */
    return *((uint32_t *)(NS_CODE_START + 4));
}
/* Platform service implementations */
void tfm_platform_hal_system_reset(void)
{
    /* Use the same system reset as tfm_hal_system_reset */
    NVIC_SystemReset();
    while(1);
}

enum tfm_platform_err_t tfm_platform_hal_ioctl(tfm_platform_ioctl_req_t request,
                                                psa_invec *in_vec,
                                                psa_outvec *out_vec)
{
    /* Platform-specific IOCTL stub - can be extended for platform services */
    (void)request;
    (void)in_vec;
    (void)out_vec;

    /* Return not supported for now */
    return TFM_PLATFORM_ERR_NOT_SUPPORTED;
}
