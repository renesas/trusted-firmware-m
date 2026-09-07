/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Stub implementation of FSP BSP initialization structures
 * TF-M handles its own startup, so we provide minimal stubs here
 */

/* stdint.h first: bsp_linker_info.h declares uint32_t-typed tables and does not include
 * it itself. This file used to rely on platform_s pulling stdint in through some other
 * path; it is compiled into fsp_bsp now, so make it self-sufficient. */
#include <stdint.h>
#include "bsp_linker_info.h"

/* Empty initialization tables - TF-M handles startup */
static const bsp_init_zero_info_t zero_list[] = {
    /* Empty - no zero initialization needed */
};

static const bsp_init_copy_info_t copy_list[] = {
    /* Empty - no copy initialization needed */
};

static const bsp_mpu_nocache_info_t nocache_list[] = {
    /* Empty - no nocache regions */
};

/* Weak stub initialization data structure - can be overridden by real g_init_info */
__attribute__((weak)) const bsp_init_info_t g_init_info =
{
    .zero_count  = 0,
    .p_zero_list = zero_list,
    .copy_count  = 0,
    .p_copy_list = copy_list,
    .nocache_count  = 0,
    .p_nocache_list = nocache_list
};

#include <stdint.h>

/* g_main_stack - aliased onto the image's REAL stack, not a private array.
 *
 * FSP's system.c uses this symbol for two things during SystemInit():
 *
 *   system.c:221  *(uint32_t *)&g_main_stack[BSP_CFG_STACK_MAIN_BYTES] = seal
 *   system.c:349  __set_MSPLIM((uint32_t)&g_main_stack[0])
 *
 * both of which describe whatever stack the CPU is actually running on. TF-M owns that
 * stack - the linker script places it and the vector table's initial SP points at its
 * limit - so a separate array here is not merely redundant, it is wrong in both
 * directions. It used to be a 0x400 array wherever the linker happened to drop it:
 *
 *   tfm_s: array at 0x2000C578, real stack 0x20000E00..0x200015F8. MSPLIM landed 44 KB
 *          ABOVE the live SP, so the first push inside SystemRuntimeInit() raised a
 *          stack-overflow UsageFault -> HardFault. The seal write also went 0xC00 past
 *          the end of the array, silently corrupting .bss just before that.
 *   bl2:   array at 0x200032E0, real stack 0x200036E0..0x20004EE0. MSPLIM landed just
 *          BELOW the stack, so it happened not to fault - luck, not correctness. BL2 is
 *          a flat build so BSP_TZ_SECURE_BUILD is 0 and it skips the seal write.
 *
 * Aliasing the symbol to Image$$ARM_LIB_STACK$$ZI$$Base makes both operations correct:
 * MSPLIM protects the stack it is supposed to, and the seal lands exactly on the eight
 * bytes the generated script already reserves at __StackSeal. That needs the stack size
 * FSP believes in to match the one TF-M lays out, which ra6e1_layout_checks.c asserts
 * (S_MSP_STACK_SIZE == BSP_CFG_STACK_MAIN_BYTES + STACKSEAL_SIZE).
 *
 * The alias is made at LINK time, in the platform CMakeLists:
 *
 *     -Wl,--defsym=g_main_stack=Image$$ARM_LIB_STACK$$ZI$$Base
 *
 * and deliberately not here. __attribute__((alias)) needs the target defined in the same
 * translation unit, and an assembler .set against a linker-defined symbol leaves
 * g_main_stack undefined at link time. So there is no definition in this file - the
 * symbol arrives from the link line for both the secure image and BL2.
 *
 * The NS image never sees this file; it keeps FSP's own startup.c and bsp_linker.c,
 * which define g_main_stack the ordinary way.
 */

/* Forward declarations for weak stubs */
void g_hal_init(void);
uint32_t vPortAllocateSecureContext(uint32_t ulSecureStackSize);
void vPortFreeSecureContext(uint32_t *pulSecureContext);

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
