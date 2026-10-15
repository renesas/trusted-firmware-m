/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "tfm_ns_mailbox.h"
#include "tfm_multi_core_api.h"

int32_t tfm_ns_mailbox_hal_init(struct ns_mailbox_queue_t *queue)
{
    /* Stub: will be suppported in other MRs*/
    return MAILBOX_INIT_ERROR;
}

int32_t tfm_ns_mailbox_hal_notify_peer(void)
{
    /* Stub: will be suppported in other MRs*/
    return MAILBOX_GENERIC_ERROR;
}

uint32_t tfm_ns_mailbox_hal_enter_critical(void)
{
    /* Stub: will be suppported in other MRs*/
    return MAILBOX_GENERIC_ERROR;
}

void tfm_ns_mailbox_hal_exit_critical(uint32_t state)
{
    /* Stub: will be suppported in other MRs*/
}

uint32_t tfm_ns_mailbox_hal_enter_critical_isr(void)
{
    /* Stub: will be suppported in other MRs*/
    return MAILBOX_GENERIC_ERROR;
}

void tfm_ns_mailbox_hal_exit_critical_isr(uint32_t state)
{
    /* Stub: will be suppported in other MRs*/
}

int32_t tfm_platform_ns_wait_for_s_cpu_ready(void)
{
    /* Stub: will be suppported in other MRs*/
    return MAILBOX_SUCCESS;
}
