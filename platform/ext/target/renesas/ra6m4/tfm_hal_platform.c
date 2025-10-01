/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tfm_hal_platform.h"
#include "target_cfg.h"
#include "bsp_api.h"
#include "cmsis.h"

enum tfm_hal_status_t tfm_hal_platform_init(void)
{
    if (target_cfg_init() != TFM_PLAT_ERR_SUCCESS) {
        return TFM_HAL_ERROR_GENERIC;
    }

    return TFM_HAL_SUCCESS;
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