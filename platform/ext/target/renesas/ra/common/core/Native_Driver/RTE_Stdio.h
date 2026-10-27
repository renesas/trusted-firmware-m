/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RTE_STDIO_H_
#define RTE_STDIO_H_

/*
 * Stdio console selection. The board's RTE_Device.h sets the console
 * channel of each image: RTE_S_STDIO_USART and RTE_NS_STDIO_USART.
 */
#include "RTE_Device.h"

/* 1 for the TFM NS image build, 0 for the S image build */
#if defined(DOMAIN_NS) && DOMAIN_NS
  #define RTE_STDIO_IS_NS  1
#else
  #define RTE_STDIO_IS_NS  0
#endif

/* Stdio console channel of the domain being built */
#if RTE_STDIO_IS_NS
  #define RTE_STDIO_USART  RTE_NS_STDIO_USART
#else
  #define RTE_STDIO_USART  RTE_S_STDIO_USART
#endif

/* USART driver instance of a channel, e.g. RA_USART_DRIVER(8) -> Driver_USART8 */
#define RA_USART_DRIVER_(ch)  Driver_USART##ch
#define RA_USART_DRIVER(ch)   RA_USART_DRIVER_(ch)

#endif /* RTE_STDIO_H_ */
