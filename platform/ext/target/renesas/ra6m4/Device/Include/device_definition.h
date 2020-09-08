/*
 * Copyright (c) 2017-2019 Arm Limited. All rights reserved.
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
 * \file device_definition.h
 * \brief The structure definitions in this file are exported based on the
 * peripheral definitions from device_cfg.h.
 * This file is meant to be used as a helper for baremetal
 * applications and/or as an example of how to configure the generic
 * driver structures.
 */

#ifndef __RA6M4_DEVICE_DEFINITION_H__
#define __RA6M4_DEVICE_DEFINITION_H__

#include "device_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ======= Peripheral configuration structure declarations ======= */
/** ARM UART RA6M4 driver structures */
#ifdef UART0_RA6M4_S
#include "r_sci_uart.h"
extern uart_instance_t UART0_RA6M4_DEV_S;
void user_uart_callback (uart_callback_args_t * p_args);
#endif
#ifdef UART0_RA6M4_NS
#include "r_sci_uart.h"
extern uart_instance_t UART0_RA6M4_DEV_NS;
void user_uart_callback_ns (uart_callback_args_t * p_args);
#endif
#ifdef UART1_RA6M4_S
#include "r_sci_uart.h"
extern struct uart_instance_t UART1_RA6M4_DEV_S;
#endif
#ifdef UART1_RA6M4_NS
#include "r_sci_uart.h"
extern struct uart_instance_t UART1_RA6M4_DEV_NS;
#endif

#if defined(DFLASH0_S) || defined(CFLASH0_S)
#include "r_flash_hp.h"
extern flash_instance_t FLASH0_DEV_S;
#endif


#ifdef __cplusplus
}
#endif

#endif  /* __RA6M4_DEVICE_DEFINITION_H__ */
