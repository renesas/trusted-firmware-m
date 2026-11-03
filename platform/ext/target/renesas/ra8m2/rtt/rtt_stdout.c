/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * SEGGER RTT stdout backend for TF-M on RA8M2.
 *
 * Replaces the common UART stdout backend (platform/ext/common/uart_stdout.c). It
 * implements the same stdio_* interface used by SPM and partition logging and by MCUboot
 * (BL2), but writes to SEGGER RTT over J-Link instead of a UART. This avoids UART wiring
 * and secure/non-secure peripheral contention.
 *
 * The platform CMakeLists builds this file when RA8M2_STDOUT_RTT is ON, and config.cmake
 * then turns PLATFORM_DEFAULT_UART_STDOUT off so the common uart_stdout.c is not compiled.
 * RA8M2_STDOUT_RTT=OFF selects the UART path through cmsis_drivers/Driver_USART.c, which
 * needs an SCI UART instance g_uart0 that no shipped e2 project has; see CONFIGURATION.md.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "SEGGER_RTT.h"

/* RTT up-channel for stdout. Channel 0 is the default for J-Link RTT Viewer and
 * `JLinkRTTClient`. */
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
/*
 * newlib's write() backend, so printf() reaches RTT.
 *
 * RA8M2_STDOUT_NO_WRITE removes it where _write() comes from elsewhere. On the non-secure
 * side ns/syscalls_ns.c defines the whole newlib syscall set, including an RTT-backed
 * _write(), and both files are needed: this one for the stdio_*() backend that TF-M and
 * tf-m-tests logging call, that one for _close/_lseek/_read. Without the guard the link
 * fails with a duplicate _write.
 */
#ifndef RA8M2_STDOUT_NO_WRITE
int _write(int fd, char *str, int len)
{
    (void)fd;
    return stdio_output_string(str, (uint32_t)len);
}
#endif /* RA8M2_STDOUT_NO_WRITE */

#elif defined(__ICCARM__)
/*
 * IAR equivalents. DLIB calls __write(), never _write(), so the GNU hook above does not
 * cover IAR builds.
 *
 * Two hooks are needed. TF-M's toolchain_IARARM.cmake links with
 *
 *     --semihosting
 *     --redirect __write=__write_buffered
 *
 * so every reference to __write resolves to DLIB's buffered writer, and __write below is
 * defined but unreachable. The real path is
 *
 *     printf -> _Prout -> fputc -> fflushOne -> __write_buffered -> __dwrite -> __iar_sh_stdout
 *
 * which ends in semihosting: silent with no debugger attached, and a halt on the
 * semihosting breakpoint with one. __dwrite() is the hook on that path. It is declared in
 * <LowLevelIOInterface.h> with __write's signature, and defining it here keeps the
 * library's dwrite.o and iarwstd.o out of the link.
 *
 * __write is kept for builds without the redirect. D107, D108.
 *
 * A NULL buffer is a DLIB flush request. RTT holds nothing back, so it reports success.
 */
#ifndef RA8M2_STDOUT_NO_WRITE
#include <LowLevelIOInterface.h>

static size_t rtt_llio_write(int handle, const unsigned char *buffer, size_t size)
{
    if (buffer == NULL) {
        return 0;
    }

    if ((handle != _LLIO_STDOUT) && (handle != _LLIO_STDERR)) {
        return (size_t)_LLIO_ERROR;
    }

    return (size_t)stdio_output_string((const char *)buffer, (uint32_t)size);
}

/* Reached when the build does not redirect __write. */
size_t __write(int handle, const unsigned char *buffer, size_t size)
{
    return rtt_llio_write(handle, buffer, size);
}

/* Reached when it does, which TF-M's toolchain_IARARM.cmake always does. */
size_t __dwrite(int handle, const unsigned char *buffer, size_t size)
{
    return rtt_llio_write(handle, buffer, size);
}
#endif /* RA8M2_STDOUT_NO_WRITE */

#endif

void stdio_init(void)
{
    SEGGER_RTT_Init();

#if defined(__ICCARM__)
    /*
     * Make stdout unbuffered. This is the second of two fixes BL2's output needs under IAR;
     * the first is the __dwrite hook above. Keep both.
     *
     * BL2 and the secure image print by different routes. TF-M's SPM logging calls
     * stdio_output_string() directly and never enters DLIB. MCUboot's BOOT_LOG_* expand to
     * printf(), which goes through DLIB. DLIB buffers stdout and calls the low-level writer
     * only when the FILE buffer fills or is flushed, and BL2 does neither: it logs a few
     * hundred bytes and then chainloads. Without this call the bytes never reach
     * __dwrite().
     *
     * D110 records the three builds that show each fix is needed on its own.
     *
     * newlib line-buffers stdout and is unaffected, so this is guarded to IAR.
     * DECISIONS D107, D108, D110.
     */
    (void)setvbuf(stdout, NULL, _IONBF, 0);
#endif

    is_initialized = true;
}

void stdio_uninit(void)
{
    is_initialized = false;
}
