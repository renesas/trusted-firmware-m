/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Common Renesas IPC operations shared by SPE and remote NSPE builds. */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "tfm_mailbox.h"
#include "platform_multicore.h"

#ifndef DOMAIN_NS
#include "region_defs.h"
#include "tfm_hal_mailbox.h"

#if !defined(CPU1_NS_DATA_START) || !defined(CPU1_NS_RAM_DATA_SIZE)
#error "CPU1_NS_DATA_START and CPU1_NS_RAM_DATA_SIZE must be defined"
#endif
#endif

/* IPC channel used for the mailbox */
#define RA_MAILBOX_IPC_CHANNEL            UINT32_C(0)

ipc_instance_ctrl_t ra_ipc_ctrl = {0};

static const ipc_cfg_t ra_ipc_cfg = {
    .channel = RA_MAILBOX_IPC_CHANNEL,
    .irq = FSP_INVALID_VECTOR,
    .ipl = BSP_IRQ_DISABLED,
    .p_callback = NULL,
    .p_context = NULL,
};

#ifndef DOMAIN_NS
/* Check that an object lies completely in the remote NSPE data region. */
static bool platform_mailbox_ns_range_valid(uintptr_t address, size_t size)
{
    const uintptr_t start = (uintptr_t) CPU1_NS_DATA_START;
    const uintptr_t end = start + (uintptr_t) CPU1_NS_RAM_DATA_SIZE;

    if ((address < start) || (address >= end)) {
        return false;
    }

    return size <= (end - address);
}
#endif

/* Open the IPC hardware instance if it is not already open. */
fsp_err_t platform_mailbox_open(void)
{
    fsp_err_t err;

    if (ra_ipc_ctrl.open != 0) {
        return FSP_SUCCESS;
    }

    err = R_IPC_Open((ipc_ctrl_t *) &ra_ipc_ctrl, &ra_ipc_cfg);
    if (err != FSP_SUCCESS) {
        return err;
    }

    return FSP_SUCCESS;
}

/* Read and validate the remote mailbox descriptor without polling. */
fsp_err_t platform_mailbox_buffer_get(struct mailbox_init_t *p_mailbox_init)
{
#ifndef DOMAIN_NS
    uint32_t raw_address;
    uintptr_t ns_init_addr;

    if (p_mailbox_init == NULL) {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    if (ra_ipc_ctrl.open == 0) {
        return FSP_ERR_NOT_OPEN;
    }

    if (ra_ipc_ctrl.p_recv_reg->STA_b.RDY == 0U) {
        return FSP_ERR_NOT_FOUND;
    }

    /* Reading RXD consumes one word from the receive FIFO. */
    raw_address = ra_ipc_ctrl.p_recv_reg->RXD;
    ns_init_addr = (uintptr_t) raw_address;

    if (!platform_mailbox_ns_range_valid(ns_init_addr,
                                         sizeof(*p_mailbox_init))) {
        return FSP_ERR_INVALID_ADDRESS;
    }

    (void) memcpy(p_mailbox_init,
                  (const void *) ns_init_addr,
                  sizeof(*p_mailbox_init));

    __DMB();

    if ((p_mailbox_init->status == NULL) ||
        (p_mailbox_init->slots == NULL) ||
        (p_mailbox_init->slot_count == 0U) ||
        (p_mailbox_init->slot_count > NUM_MAILBOX_QUEUE_SLOT) ||
        (p_mailbox_init->slot_count >
         (SIZE_MAX / sizeof(struct mailbox_slot_t)))) {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    if (!platform_mailbox_ns_range_valid((uintptr_t) p_mailbox_init->status,
                                         sizeof(struct mailbox_status_t))) {
        return FSP_ERR_INVALID_ADDRESS;
    }

    if (!platform_mailbox_ns_range_valid((uintptr_t) p_mailbox_init->slots,
                                         sizeof(struct mailbox_slot_t) *
                                         p_mailbox_init->slot_count)) {
        return FSP_ERR_INVALID_ADDRESS;
    }

    return FSP_SUCCESS;
#else
    (void) p_mailbox_init;

    return FSP_ERR_UNSUPPORTED;
#endif
}

/* Send the mailbox descriptor address without polling. */
fsp_err_t platform_mailbox_buffer_set(const struct mailbox_init_t *p_mailbox_init)
{
#ifdef DOMAIN_NS
    if (p_mailbox_init == NULL) {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    if (ra_ipc_ctrl.open == 0) {
        return FSP_ERR_NOT_OPEN;
    }

    __DMB();

    return R_IPC_MessageSend((ipc_ctrl_t *) &ra_ipc_ctrl,
                             (uint32_t)(uintptr_t) p_mailbox_init);
#else
    (void)p_mailbox_init;

    return FSP_ERR_UNSUPPORTED;
#endif
}

/* Read and acknowledge the IRQ0 event status once without polling. */
fsp_err_t platform_mailbox_get_event(ipc_event_t *p_event)
{
    if (p_event == NULL) {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    if (ra_ipc_ctrl.open == 0) {
        return FSP_ERR_NOT_OPEN;
    }

    *p_event = (ipc_event_t) ra_ipc_ctrl.p_recv_reg->STA;
    ra_ipc_ctrl.p_recv_reg->CLR = (uint32_t) *p_event;
    __DMB();

    return FSP_SUCCESS;
}

/* Generate an IPC software event for the peer core. */
fsp_err_t platform_mailbox_generate_event(ipc_generate_event_t event)
{
    if ((uint32_t) event == 0U) {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    if (ra_ipc_ctrl.open == 0) {
        return FSP_ERR_NOT_OPEN;
    }

    __DMB();

    return R_IPC_EventGenerate((ipc_ctrl_t *) &ra_ipc_ctrl,event);
}

/* Close the IPC hardware instance if it is open. */
fsp_err_t platform_mailbox_close(void)
{
    if (ra_ipc_ctrl.open == 0) {
        return FSP_SUCCESS;
    }

    return R_IPC_Close((ipc_ctrl_t *) &ra_ipc_ctrl);
}
