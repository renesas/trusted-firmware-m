/*
 * Copyright (c) 2021-2024, Arm Limited. All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Remote NSPE mailbox initialization and runtime notification HAL. */

#include <stddef.h>
#include <stdint.h>

#include "tfm_ns_mailbox.h"
#include "platform_multicore.h"
#include "tfm_multi_core_api.h"

/* IPC event status values for the remote NSPE mailbox */
#define RA_PLATFORM_MAILBOX_EVENT_IRQ0_NONE       UINT32_C(0)
#define RA_PLATFORM_MAILBOX_EVENT_IRQ0_PENDING    ((uint32_t)IPC_EVENT_IRQ0)

/* Open IPC, wait for SPE readiness, and acknowledge remote readiness. */
int32_t tfm_platform_ns_wait_for_s_cpu_ready(void)
{
    ipc_event_t event;
    fsp_err_t err;
    uint32_t event_status = RA_PLATFORM_MAILBOX_EVENT_IRQ0_NONE;

    err = platform_mailbox_open();
    if (err != FSP_SUCCESS) {
        (void) platform_mailbox_close();
        return err;
    }

    do {
        err = platform_mailbox_get_event(&event);
        if (err == FSP_SUCCESS) {
            event_status = (uint32_t) event &
                           (uint32_t) IPC_EVENT_IRQ0;
        }
    } while (event_status != RA_PLATFORM_MAILBOX_EVENT_IRQ0_PENDING);

    __DSB();

    err = platform_mailbox_generate_event(IPC_GENERATE_EVENT_IRQ0);
    if (err != FSP_SUCCESS) {
        (void) platform_mailbox_close();
        return err;
    }

    return MAILBOX_SUCCESS;
}

/* Exchange the mailbox descriptor with the SPE. */
int32_t tfm_ns_mailbox_hal_init(struct ns_mailbox_queue_t *queue)
{
    struct mailbox_init_t ns_init MAILBOX_ALIGN;
    ipc_event_t event;
    fsp_err_t err;
    uint32_t event_status = RA_PLATFORM_MAILBOX_EVENT_IRQ0_NONE;

    if (queue == NULL) {
        return MAILBOX_INIT_ERROR;
    }

    ns_init.status = &queue->status;
    ns_init.slot_count = NUM_MAILBOX_QUEUE_SLOT;
    ns_init.slots = &queue->slots[0];

    /*
     * Wait for the SPE to request the remote NSPE mailbox descriptor.
     * The SPE generates IPC IRQ0 after it is ready to receive the
     * descriptor address.
     */
    do {
        err = platform_mailbox_get_event(&event);
        if (err == FSP_SUCCESS) {
            event_status = (uint32_t) event &
                           (uint32_t) IPC_EVENT_IRQ0;
        }
    } while (event_status != RA_PLATFORM_MAILBOX_EVENT_IRQ0_PENDING);

    __DSB();

    /*
     * Send the address of the remote NSPE mailbox descriptor to the SPE.
     */
    err = platform_mailbox_buffer_set(&ns_init);
    if (err != FSP_SUCCESS) {
        goto init_error;
    }

    /*
     * Wait for the SPE to acknowledge that it has received, validated,
     * and accepted the remote NSPE mailbox descriptor.
     */
    do {
        err = platform_mailbox_get_event(&event);
        if (err == FSP_SUCCESS) {
            event_status = (uint32_t) event &
                           (uint32_t) IPC_EVENT_IRQ0;
        }
    } while (event_status != RA_PLATFORM_MAILBOX_EVENT_IRQ0_PENDING);

    __DSB();

    return MAILBOX_SUCCESS;

init_error:
    (void) platform_mailbox_close();

    return MAILBOX_INIT_ERROR;
}

/* Notify the SPE that a runtime mailbox request is pending. */
int32_t tfm_ns_mailbox_hal_notify_peer(void)
{
    fsp_err_t err;

    err = platform_mailbox_generate_event(IPC_GENERATE_EVENT_IRQ0);

    return (err == FSP_SUCCESS) ? MAILBOX_SUCCESS :
                                  MAILBOX_INIT_ERROR;
}

/* Enter the single-request mailbox critical section. */
uint32_t tfm_ns_mailbox_hal_enter_critical(void)
{
    /* TODO: Fully implement this stub when NS mailbox concurrency is supported */
    return 0U;
}

/* Exit the single-request mailbox critical section. */
void tfm_ns_mailbox_hal_exit_critical(uint32_t key)
{
    /* TODO: Fully implement this stub when NS mailbox concurrency is supported */
    (void) key;
}
