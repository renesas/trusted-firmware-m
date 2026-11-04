/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_MULTICORE_H
#define PLATFORM_MULTICORE_H

#include <stdint.h>

#include "tfm_mailbox.h"
#include "bsp_api.h"
#include "r_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Open the IPC hardware instance if it is not already open. */
fsp_err_t platform_mailbox_open(void);

/* Read and validate the remote mailbox descriptor */
fsp_err_t platform_mailbox_buffer_get(struct mailbox_init_t *p_mailbox_init);

/* Send the mailbox descriptor address */
fsp_err_t platform_mailbox_buffer_set(const struct mailbox_init_t *p_mailbox_init);

/* Read and acknowledge the IRQ0 event status once */
fsp_err_t platform_mailbox_get_event(ipc_event_t *p_event);

/* Generate an IPC software event for the peer core. */
fsp_err_t platform_mailbox_generate_event(ipc_generate_event_t event);

/* Close the IPC hardware instance if it is open. */
fsp_err_t platform_mailbox_close(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_MULTICORE_H */
