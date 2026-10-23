/*
 * Copyright (c) 2021-2024, Arm Limited. All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "common_target_cfg.h"
#include "tfm_hal_device_header.h"
#include "tfm_hal_platform.h"
#include "tfm_plat_defs.h"
#include "uart_stdout.h"

#define ICU_BITS_PER_WORD    (32U)

#if BSP_MULTICORE_PROJECT

static void platform_icu_init(void)
{
    uint32_t intselr_count = sizeof(R_ICU->INTSELR) / sizeof(R_ICU->INTSELR[0]);
    uint32_t volatile *p_icusarg = &R_CPSCU->ICUSARJ;

    /* At boot time, the CM85 SPE will:
     *  - Set all interrupts route to CM33 core
     *  - Set all CM33 NSPE event link attribute to non-secure
     *  - Set all CM85 SPE's event link attribute to secure, and other to NS attribute
     * If the CM85 NSPE need to use an interrupt line, the NPSE firmware need to:
     *  - Set that interrupt request to CM85 core via interrupt routing service
     */

    /* Route all interrupt request to CM33 */
    for (uint32_t i = 0U; i < intselr_count; i++) {
        R_ICU->INTSELR[i] = UINT32_MAX;
    }

    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_SAR);

    /* Set the CM33's event link attribute to non-secure */
    for (uint32_t i = 0U; i < BSP_ICU_VECTOR_MAX_ENTRIES / ICU_BITS_PER_WORD; i++) {
        p_icusarg[i]  = UINT32_MAX;
    }

    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_SAR);
}
#endif

enum tfm_hal_status_t tfm_hal_platform_init(void)
{
#if BSP_MULTICORE_PROJECT
    platform_icu_init();
#endif

    __enable_irq();
    stdio_init();

    return TFM_HAL_SUCCESS;
}

/*
 * PPC (Peripheral Protection Controller) functions.
 * These functions are provided as stubs since peripheral security is configured
 * by the FSP during early boot.
 *
 * TODO: Provide PPC support when enabling higher isolation levels.
 * Reference: tfm_hal_isolation_v8m.c implementation.
 */

void ppc_configure_to_secure(ppc_bank_t bank, uint32_t pos)
{
    (void)bank;
    (void)pos;
}

void ppc_configure_to_non_secure(ppc_bank_t bank, uint32_t pos)
{
    (void)bank;
    (void)pos;
}

void ppc_en_secure_unpriv(ppc_bank_t bank, uint32_t pos)
{
    (void)bank;
    (void)pos;
}

void ppc_clr_secure_unpriv(ppc_bank_t bank, uint32_t pos)
{
    (void)bank;
    (void)pos;
}
