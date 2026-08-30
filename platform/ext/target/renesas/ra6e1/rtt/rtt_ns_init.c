/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Non-secure RTT bring-up signal.
 *
 * Until this file existed the NS image had no output of any kind: rtt/SEGGER_RTT.c was
 * compiled into tfm_ns but nothing referenced it, so --gc-sections dropped the control
 * block and a successful S->NS jump was indistinguishable from a hang. That is the same
 * class of defect that hid the secure image's output until 2026-08-29 - there the missing
 * caller was stdio_init(), here it was anything at all.
 *
 * WHY A LINKER WRAP AND NOT A CONSTRUCTOR
 *
 * The obvious approach - __attribute__((constructor)) - silently does nothing in this
 * image. The NS side links FSP's own startup.c and the solution's script/fsp.ld, and that
 * combination has no .init_array at all: readelf finds no such section, and the image
 * contains no call to __libc_init_array(). FSP's Reset_Handler does its own .data/.bss
 * setup and reaches main() without newlib's crt0. A constructor therefore links (the
 * function is emitted) but is never called, which is worse than not building - it looks
 * correct and produces silence.
 *
 * The NS entry path is main() -> hal_entry(), both in generated/user project files this
 * port must not edit. main() reaches hal_entry() through an ordinary bl against a global
 * symbol, so -Wl,--wrap=hal_entry redirects it here with no source change: the linker
 * rewrites main.o's reference to __wrap_hal_entry, and leaves the original reachable as
 * __real_hal_entry. Guaranteed to run, and it cannot be dropped by --gc-sections because
 * main() references it.
 *
 * The wrap is added in the platform CMakeLists next to the source, and both halves must
 * move together - defining __wrap_hal_entry() without the flag is dead code, and passing
 * the flag without a definition is an undefined-symbol link error. Neither fails quietly.
 *
 * THE NS CONTROL BLOCK IS SEPARATE FROM THE SECURE ONE
 *
 * Two live "SEGGER RTT" ID strings exist in RAM once NS starts, and J-Link RTT Viewer
 * finds the block by searching for that string - so a search range latches onto whichever
 * it hits first. To watch the NS side specifically, give the viewer this image's address:
 *
 *   arm-none-eabi-nm --defined-only bin/tfm_ns.axf | grep _SEGGER_RTT
 *
 * It lives in .bss and moves whenever NS buffer sizing changes, so re-read it per build
 * rather than noting it down.
 */

#include "SEGGER_RTT.h"

#ifdef RA6E1_NS_SERVICE_TESTS
#include "tfm_service_tests.h"
#endif

/* Provided by the NS application; renamed by -Wl,--wrap=hal_entry. */
extern void __real_hal_entry(void);

void __wrap_hal_entry(void);

void __wrap_hal_entry(void)
{
    /* SEGGER_RTT_Write() self-initialises through its INIT() macro, but call this
     * explicitly so the control block is populated even if the write below is ever
     * removed - RTT Viewer cannot find a zeroed block. */
    SEGGER_RTT_Init();
    SEGGER_RTT_WriteString(0U, "tfm_ns: non-secure image running\r\n");

#ifdef RA6E1_NS_SERVICE_TESTS
    /* Before __real_hal_entry(), not after: the FSP application entry point is the
     * project's own code and may never return. Running the smoke test first also keeps
     * its output from interleaving with anything the application prints. */
    tfm_service_tests_run();
#endif

    __real_hal_entry();
}
