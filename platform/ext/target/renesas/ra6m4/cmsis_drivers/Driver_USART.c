/*
 * Copyright (c) 2025 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * CMSIS UART Driver wrapper for Renesas FSP SCI UART
 */

#include "Driver_USART.h"
#include "hal_data.h"
#include "r_sci_uart.h"

#define ARM_USART_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_USART_API_VERSION,
    ARM_USART_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_USART_CAPABILITIES DriverCapabilities = {
    1, /* supports UART (Asynchronous) mode */
    0, /* supports Synchronous Master mode */
    0, /* supports Synchronous Slave mode */
    0, /* supports UART Single-wire mode */
    0, /* supports UART IrDA mode */
    0, /* supports UART Smart Card mode */
    0, /* Smart Card Clock generator */
    0, /* RTS Flow Control available */
    0, /* CTS Flow Control available */
    0, /* Transmit completed event */
    0, /* Signal receive character timeout event */
    0, /* RTS Line */
    0, /* CTS Line */
    0, /* DTR Line */
    0, /* DSR Line */
    0, /* DCD Line */
    0, /* RI Line */
    0, /* Signal CTS change event */
    0, /* Signal DSR change event */
    0, /* Signal DCD change event */
    0, /* Signal RI change event */
    0  /* Reserved */
};

typedef struct {
    ARM_USART_SignalEvent_t cb_event;
    uint32_t                flags;
} USART_INFO;

static USART_INFO USART0_Info = {0};

static ARM_DRIVER_VERSION ARM_USART_GetVersion(void)
{
    return DriverVersion;
}

static ARM_USART_CAPABILITIES ARM_USART_GetCapabilities(void)
{
    return DriverCapabilities;
}

static int32_t ARM_USART_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    fsp_err_t err;

    USART0_Info.cb_event = cb_event;

    err = R_SCI_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
    if (FSP_SUCCESS != err) {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

static int32_t ARM_USART_Uninitialize(void)
{
    R_SCI_UART_Close(&g_uart0_ctrl);
    USART0_Info.cb_event = NULL;
    return ARM_DRIVER_OK;
}

static int32_t ARM_USART_PowerControl(ARM_POWER_STATE state)
{
    switch (state) {
    case ARM_POWER_OFF:
    case ARM_POWER_FULL:
        return ARM_DRIVER_OK;
    case ARM_POWER_LOW:
    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
}

static int32_t ARM_USART_Send(const void *data, uint32_t num)
{
    if ((data == NULL) || (num == 0U)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    fsp_err_t err = R_SCI_UART_Write(&g_uart0_ctrl, (uint8_t *)data, num);
    return (FSP_SUCCESS == err) ? ARM_DRIVER_OK : ARM_DRIVER_ERROR;
}

static int32_t ARM_USART_Receive(void *data, uint32_t num)
{
    if ((data == NULL) || (num == 0U)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    fsp_err_t err = R_SCI_UART_Read(&g_uart0_ctrl, (uint8_t *)data, num);
    return (FSP_SUCCESS == err) ? ARM_DRIVER_OK : ARM_DRIVER_ERROR;
}

static int32_t ARM_USART_Transfer(const void *data_out, void *data_in, uint32_t num)
{
    (void)data_out;
    (void)data_in;
    (void)num;
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static uint32_t ARM_USART_GetTxCount(void) { return 0; }
static uint32_t ARM_USART_GetRxCount(void) { return 0; }

static int32_t ARM_USART_Control(uint32_t control, uint32_t arg)
{
    (void)control;
    (void)arg;
    return ARM_DRIVER_OK;
}

static ARM_USART_STATUS ARM_USART_GetStatus(void)
{
    ARM_USART_STATUS status = {0};
    return status;
}

static int32_t ARM_USART_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    (void)control;
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_USART_MODEM_STATUS ARM_USART_GetModemStatus(void)
{
    ARM_USART_MODEM_STATUS status = {0};
    return status;
}

/* USART0 Driver Control Block */
ARM_DRIVER_USART Driver_USART0 = {
    ARM_USART_GetVersion,
    ARM_USART_GetCapabilities,
    ARM_USART_Initialize,
    ARM_USART_Uninitialize,
    ARM_USART_PowerControl,
    ARM_USART_Send,
    ARM_USART_Receive,
    ARM_USART_Transfer,
    ARM_USART_GetTxCount,
    ARM_USART_GetRxCount,
    ARM_USART_Control,
    ARM_USART_GetStatus,
    ARM_USART_SetModemControl,
    ARM_USART_GetModemStatus
};