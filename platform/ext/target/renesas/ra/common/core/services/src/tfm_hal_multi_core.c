/*
 * Copyright (c) 2021-2024, Arm Limited. All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* SPE-side secondary-core boot and readiness synchronization. */

#include <stdint.h>

#include "utilities.h"
#include "platform_multicore.h"
#include "tfm_hal_multi_core.h"
#include "tfm_peripherals_def.h"

/* IPC event status value indicating that IRQ0 is pending */
#define RA_PLATFORM_MAILBOX_EVENT_IRQ0_PENDING    ((uint32_t) IPC_EVENT_IRQ0)

/* Each separately built image owns one common IPC control block. */
extern ipc_instance_ctrl_t ra_ipc_ctrl;

/* Open IPC, register the FSP ISR context, and start the remote core. */
void tfm_hal_boot_ns_cpu(uintptr_t start_addr)
{
    fsp_err_t err;

    (void) start_addr;

    err = platform_mailbox_open();
    if (err != FSP_SUCCESS) {
        tfm_core_panic();
    }

    R_FSP_IsrContextSet(MAILBOX_IRQ, &ra_ipc_ctrl);
    R_BSP_SecondaryCoreStart();
}

/* Notify the remote NSPE that the SPE is ready and wait for its reply. */
void tfm_hal_wait_for_ns_cpu_ready(void)
{
    ipc_event_t event;
    uint32_t event_status = 0U;
    fsp_err_t err;

    err = platform_mailbox_generate_event(IPC_GENERATE_EVENT_IRQ0);
    if (err != FSP_SUCCESS) {
        goto fatal_error;
    }

    do {
        err = platform_mailbox_get_event(&event);
        if (err == FSP_SUCCESS) {
            event_status = (uint32_t) event & (uint32_t) IPC_EVENT_IRQ0;
        }
    } while (event_status != RA_PLATFORM_MAILBOX_EVENT_IRQ0_PENDING);

    __DMB();

    return;

fatal_error:
    (void) platform_mailbox_close();
    tfm_core_panic();
}
