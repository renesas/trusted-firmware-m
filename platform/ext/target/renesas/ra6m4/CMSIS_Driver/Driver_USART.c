/***********************************************************************************************************************
 * File Name    : driver_usart.c
 * Description  : This file contains uart driver specific API implementation.
 * ***********************************************************************************************************************/

/***********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
 * SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2019 Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/

 #include <Driver_USART.h>
 #include <string.h>
 #include <stdint.h>
 #include "r_sci_uart.h"
 #include "r_uart_api.h"
 #include "cmsis_driver_config.h"
 #include "RTE_Device.h"
#include "bsp_feature.h"
#include "renesas.h"
#include "psa_manifest/pid.h"
#include "psa/service.h"
#include "../psa-arch-tests/api-tests/platform/manifests/psa_manifest/driver_partition_psa.h"
extern void tfm_irq_handler(uint32_t partition_id, psa_signal_t signal, IRQn_Type irq_line);
 #ifndef ARG_UNUSED
  #define ARG_UNUSED(arg)    ((void) arg)
 #endif

/* Driver version */
 #define ARM_USART_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)

volatile uint32_t g_uart_evt = 1U;
static volatile uint32_t g_uart_evt_last = 0U;
static volatile uint32_t     g_num      = 0U;
extern volatile uint8_t tx_irq_triggered;
extern volatile uint8_t tx_irq_triggered_irq;
/* Driver Version */
static ARM_DRIVER_VERSION DriverVersion =
{
    ARM_USART_API_VERSION,
    ARM_USART_DRV_VERSION
};

/* Driver Capabilities */
static ARM_USART_CAPABILITIES DriverCapabilities =
{
    1,                                 /* supports UART (Asynchronous) mode */
    0,                                 /* supports Synchronous Master mode */
    0,                                 /* supports Synchronous Slave mode */
    0,                                 /* supports UART Single-wire mode */
    0,                                 /* supports UART IrDA mode */
    0,                                 /* supports UART Smart Card mode */
    0,                                 /* Smart Card Clock generator available */
    0,                                 /* RTS Flow Control available */
    0,                                 /* CTS Flow Control available */
    0,                                 /* Transmit completed event: \ref ARM_USARTx_EVENT_TX_COMPLETE */
    0,                                 /* Signal receive character timeout event: \ref ARM_USARTx_EVENT_RX_TIMEOUT */
    0,                                 /* RTS Line: 0=not available, 1=available */
    0,                                 /* CTS Line: 0=not available, 1=available */
    0,                                 /* DTR Line: 0=not available, 1=available */
    0,                                 /* DSR Line: 0=not available, 1=available */
    0,                                 /* DCD Line: 0=not available, 1=available */
    0,                                 /* RI Line: 0=not available, 1=available */
    0,                                 /* Signal CTS change event: \ref ARM_USARTx_EVENT_CTS */
    0,                                 /* Signal DSR change event: \ref ARM_USARTx_EVENT_DSR */
    0,                                 /* Signal DCD change event: \ref ARM_USARTx_EVENT_DCD */
    0,                                 /* Signal RI change event: \ref ARM_USARTx_EVENT_RI */
    0                                  /* Reserved */
};

typedef struct {
    uart_instance_t* dev;      /* UART device structure */
    uint32_t tx_nbr_bytes;             /* Number of bytes transfered */
    uint32_t rx_nbr_bytes;             /* Number of bytes recevied */
    ARM_USART_SignalEvent_t cb_event;  /* Callback function for events */
} UARTx_Resources;

static UARTx_Resources USART1_DEV = {
    .dev = &UART1_DEV,
    .tx_nbr_bytes = 0,
    .rx_nbr_bytes = 0,
    .cb_event = NULL,
};

