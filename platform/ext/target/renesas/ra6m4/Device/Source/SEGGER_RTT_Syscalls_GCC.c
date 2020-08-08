/*********************************************************************
 *                    SEGGER Microcontroller GmbH                     *
 *                        The Embedded Experts                        *
 **********************************************************************
 *                                                                    *
 *            (c) 1995 - 2019 SEGGER Microcontroller GmbH             *
 *                                                                    *
 *       www.segger.com     Support: support@segger.com               *
 *                                                                    *
 **********************************************************************
 *                                                                    *
 *       SEGGER RTT * Real Time Transfer for embedded targets         *
 *                                                                    *
 **********************************************************************
 *                                                                    *
 * All rights reserved.                                               *
 *                                                                    *
 * SEGGER strongly recommends to not make any changes                 *
 * to or modify the source code of this software in order to stay     *
 * compatible with the RTT protocol and J-Link.                       *
 *                                                                    *
 * Redistribution and use in source and binary forms, with or         *
 * without modification, are permitted provided that the following    *
 * conditions are met:                                                *
 *                                                                    *
 * o Redistributions of source code must retain the above copyright   *
 *   notice, this list of conditions and the following disclaimer.    *
 *                                                                    *
 * o Redistributions in binary form must reproduce the above          *
 *   copyright notice, this list of conditions and the following      *
 *   disclaimer in the documentation and/or other materials provided  *
 *   with the distribution.                                           *
 *                                                                    *
 * o Neither the name of SEGGER Microcontroller GmbH                  *
 *   nor the names of its contributors may be used to endorse or      *
 *   promote products derived from this software without specific     *
 *   prior written permission.                                        *
 *                                                                    *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
 * DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
 * DAMAGE.                                                            *
 *                                                                    *
 **********************************************************************
 *                                                                    *
 *       RTT version: 6.44h                                           *
 *                                                                    *
 **********************************************************************
 * ---------------------------END-OF-HEADER------------------------------
 * File    : SEGGER_RTT_Syscalls_GCC.c
 * Purpose : Low-level functions for using printf() via RTT in GCC.
 *        To use RTT for printf output, include this file in your
 *        application.
 * Revision: $Rev: 9599 $
 * ----------------------------------------------------------------------
 */
#if (defined __GNUC__) && !(defined __SES_ARM) && !(defined __CROSSWORKS_ARM) && (TEST_FSP_USE_GCOV == 1)

#include <reent.h>                     // required for _write_r
#include "SEGGER_RTT.h"
#include <stdint.h>
#include <string.h>

/*********************************************************************
 *
 *       Types
 *
 **********************************************************************
 */

//
// If necessary define the _reent struct
// to match the one passed by the used standard library.
//
struct _reent;

/*********************************************************************
 *
 *       Function prototypes
 *
 **********************************************************************
 */
_ssize_t _write_r(struct _reent *, int, const void *, size_t);

// int _write_r(struct _reent *r, int file, const void *ptr, int len);

/*********************************************************************
 *
 *       Global functions
 *
 **********************************************************************
 */

/* Since we have 1 buffer and 1 index, this only works if the application is only
 * working on 1 file at a time. This is done for gcov and gcov works on 1 file at
 * a time. */
uint32_t g_buffer_index = 0;
uint8_t  g_buffer[32];

/*********************************************************************
 *
 *       _write_r()
 *
 * Function description
 *   Low-level reentrant write function.
 *   libc subroutines will use this system routine for output to all files,
 *   including stdout.
 *   Write data via RTT.
 */
_ssize_t _write_r (struct _reent * r, int file, const void * ptr, size_t len) {
    (void) file;                       /* Not used, avoid warning */
    (void) r;                          /* Not used, avoid warning */

    /* See if this write will overflow buffer. If so, then flush and start over. */
    if ((g_buffer_index + len) > sizeof(g_buffer))
    {
        SEGGER_RTT_WriteNoLock(0, &g_buffer[0], g_buffer_index);
        g_buffer_index = 0;
    }

    /* If this write is bigger or same size as our internal buffer, then go ahead and write. */
    if (len >= sizeof(g_buffer))
    {
        SEGGER_RTT_WriteNoLock(0, ptr, len);
    }
    else
    {
        /* Queue up bytes. */
        memcpy(&g_buffer[g_buffer_index], ptr, len);
        g_buffer_index += len;
    }

    return (_ssize_t) len;
}

int _open_r(struct _reent * r, const char * file, int flags, int mode);

int _open_r (struct _reent * r, const char * file, int flags, int mode)
{
    (void) r;
    (void) file;
    (void) flags;
    (void) mode;

    g_buffer_index = 0;

    SEGGER_RTT_WriteString(0, "<<<<Opening::");
    SEGGER_RTT_WriteString(0, file);
    SEGGER_RTT_WriteString(0, ">>>>");

    // SEGGER_RTT_printf(0, "<<<<Opening::%s>>>>", file);

    // SEGGER_RTT_WriteString(0, file);
    return 42;
}

int _close_r(struct _reent * r, int fildes);

int _close_r (struct _reent * r, int fildes)
{
    (void) r;

    /* See if we need to flush. */
    if (0 < g_buffer_index)
    {
        /* Write final bytes. */
        SEGGER_RTT_WriteNoLock(0, &g_buffer[0], g_buffer_index);
    }

    SEGGER_RTT_printf(0, "<<<<Closing::%d>>>>", fildes);

    return 0;
}

#endif

/****** End Of File *************************************************/
