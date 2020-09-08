/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "target_cfg.h"
#include "tfm_spm_hal.h"
#include "uart_stdout.h"
#include "bsp_feature.h"
#include "renesas.h"

void bsp_clock_init (void);

/* platform-specific hw initialization */
__WEAK enum tfm_plat_err_t tfm_spm_hal_post_init_platform(void)
{
    bsp_clock_init();
    R_ICU->IELSR[4] = (uint32_t) (426);
    R_ICU->IELSR[5] = (uint32_t) (427);
    R_ICU->IELSR[6] = (uint32_t) (428);
    R_ICU->IELSR[7] = (uint32_t) (429);
    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t tfm_spm_hal_post_init(void)
{
    if (tfm_spm_hal_post_init_platform() != TFM_PLAT_ERR_SUCCESS) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }

    __enable_irq();
    stdio_init();

    return TFM_PLAT_ERR_SUCCESS;
}

__WEAK void tfm_spm_hal_system_reset(void)
{
    NVIC_SystemReset();
}
