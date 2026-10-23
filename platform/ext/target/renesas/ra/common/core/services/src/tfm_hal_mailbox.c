/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "load/interrupt_defs.h"
#include "tfm_hal_device_header.h"
#include "tfm_hal_interrupt.h"
#include "tfm_hal_mailbox.h"
#include "tfm_multi_core.h"

int32_t tfm_mailbox_hal_init(struct secure_mailbox_queue_t *s_queue)
{
    /* Stub: will be suppported in other MRs*/
    return MAILBOX_SUCCESS;
}

int32_t tfm_mailbox_hal_notify_peer(void)
{
    /* Stub: will be suppported in other MRs*/
    return MAILBOX_SUCCESS;
}

uint32_t tfm_mailbox_hal_enter_critical(void)
{
    /* Stub: will be suppported in other MRs*/
    return MAILBOX_SUCCESS;
}

void tfm_mailbox_hal_exit_critical(uint32_t state)
{
    /* Stub: will be suppported in other MRs*/
}

enum tfm_hal_status_t mailbox_irq_init(void *p_pt,
                                        const struct irq_load_info_t *p_ildi)
{
    /* Stub: will be suppported in other MRs*/
    return TFM_HAL_SUCCESS;
}
