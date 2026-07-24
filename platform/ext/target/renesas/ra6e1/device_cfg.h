/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DEVICE_CFG_H__
#define __DEVICE_CFG_H__

/* ARM UART */
#define DEFAULT_UART_BAUDRATE  115200
#define DEFAULT_UART_CONTROL   0  /* 8-N-1 configuration */

/* UART Driver */
#define UART0_DEV NS

/* STDIO UART Driver for TF-M logging */
extern ARM_DRIVER_USART Driver_USART0;
#define TFM_DRIVER_STDIO Driver_USART0

#endif /* __DEVICE_CFG_H__ */