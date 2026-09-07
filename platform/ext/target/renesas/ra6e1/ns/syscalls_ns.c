/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Newlib syscall stubs and weak function stubs for non-secure FreeRTOS application
 */

#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>

#ifdef RA6E1_STDOUT_RTT
#include "SEGGER_RTT.h"
#endif

/* Weak stub for HAL init - FSP applications provide their own implementation */
__attribute__((weak)) void g_hal_init(void) {
    /* Empty - FSP applications provide their own implementation */
}

/* Weak stubs for FreeRTOS TrustZone functions - needed for NS with FreeRTOS */
__attribute__((weak)) uint32_t vPortAllocateSecureContext(uint32_t ulSecureStackSize) {
    (void)ulSecureStackSize;
    return 0;  /* Stub - real implementation in FreeRTOS port */
}

__attribute__((weak)) void vPortFreeSecureContext(uint32_t *pulSecureContext) {
    (void)pulSecureContext;
    /* Stub - real implementation in FreeRTOS port */
}

/* Newlib syscall stubs - disable warnings from -Wl,-fatal-warnings */

int _close(int file) {
    (void)file;
    errno = ENOSYS;
    return -1;
}

int _fstat(int file, struct stat *st) {
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file) {
    (void)file;
    return 1;
}

int _lseek(int file, int ptr, int dir) {
    (void)file;
    (void)ptr;
    (void)dir;
    errno = ENOSYS;
    return -1;
}

int _read(int file, char *ptr, int len) {
    (void)file;
    (void)ptr;
    (void)len;
    errno = ENOSYS;
    return -1;
}

int _write(int file, char *ptr, int len) {
    (void)file;
#ifdef RA6E1_STDOUT_RTT
    /* Route the NS application's printf() to RTT. This is deliberately here rather than in
     * rtt/rtt_stdout.c: that file defines _write() too, and adding it to tfm_ns collides
     * with this one at link. The secure side uses rtt_stdout.c because it also needs the
     * stdio_* backend API that TF-M's SPM logging calls; NS needs only the newlib hook.
     *
     * _isatty() above returns 1, so newlib line-buffers stdout and a printf ending in \n
     * reaches RTT immediately - no fflush() needed in the application.
     */
    return (int)SEGGER_RTT_Write(0U, ptr, (unsigned)len);
#else
    (void)ptr;
    return len;  /* No backend - pretend we wrote everything to avoid errors */
#endif
}
