/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RTE_DEVICE_H
#define RTE_DEVICE_H

/*
 * Flash channel configuration.
 *
 *   RTE_FLASH0 - 1 = enable, 0 = disable
 *
 * The flash geometry (program unit, sector size, base address) is configured
 * in config.cmake, not here.
 */
#define RTE_FLASH0          1

/*
 * USART channel configuration.
 *
 *   RTE_USARTx           - 1 = enable, 0 = disable
 *   RTE_USARTx_TXD_PORT  - TXD port number
 *   RTE_USARTx_TXD_PIN   - TXD pin number
 *   RTE_USARTx_RXD_PORT  - RXD port number
 *   RTE_USARTx_RXD_PIN   - RXD pin number
 *
 * PSEL is inferred from the channel: even (0,8)→4, odd (9)→5.
 * Each image only configures the pins of its stdio UART:
 * RTE_S_STDIO_USART for S, RTE_NS_STDIO_USART for NS.
 */

/* USART0: SCI0 @ P602(RXD0_B) P603(TXD0_B) — PMOD2 */
#define RTE_USART0          1
#define RTE_USART0_TXD_PORT 6
#define RTE_USART0_TXD_PIN  3
#define RTE_USART0_RXD_PORT 6
#define RTE_USART0_RXD_PIN  2

/* USART7: SCI7 @ P809(TXD7_B) P808(RXD7_B) — Arduino Uno D1/D0*/
#define RTE_USART7          1
#define RTE_USART7_TXD_PORT 8
#define RTE_USART7_TXD_PIN  9
#define RTE_USART7_RXD_PORT 8
#define RTE_USART7_RXD_PIN  8

/* USART8: SCI8 @ P132(TXD8) P133(RXD8) — J-Link */
#define RTE_USART8          1
#define RTE_USART8_TXD_PORT 13
#define RTE_USART8_TXD_PIN  2
#define RTE_USART8_RXD_PORT 13
#define RTE_USART8_RXD_PIN  3

/* USART used as stdio by the S image */
#define RTE_S_STDIO_USART   8

/* USART used as stdio by the NS image on each core */
#if (RA_CORE == RA_CM33)
#define RTE_NS_STDIO_USART  7
#elif (RA_CORE == RA_CM85)
#define RTE_NS_STDIO_USART  0
#endif

#endif /* RTE_DEVICE_H */
