/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include "cmsis.h"
#include "tfm_hal_interrupt.h"
#include "tfm_hal_defs.h"
#include "region_defs.h"

enum tfm_hal_status_t tfm_hal_irq_enable(uint32_t irq_num)
{
    if (irq_num >= NVIC_USER_IRQ_NUMBER) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    NVIC_EnableIRQ(irq_num);
    return TFM_HAL_SUCCESS;
}

enum tfm_hal_status_t tfm_hal_irq_disable(uint32_t irq_num)
{
    if (irq_num >= NVIC_USER_IRQ_NUMBER) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    NVIC_DisableIRQ(irq_num);
    return TFM_HAL_SUCCESS;
}

enum tfm_hal_status_t tfm_hal_irq_clear_pending(uint32_t irq_num)
{
    if (irq_num >= NVIC_USER_IRQ_NUMBER) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    NVIC_ClearPendingIRQ(irq_num);
    return TFM_HAL_SUCCESS;
}

enum tfm_hal_status_t tfm_hal_irq_set_priority(uint32_t irq_num,
                                                 uint32_t priority)
{
    if (irq_num >= NVIC_USER_IRQ_NUMBER) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    NVIC_SetPriority(irq_num, priority);
    return TFM_HAL_SUCCESS;
}