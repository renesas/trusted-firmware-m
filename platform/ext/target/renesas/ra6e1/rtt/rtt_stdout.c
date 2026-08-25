/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * SEGGER RTT stdout backend for TF-M on RA6M4.
 *
 * This is a drop-in replacement for the common UART stdout backend
 * (platform/ext/common/uart_stdout.c). It implements the same stdio_*
 * interface used by the SPM/partition logging and MCUboot (BL2), but sends
 * output to SEGGER RTT over J-Link instead of a physical UART. This avoids
 * UART wiring and secure/non-secure peripheral contention during bring-up.
 *
 * Selection is done at the CMake level: when the port's stdout backend is set
 * to RTT, PLATFORM_DEFAULT_UART_STDOUT is turned OFF (so the common
 * uart_stdout.c is not compiled) and this file is compiled instead. To switch
 * back to the FSP SCI UART, re-enable PLATFORM_DEFAULT_UART_STDOUT and drop
 * this file from the build - no other code changes required. The FSP UART
 * driver (cmsis_drivers/Driver_USART.c) is left untouched either way.
 */

#include <stdbool.h>
#include <stdint.h>
#include "SEGGER_RTT.h"

/* RTT "Terminal" up-channel used for stdout. Channel 0 is the default that
 * J-Link RTT Viewer / `JLinkRTTClient` connect to. */
#define RTT_STDOUT_CHANNEL   0U

static bool is_initialized = false;

int stdio_output_string(const char *str, uint32_t len)
{
    if ((str == NULL) || (len == 0U)) {
        return 0;
    }

    /* SEGGER_RTT_Write returns the number of bytes copied into the up-buffer. */
    return (int)SEGGER_RTT_Write(RTT_STDOUT_CHANNEL, str, len);
}

void stdio_is_initialized_reset(void)
{
    is_initialized = false;
}

bool stdio_is_initialized(void)
{
    return is_initialized;
}

/* Redirect newlib's printf/puts family to RTT for the GNUARM toolchain, mirroring
 * the toolchain hooks in the common uart_stdout.c. */
#if defined(__GNUC__)
int _write(int fd, char *str, int len)
{
    (void)fd;
    return stdio_output_string(str, (uint32_t)len);
}
#endif

void stdio_init(void)
{
    SEGGER_RTT_Init();
    is_initialized = true;
}

void stdio_uninit(void)
{
    is_initialized = false;
}
