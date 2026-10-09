/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * SEGGER RTT stdout backend for TF-M on RA8M2.
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
#include <stdio.h>
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
/*
 * newlib's write() backend, so printf() reaches RTT.
 *
 * RA8M2_STDOUT_NO_WRITE suppresses it for builds that get _write() from somewhere else.
 * ns/syscalls_ns.c defines the whole newlib syscall set including an RTT-backed _write(),
 * and both files are needed on the non-secure side: this one for the stdio_*() backend
 * that TF-M's and tf-m-tests' logging call, that one for _close/_lseek/_read. Without the
 * guard the two collide at link with a duplicate _write.
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
 * The IAR equivalents. DLIB calls __write(), never _write(), so the GNU hook above leaves
 * IAR builds with no backend at all.
 *
 * TWO HOOKS ARE NEEDED, NOT ONE. TF-M's toolchain_IARARM.cmake links with
 *
 *     --semihosting
 *     --redirect __write=__write_buffered
 *
 * so every reference to __write resolves to DLIB's buffered writer instead. __write below is
 * then DEFINED BUT UNREACHABLE: the real path is
 *
 *     printf -> _Prout -> fputc -> fflushOne -> __write_buffered -> __dwrite -> __iar_sh_stdout
 *
 * ending in SEMIHOSTING - silent with no debugger attached, and a halt on the semihosting
 * breakpoint with one. __dwrite() is the hook that path actually calls; it is declared in
 * <LowLevelIOInterface.h> with __write's signature, and defining it here keeps the library's
 * dwrite.o and iarwstd.o out of the link entirely.
 *
 * __write is kept for builds that do not carry the redirect. Both are needed; neither alone
 * is sufficient. DECISIONS D107, D108.
 *
 * A NULL buffer is DLIB asking for a flush; RTT holds nothing back, so there is nothing
 * to do but report success.
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

/* Reached when the build does NOT redirect __write. */
size_t __write(int handle, const unsigned char *buffer, size_t size)
{
    return rtt_llio_write(handle, buffer, size);
}

/* Reached when it does - which TF-M's toolchain_IARARM.cmake always does. */
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
     * Make stdout unbuffered. This is the SECOND of two independent blockers on BL2's output;
     * the first is the __write redirect handled above. Keep both.
     *
     * DLIB buffers stdout and only calls the low-level writer when the FILE buffer fills or
     * something flushes it, and BL2 does neither - it logs a few hundred bytes through
     * MCUboot's BOOT_LOG_* (which expand to printf()) and then chainloads. So the bytes never
     * reach __dwrite() at all.
     *
     * Proven by three builds, differing only in these two fixes:
     *
     *   no setvbuf, no __dwrite  -> silent, WrOff 0, no halt   (buffer holds it)
     *   setvbuf,    no __dwrite  -> HALT in __iar_sh_stdout    (buffer released it;
     *                                                           backend was semihosting)
     *   setvbuf +   __dwrite     -> full log on RTT
     *
     * The middle one is the evidence that buffering alone blocks delivery: releasing it
     * changed the failure. Note what buffering did NOT cause - WrOff was 0 because output
     * went to semihosting, not because it was buffered. Buffering decided which failure you
     * saw, not where the bytes went.
     *
     * ROOT CAUSE, for the record: BL2 and the secure image print by different routes. TF-M's
     * SPM logging calls stdio_output_string() directly and never enters DLIB, so it was never
     * affected by either problem. Only MCUboot's printf-based logging was, which made the
     * fault look BL2-specific and therefore like a wrong RTT address.
     *
     * newlib is unaffected - it line-buffers stdout - so this is guarded to IAR rather than
     * applied to both. DECISIONS D107, D108, D110.
     */
    (void)setvbuf(stdout, NULL, _IONBF, 0);
#endif

    is_initialized = true;
}

void stdio_uninit(void)
{
    is_initialized = false;
}
