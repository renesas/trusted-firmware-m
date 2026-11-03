/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Non-secure RTT bring-up signal.
 *
 * Without this file the NS image has no output: rtt/SEGGER_RTT.c is compiled into tfm_ns
 * but nothing references it, so --gc-sections drops the control block, and a successful
 * S->NS jump looks like a hang.
 *
 * Why a linker wrap and not a constructor
 *
 * __attribute__((constructor)) links but never runs in this image. The NS side links FSP's
 * own startup.c and the solution's script/fsp.ld, which have no .init_array section, and
 * the image contains no call to __libc_init_array(). FSP's Reset_Handler sets up .data and
 * .bss itself and reaches main() without newlib's crt0. A constructor therefore looks
 * correct and produces silence.
 *
 * The NS entry path is main() -> hal_entry(), both in generated or user project files that
 * this port must not edit. main() reaches hal_entry() through an ordinary bl to a global
 * symbol, so -Wl,--wrap=hal_entry redirects it here with no source change: the linker
 * points main.o's reference at __wrap_hal_entry and keeps the original reachable as
 * __real_hal_entry. main() references the wrapper, so --gc-sections cannot drop it.
 *
 * The CMakeLists that adds this source also adds the wrap flag, and the two must move
 * together. The definition without the flag is dead code; the flag without the definition
 * is an undefined-symbol link error.
 *
 * The NS control block is separate from the secure one
 *
 * Once NS starts, RAM holds two live "SEGGER RTT" ID strings. J-Link RTT Viewer finds the
 * block by searching for that string, so a search range finds whichever comes first. To
 * watch the NS side, give the viewer this image's address:
 *
 *   arm-none-eabi-nm --defined-only bin/tfm_ns.axf | grep _SEGGER_RTT
 *
 * The block is in .bss and moves when NS buffer sizing changes, so read the address again
 * for each build.
 */

#include "SEGGER_RTT.h"

#ifdef RA6M5_NS_SERVICE_TESTS
#include "tfm_service_tests.h"
#endif

/* Provided by the NS application; renamed by -Wl,--wrap=hal_entry. */
extern void __real_hal_entry(void);

void __wrap_hal_entry(void);

void __wrap_hal_entry(void)
{
    /* SEGGER_RTT_Write() self-initialises through its INIT() macro. The explicit call keeps
     * the control block populated if the write below is removed; RTT Viewer cannot find a
     * zeroed block. */
    SEGGER_RTT_Init();
    SEGGER_RTT_WriteString(0U, "tfm_ns: non-secure image running\r\n");

#ifdef RA6M5_NS_SERVICE_TESTS
    /* Before __real_hal_entry(), not after: the FSP application entry point is project
     * code and may never return. Running the smoke test first also keeps its output from
     * interleaving with the application's. */
    tfm_service_tests_run();
#endif

    __real_hal_entry();
}
