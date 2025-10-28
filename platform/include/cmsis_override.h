/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __CMSIS_OVERRIDE_H__
#define __CMSIS_OVERRIDE_H__

#include <stdint.h>

/* Try to detect CMSIS version - CMSIS 6 defines this in cmsis_version.h */
#if __has_include("cmsis_version.h")
#include "cmsis_version.h"
#endif

#if defined(__GNUC__)

#define __INITIAL_SP    Image$$ARM_LIB_STACK$$ZI$$Limit
#define __STACK_LIMIT   Image$$ARM_LIB_STACK$$ZI$$Base

#elif defined(__ICCARM__)

#define __INITIAL_SP    ARM_LIB_STACK$$Limit
#define __STACK_LIMIT   ARM_LIB_STACK$$Base

#endif

/* ARM TrustZone CMSE defines that may not be in all CMSIS headers */
#ifndef __TZ_STACK_SEAL_VALUE
#define __TZ_STACK_SEAL_VALUE  0xFEF5EDA5FEF5EDA5ULL
#endif

#ifndef CMSE_NONSECURE
#define CMSE_NONSECURE 1
#endif

/*
 * CMSIS 6 provides these intrinsics natively, so only define them for CMSIS 5
 * Check for CMSIS 6 by looking for __CM_CMSIS_VERSION_MAIN >= 6
 */
#if !defined(__CM_CMSIS_VERSION_MAIN) || (__CM_CMSIS_VERSION_MAIN < 6)

/* CMSE intrinsics that may not be available in CMSIS 5 */
#ifndef __TZ_set_MSP_NS
__attribute__((always_inline)) static inline void __TZ_set_MSP_NS(uint32_t topOfMainStack)
{
    __asm volatile ("MSR msp_ns, %0" : : "r" (topOfMainStack) : );
}
#endif

#ifndef __TZ_get_CONTROL_NS
__attribute__((always_inline)) static inline uint32_t __TZ_get_CONTROL_NS(void)
{
    uint32_t result;
    __asm volatile ("MRS %0, control_ns" : "=r" (result) );
    return(result);
}
#endif

/* Non-secure SCB pointer */
#ifndef SCB_NS
#define SCB_NS ((SCB_Type *) ((SCB_BASE) + 0x10000000UL))
#endif

/* Naked veneer attribute for TrustZone */
#ifndef __tz_naked_veneer
#define __tz_naked_veneer __attribute__((cmse_nonsecure_entry, naked))
#endif

#endif /* !CMSIS 6 */

#endif /* __CMSIS_OVERRIDE_H__ */
