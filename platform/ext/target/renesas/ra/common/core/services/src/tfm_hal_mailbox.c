/*
 * Copyright (c) 2021-2024, Arm Limited. All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "platform_multicore.h"
#include "tfm_hal_mailbox.h"

static void ra_mailbox_publish_empty(struct secure_mailbox_queue_t *s_queue)
{
    s_queue->ns_status = NULL;
    s_queue->ns_slot_count = 0U;
    s_queue->ns_slots = NULL;

    __DMB();
}

int32_t tfm_mailbox_hal_init(struct secure_mailbox_queue_t *s_queue)
{
    struct mailbox_init_t init_snapshot;
    fsp_err_t err;

    if (s_queue == NULL) {
        return MAILBOX_INVAL_PARAMS;
    }

    ra_mailbox_publish_empty(s_queue);

    /* Request the mailbox descriptor from the remote NSPE. */
    err = platform_mailbox_generate_event(IPC_GENERATE_EVENT_IRQ0);
    if (err != FSP_SUCCESS) {
        goto init_error;
    }

    while (platform_mailbox_buffer_get(&init_snapshot) != FSP_SUCCESS) {
        /* Wait for the remote NSPE to provide the mailbox descriptor. */
    }

    /*
     * platform_mailbox_buffer_get() has validated the descriptor,
     * status object, slot array, slot count, and size calculation.
     */
    s_queue->ns_status = init_snapshot.status;
    s_queue->ns_slot_count = init_snapshot.slot_count;
    s_queue->ns_slots = init_snapshot.slots;

    __DMB();

    /* Notify the remote NSPE that mailbox initialization is complete. */
    err = platform_mailbox_generate_event(IPC_GENERATE_EVENT_IRQ0);
    if (err != FSP_SUCCESS) {
        goto init_error;
    }

    return MAILBOX_SUCCESS;

init_error:
    ra_mailbox_publish_empty(s_queue);

    return MAILBOX_INIT_ERROR;
}

int32_t tfm_mailbox_hal_notify_peer(void)
{
    /* TODO: Fully implement this stub when SPE-to-NSPE notification is required */
    return MAILBOX_SUCCESS;
}

uint32_t tfm_mailbox_hal_enter_critical(void)
{
    /* TODO: Fully implement this stub when mailbox concurrency is supported */
    return 0U;
}

void tfm_mailbox_hal_exit_critical(uint32_t key)
{
    /* TODO: Fully implement this stub when mailbox concurrency is supported */
    (void) key;
}
