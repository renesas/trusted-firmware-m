/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RTE_PIN_CFG_H_
#define RTE_PIN_CFG_H_

#include <bsp_api.h>
#include <r_ioport.h>
#include <RTE_Device.h>

/*
 * Helpers to construct FSP ioport_pin_cfg_t entries from the raw
 * port/pin numbers defined in RTE_Device.h.
 *
 *   BSP_IO_PORT_xx_PIN_yy  = (port << 8) | pin      e.g. 0x0602 -> port 6, pin 2
 */
#define RTE_PORT_PIN(port, pin)  (((uint32_t)(port) << 8) | (uint32_t)(pin))

/* Convert raw PSEL number to FSP peripheral value */
#define RTE_PSEL(val)  ((uint32_t)(val) << IOPORT_PRV_PFS_PSEL_OFFSET)

/* Generic pin entry - psel is the final FSP IOPORT_PERIPHERAL_* value */
#define RTE_PIN_CFG(port, _pin, psel) \
    {.pin = RTE_PORT_PIN(port, _pin), \
     .pin_cfg = ((uint32_t) IOPORT_CFG_PERIPHERAL_PIN | (uint32_t)(psel))},

/* UART PSEL - derived from FSP IOPORT_PERIPHERAL_SCI* macros */
#define RTE_UART_PSEL(ch) \
    ((ch) & 1) ? IOPORT_PERIPHERAL_SCI1_3_5_7_9 : IOPORT_PERIPHERAL_SCI0_2_4_6_8

/* TX + RX pair for one UART channel (TXD first, then RXD) */
#define RTE_PINS_UART(ch) \
    RTE_PIN_CFG(RTE_USART##ch##_TXD_PORT, RTE_USART##ch##_TXD_PIN, RTE_UART_PSEL(ch)) \
    RTE_PIN_CFG(RTE_USART##ch##_RXD_PORT, RTE_USART##ch##_RXD_PIN, RTE_UART_PSEL(ch))

/*
 * Domain resolution - S is default, NS assignment from PSAR.
 * RENESAS_RA_PSAR_NS_SCIx compile definitions come from
 * s_ns_peripheral.cmake (1 = NS-accessible, 0 = S-only).
 */
#if DOMAIN_NS /* TFM NS image build */
  #define _RTE_HAS_USART0  (RTE_USART0 && RENESAS_RA_PSAR_NS_SCI0)
  #define _RTE_HAS_USART7  (RTE_USART7 && RENESAS_RA_PSAR_NS_SCI7)
  #define _RTE_HAS_USART8  (RTE_USART8 && RENESAS_RA_PSAR_NS_SCI8)
#else         /* TFM S image build */
  #define _RTE_HAS_USART0  (RTE_USART0 && !RENESAS_RA_PSAR_NS_SCI0)
  #define _RTE_HAS_USART7  (RTE_USART7 && !RENESAS_RA_PSAR_NS_SCI7)
  #define _RTE_HAS_USART8  (RTE_USART8 && !RENESAS_RA_PSAR_NS_SCI8)
#endif

#endif /* RTE_PIN_CFG_H_ */
