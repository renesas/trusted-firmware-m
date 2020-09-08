/*
 * Copyright (c) 2017-2020 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file device_definition.c
 * \brief This file defines exports the structures based on the peripheral
 * definitions from device_cfg.h.
 * This file is meant to be used as a helper for baremetal
 * applications and/or as an example of how to configure the generic
 * driver structures.
 */

#include "device_cfg.h"
#include "device_definition.h"
#include "platform_base_address.h"
#include "tfm_plat_defs.h"

/* ======= Peripheral configuration structure definitions ======= */

#ifdef UART0_RA6M4_S
void user_uart_callback(uart_callback_args_t * p_args);
#define VECTOR_NUMBER_SCI7_RXI ((IRQn_Type) 4) /* SCI0 RXI (Receive data full) */
#define VECTOR_NUMBER_SCI7_TXI ((IRQn_Type) 5) /* SCI0 TXI (Transmit data empty) */
#define VECTOR_NUMBER_SCI7_TEI ((IRQn_Type) 6) /* SCI0 TEI (Transmit end) */
#define VECTOR_NUMBER_SCI7_ERI ((IRQn_Type) 7) /* SCI0 ERI (Receive error) */

static sci_uart_instance_ctrl_t g_uart0_ctrl;

static baud_setting_t g_uart0_baud_setting =
{
/* Baud rate calculated with 0.469% error. */.abcse = 0,
  .abcs = 0, .bgdm = 1, .cks = 1, .brr = 161, .mddr = (uint8_t) 256, .brme = false };

/** UART extended configuration for UARTonSCI HAL driver */
static sci_uart_extended_cfg_t g_uart0_cfg_extend =
{ .clock = SCI_UART_CLOCK_INT,
  .rx_edge_start = SCI_UART_START_BIT_FALLING_EDGE,
  .noise_cancel = SCI_UART_NOISE_CANCELLATION_DISABLE,
  .rx_fifo_trigger = SCI_UART_RX_FIFO_TRIGGER_MAX,
  .p_baud_setting = &g_uart0_baud_setting,
  .uart_mode = UART_MODE_RS232,
  .ctsrts_en = SCI_UART_CTSRTS_RTS_OUTPUT,
  .flow_control_pin = (bsp_io_port_pin_t) (0xFFFFU),
    };

/** UART interface configuration */
static uart_cfg_t g_uart0_cfg =
{ .channel = 7, .data_bits = UART_DATA_BITS_8, .parity = UART_PARITY_OFF, .stop_bits = UART_STOP_BITS_1, .p_callback =
          user_uart_callback,
  .p_context = NULL, .p_extend = &g_uart0_cfg_extend,
  .p_transfer_tx = NULL,
  .p_transfer_rx = NULL,
  .rxi_ipl = (12),
  .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (12),
  .rxi_irq = VECTOR_NUMBER_SCI7_RXI,
  .txi_irq = VECTOR_NUMBER_SCI7_TXI,
  .tei_irq = VECTOR_NUMBER_SCI7_TEI,
  .eri_irq = VECTOR_NUMBER_SCI7_ERI,
        };

 /* Instance structure to use this module. */
uart_instance_t UART0_RA6M4_DEV_S =
{ .p_ctrl = &g_uart0_ctrl, .p_cfg = &g_uart0_cfg, .p_api = &g_uart_on_sci };
#endif

#ifdef UART0_RA6M4_NS
void user_uart_callback(uart_callback_args_t * p_args);
#define VECTOR_NUMBER_SCI0_RXI ((IRQn_Type) 0) /* SCI0 RXI (Receive data full) */
#define VECTOR_NUMBER_SCI0_TXI ((IRQn_Type) 1) /* SCI0 TXI (Transmit data empty) */
#define VECTOR_NUMBER_SCI0_TEI ((IRQn_Type) 2) /* SCI0 TEI (Transmit end) */
#define VECTOR_NUMBER_SCI0_ERI ((IRQn_Type) 3) /* SCI0 ERI (Receive error) */

static sci_uart_instance_ctrl_t g_uart0_ctrl_ns;

static baud_setting_t g_uart0_baud_setting_ns =
{
/* Baud rate calculated with 0.469% error. */.abcse = 0,
  .abcs = 0, .bgdm = 1, .cks = 1, .brr = 161, .mddr = (uint8_t) 256, .brme = false };

/** UART extended configuration for UARTonSCI HAL driver */
static sci_uart_extended_cfg_t g_uart0_cfg_extend_ns =
{ .clock = SCI_UART_CLOCK_INT,
  .rx_edge_start = SCI_UART_START_BIT_FALLING_EDGE,
  .noise_cancel = SCI_UART_NOISE_CANCELLATION_DISABLE,
  .rx_fifo_trigger = SCI_UART_RX_FIFO_TRIGGER_MAX,
  .p_baud_setting = &g_uart0_baud_setting_ns,
  .uart_mode = UART_MODE_RS232,
  .ctsrts_en = SCI_UART_CTSRTS_RTS_OUTPUT,
  .flow_control_pin = (bsp_io_port_pin_t) (0xFFFFU),
    };

/** UART interface configuration */
static uart_cfg_t g_uart0_cfg_ns =
{ .channel = 0, .data_bits = UART_DATA_BITS_8, .parity = UART_PARITY_OFF, .stop_bits = UART_STOP_BITS_1, .p_callback =
		user_uart_callback_ns,
  .p_context = NULL, .p_extend = &g_uart0_cfg_extend_ns,
  .p_transfer_tx = NULL,
  .p_transfer_rx = NULL,
  .rxi_ipl = (12),
  .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (12),
  .rxi_irq = VECTOR_NUMBER_SCI0_RXI,
  .txi_irq = VECTOR_NUMBER_SCI0_TXI,
  .tei_irq = VECTOR_NUMBER_SCI0_TEI,
  .eri_irq = VECTOR_NUMBER_SCI0_ERI,
        };

 /* Instance structure to use this module. */
uart_instance_t UART0_RA6M4_DEV_NS =
{ .p_ctrl = &g_uart0_ctrl_ns, .p_cfg = &g_uart0_cfg_ns, .p_api = &g_uart_on_sci };
#endif

/** Flash driver structures */
#if defined(DFLASH0_S) || defined(CFLASH0_S)
static flash_hp_instance_ctrl_t flash_ctrl = {0};

static flash_cfg_t const flash_cfg =
{
    .p_callback     = NULL,
    .p_context      = 0,
    .p_extend       = NULL,
    .data_flash_bgo = false,
    .ipl            = 5,
    .irq            = FSP_INVALID_VECTOR,
};

flash_instance_t FLASH0_DEV_S =
{
 .p_ctrl = &flash_ctrl,
 .p_cfg = &flash_cfg
};
#endif