static int32_t ARM_USARTx_Initialize(UARTx_Resources* uart_dev)
{
    /* Initializes generic UART driver */
    fsp_err_t fsp_err = FSP_SUCCESS;

    fsp_err = R_SCI_UART_Close(uart_dev->dev->p_ctrl);

    fsp_err = R_SCI_UART_Open(uart_dev->dev->p_ctrl, uart_dev->dev->p_cfg);
    if (FSP_SUCCESS != fsp_err)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

static int32_t ARM_USARTx_Uninitialize(UARTx_Resources* uart_dev)
{
    /* Disables and uninitializes generic UART driver */
    fsp_err_t fsp_err = FSP_SUCCESS;

    fsp_err = R_SCI_UART_Close(uart_dev->dev->p_ctrl);
    if (FSP_SUCCESS != fsp_err)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

static int32_t ARM_USARTx_Send(UARTx_Resources* uart_dev, const void *data,
                               uint32_t num)
{
    fsp_err_t fsp_err = FSP_SUCCESS;
    if ((num == 0) || (data == NULL))
    {
    	return ARM_DRIVER_ERROR;
    }

    while (!(g_uart_evt == 1U));
    g_uart_evt = 0U;

    fsp_err = R_SCI_UART_Write(uart_dev->dev->p_ctrl, (uint8_t const *)data, num);
    if (FSP_SUCCESS != fsp_err)
    {
    	return ARM_DRIVER_ERROR;
    }
//    while (!(g_uart_evt_last == 1U));
//    g_uart_evt_last = 0U;
    /*Save the transmitted number to be used in ARM_USART_GetTxCount*/
    g_num = num;

    return ARM_DRIVER_OK;
}

static int32_t ARM_USARTx_Receive(UARTx_Resources* uart_dev,
                                  void *data, uint32_t num)
{
    fsp_err_t fsp_err = FSP_SUCCESS;

    if (data == NULL)
    {
        return ARM_DRIVER_ERROR;
    }

    g_uart_evt = 0;

    fsp_err = R_SCI_UART_Read(uart_dev->dev->p_ctrl, data, num);
    if (FSP_SUCCESS != fsp_err)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Wait for UART Rx to complete */
    while (!g_uart_evt)
    {
        ;
    }

    if (g_uart_evt != UART_EVENT_RX_COMPLETE)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

static ARM_DRIVER_VERSION ARM_USART1_GetVersion (void)
{
    fsp_version_t ver;
    fsp_err_t     fsp_err = FSP_SUCCESS;

    fsp_err = R_SCI_UART_VersionGet(&ver);
    if (FSP_SUCCESS != fsp_err)
    {
        DriverVersion.api = (uint16_t) ((uint16_t) (ver.api_version_major << 8) | (ver.api_version_minor));
        DriverVersion.drv = (uint16_t) ((uint16_t) (ver.code_version_major << 8) | (ver.code_version_minor));
    }
    else
    {
        memset(&DriverVersion, 0, sizeof(DriverVersion));
    }

    return DriverVersion;
}

static ARM_USART_CAPABILITIES ARM_USART1_GetCapabilities (void)
{
    return DriverCapabilities;
}

static int32_t ARM_USART1_Initialize (ARM_USART_SignalEvent_t cb_event)
{
    ARG_UNUSED(cb_event);

    return ARM_USARTx_Initialize(&USART1_DEV);
}

static int32_t ARM_USART1_Uninitialize (void)
{
    return ARM_USARTx_Uninitialize(&USART1_DEV);
}

static int32_t ARM_USART1_PowerControl (ARM_POWER_STATE state)
{
    (void) state;                      /* Not used, avoid warning */

    /* Nothing to be done */
    return ARM_DRIVER_OK;
}

static uint32_t ARM_USART1_GetTxCount (void)
{
    /* Return the last successfully transmitted data count.
     * This implementation will not work in a multithreaded environment. */
    return g_num;
}

static int32_t ARM_USART1_Control (uint32_t control, uint32_t arg)
{
    (void) arg;                        /* Not used, avoid warning */
    (void) control;                    /* Not used, avoid warning */

    /* Nothing to be done */
    return ARM_DRIVER_OK;
}

static ARM_USART_STATUS ARM_USART1_GetStatus (void)
{
    /* Nothing to be done */
    ARM_USART_STATUS status = {0, 0, 0, 0, 0, 0, 0, 0};

    return status;
}

static int32_t ARM_USART1_Send(const void *data, uint32_t num)
{
    return ARM_USARTx_Send(&USART1_DEV, data, num);
    while (!(g_uart_evt_last == 1U));
    g_uart_evt_last = 0U;
}

static int32_t ARM_USART1_Receive(void *data, uint32_t num)
{
    return ARM_USARTx_Receive(&USART1_DEV, data, num);
}

void user_uart_callback (uart_callback_args_t * p_args)
{
    if(p_args->event == UART_EVENT_TX_DATA_EMPTY)
    {
    	g_uart_evt = 1U;
    }
    if(p_args->event == UART_EVENT_TX_COMPLETE)
    {
    	g_uart_evt_last = 1U;
    	tx_irq_triggered = 1U;
    }
}

ARM_DRIVER_USART Driver_USART1 =
{
    ARM_USART1_GetVersion,
    ARM_USART1_GetCapabilities,
	ARM_USART1_Initialize,
	ARM_USART1_Uninitialize,
    ARM_USART1_PowerControl,
	ARM_USART1_Send,
	ARM_USART1_Receive,
    NULL,
    ARM_USART1_GetTxCount,
    NULL,
    ARM_USART1_Control,
    ARM_USART1_GetStatus,
    NULL,
    NULL
};


static UARTx_Resources USART0_DEV = {
    .dev = &UART0_DEV,
    .tx_nbr_bytes = 0,
    .rx_nbr_bytes = 0,
    .cb_event = NULL,
};

static int32_t ARM_USART0_Initialize (ARM_USART_SignalEvent_t cb_event)
{
    ARG_UNUSED(cb_event);

    return ARM_USARTx_Initialize(&USART0_DEV);
}

static int32_t ARM_USART0_Uninitialize (void)
{
    return ARM_USARTx_Uninitialize(&USART0_DEV);
}

static uint32_t ARM_USART0_GetTxCount (void)
{
    /* Return the last successfully transmitted data count.
     * This implementation will not work in a multithreaded environment. */
    return g_num;
}

static int32_t ARM_USART0_Send(const void *data, uint32_t num)
{
    return ARM_USARTx_Send(&USART0_DEV, data, num);
}

static int32_t ARM_USART0_Receive(void *data, uint32_t num)
{
    return ARM_USARTx_Receive(&USART0_DEV, data, num);
}

void user_uart_callback_irq (uart_callback_args_t * p_args)
{
    if(p_args->event == UART_EVENT_TX_DATA_EMPTY)
    {
    	g_uart_evt = 1U;
    }
    if(p_args->event == UART_EVENT_TX_COMPLETE)
    {
    	g_uart_evt_last = 1U;
    	tx_irq_triggered_irq = 1U;
        __disable_irq();
        /* It is OK to call tfm_irq_handler directly from here, as we are already
         * in handler mode, and we will not be pre-empted as we disabled interrupts
         */
        tfm_irq_handler(DRIVER_PARTITION, DRIVER_UART_INTR_SIG, FF_TEST_UART_IRQ);
        __enable_irq();
    }
}
ARM_DRIVER_USART Driver_USART_IRQ =
{
    ARM_USART1_GetVersion,
    ARM_USART1_GetCapabilities,
	ARM_USART0_Initialize,
	ARM_USART0_Uninitialize,
    ARM_USART1_PowerControl,
	ARM_USART0_Send,
	ARM_USART0_Receive,
    NULL,
    ARM_USART0_GetTxCount,
    NULL,
    ARM_USART1_Control,
    ARM_USART1_GetStatus,
    NULL,
    NULL
};

                                 // BL2_CFG_UART_DEBUG_ENABLE
