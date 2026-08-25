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
    /* Minimal implementation - could be enhanced to use UART */
    (void)file;
    (void)ptr;
    (void)len;
    return len;  /* Pretend we wrote everything to avoid errors */
}
