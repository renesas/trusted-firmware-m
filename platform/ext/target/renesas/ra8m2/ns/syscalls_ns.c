/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * C library retargeting and weak function stubs for the non-secure application.
 *
 * The two toolchains retarget stdout through different interfaces, so the bodies below are
 * split. GCC/newlib calls the _close/_fstat/_isatty/_lseek/_read/_write syscall family; IAR's
 * DLIB calls __write()/__read() from LowLevelIOInterface.h and has no <sys/stat.h> anywhere
 * in the installation, so that header must stay out of the IAR branch.
 *
 * The weak stubs are shared: TF-M's IAR builds compile with -e (language extensions), so
 * iccarm accepts the GCC __attribute__((weak)) spelling. Without -e iccarm rejects it with
 * Pe079/Pe130, so the stubs depend on that flag.
 */

#include <stdint.h>

#ifdef RA8M2_STDOUT_RTT
#include "SEGGER_RTT.h"
#endif

/* Weak default; an FSP application provides its own g_hal_init(). */
__attribute__((weak)) void g_hal_init(void) {
}

/* Weak stubs for the FreeRTOS TrustZone functions, needed when the NS side uses FreeRTOS. */
__attribute__((weak)) uint32_t vPortAllocateSecureContext(uint32_t ulSecureStackSize) {
    (void)ulSecureStackSize;
    return 0;  /* Stub - real implementation in FreeRTOS port */
}

__attribute__((weak)) void vPortFreeSecureContext(uint32_t *pulSecureContext) {
    (void)pulSecureContext;
    /* Stub - real implementation in FreeRTOS port */
}

#ifdef __ICCARM__

#include <LowLevelIOInterface.h>

/*
 * DLIB low-level IO. printf() reaches __write_buffered(), which calls __write(). This
 * definition overrides the library's, which under --semihosting needs a debugger attached.
 *
 * Only stdout and stderr are accepted. A NULL buffer is DLIB asking for a flush; RTT writes
 * straight into the control block and holds nothing back, so the flush reports success;
 * returning _LLIO_ERROR would fail an ordinary fflush().
 */
size_t __write(int handle, const unsigned char *buffer, size_t size)
{
    if (buffer == NULL) {
        return 0;
    }

    if ((handle != _LLIO_STDOUT) && (handle != _LLIO_STDERR)) {
        return _LLIO_ERROR;
    }

#ifdef RA8M2_STDOUT_RTT
    return (size_t)SEGGER_RTT_Write(0U, (const char *)buffer, (unsigned)size);
#else
    return size;  /* No backend - claim success rather than fail the caller */
#endif
}

size_t __read(int handle, unsigned char *buffer, size_t size)
{
    (void)handle;
    (void)buffer;
    (void)size;
    return _LLIO_ERROR;  /* No input device */
}

#else /* !__ICCARM__ - GCC/newlib */

#include <sys/stat.h>
#include <errno.h>

/* Newlib syscall stubs; nosys.specs' own stubs warn, and the link uses -Wl,-fatal-warnings. */

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
#ifdef RA8M2_STDOUT_RTT
    /* Routes the NS application's printf() to RTT. rtt/rtt_stdout.c also defines _write();
     * the NS build compiles it with RA8M2_STDOUT_NO_WRITE (ns/CMakeLists.txt), so this is the
     * only NS definition.
     *
     * _isatty() above returns 1, so newlib line-buffers stdout and a printf ending in \n
     * reaches RTT immediately; the application needs no fflush().
     */
    return (int)SEGGER_RTT_Write(0U, ptr, (unsigned)len);
#else
    (void)ptr;
    return len;  /* No backend - pretend we wrote everything to avoid errors */
#endif
}

#endif /* __ICCARM__ */
