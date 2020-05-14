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

#ifndef __RA6M4_DEVICE_CFG_H__
#define __RA6M4_DEVICE_CFG_H__

/**
 * \file device_cfg.h
 * \brief Configuration file native driver re-targeting
 *
 * \details This file can be used to add native driver specific macro
 *          definitions to select which peripherals are available in the build.
 *
 * This is a default device configuration file with all peripherals enabled.
 */

///*ARM UART Controller PL011*/
//#define UART0_PL011_S
//#define UART0_PL011_NS
//#define UART1_PL011_S
//#define UART1_PL011_NS
//
///* CMSDK Timers */
//#define CMSDK_TIMER0_S
//#define CMSDK_TIMER0_NS
//#define CMSDK_TIMER1_S
//#define CMSDK_TIMER1_NS
//
///* Flash controller */
#define DFLASH0_S
#define CFLASH0_S
//
///* Cadence QSPI Flash Controller */
//#define QSPI_IP6514E_S
//#define QSPI_IP6514E_NS
//
///* MT25QL Flash memory library */
//#define MT25QL_S
//#define MT25QL_NS
//
/* Default UART baud rate */
#define DEFAULT_UART_BAUDRATE 115200

#endif  /* __RA6M4_DEVICE_CFG_H__ */
