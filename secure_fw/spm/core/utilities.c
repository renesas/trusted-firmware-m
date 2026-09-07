/*
 * Copyright (c) 2018-2021, Arm Limited. All rights reserved.
 * Copyright (c) 2024 Cypress Semiconductor Corporation (an Infineon
 * company) or an affiliate of Cypress Semiconductor Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <inttypes.h>
#include "config_spm.h"
#include "fih.h"
#include "utilities.h"
#include "tfm_hal_platform.h"

#ifdef CONFIG_TFM_BACKTRACE_ON_CORE_PANIC
#include "tfm_log.h"
#include "backtrace.h"
#endif

/* SPM control-flow trace, enabled by TFM_SPM_DEBUG_TRACE (config_base.cmake, default OFF).
 *
 * tfm_core_panic() is called from ~140 sites and all of them land in the same
 * tfm_hal_system_halt() spin, which makes "it halted" useless as a diagnostic. Log the
 * caller's return address so the halt names its own site; resolve it with
 *
 *   arm-none-eabi-addr2line -f -e build_ra6e1/bin/tfm_s.axf <PC>
 *
 * SPMLOG_ is deliberate: it goes tfm_hal_output_spm_log() -> stdio_output_string()
 * straight to the backend, with no printf, no SVC and no memory check, so it works from
 * Handler mode and cannot itself re-enter a panic path the way a partition LOG_INFFMT can.
 *
 * __builtin_return_address(0) is valid here because tfm_core_panic() is an ordinary
 * function, not naked. The value is the LR at entry, so it points at the instruction
 * AFTER the call - addr2line lands on the right line anyway, but subtract 4 if a site
 * sits at the very start of a source line.
 */
#ifdef TFM_SPM_DEBUG_TRACE
#include "tfm_spm_log.h"
#endif

void tfm_core_panic(void)
{
#ifdef TFM_SPM_DEBUG_TRACE
    SPMLOG_ERRMSGVAL("\r\n[PANIC] tfm_core_panic() called from LR=",
                     (uint32_t)__builtin_return_address(0));
#endif

    (void)fih_delay();

#ifdef CONFIG_TFM_BACKTRACE_ON_CORE_PANIC
    tfm_dump_backtrace(__func__, tfm_log);
#endif

/* Suppress Pe111 (statement is unreachable) for IAR as redundant code is needed for FIH */
#if defined(__ICCARM__)
#pragma diag_suppress = Pe111
#endif
#ifdef CONFIG_TFM_HALT_ON_CORE_PANIC

    /*
     * Halt instead of reboot to retain the backtrace that triggered
     * the fault and thereby make it easier to debug.
     */
    tfm_hal_system_halt();

#ifdef TFM_FIH_PROFILE_ON
    (void)fih_delay();

    tfm_hal_system_halt();
#endif

#else /* CONFIG_TFM_HALT_ON_CORE_PANIC */
    /*
     * FixMe: In the first stage, the SPM will restart the entire system when a
     * programmer error is detected in either the SPE or NSPE.
     * In the next stage, the specified error codes are also sent to any NSPE
     * management firmware. The NSPE management firmware can then decide to pass
     * those error codes back to the calling task or to use its own
     * functionality for terminating an execution context.
     */
    tfm_hal_system_reset();

#ifdef TFM_FIH_PROFILE_ON
    (void)fih_delay();

    tfm_hal_system_reset();
#endif

#endif /* CONFIG_TFM_HALT_ON_CORE_PANIC */
#if defined(__ICCARM__)
#pragma diag_default = Pe111
#endif
}
