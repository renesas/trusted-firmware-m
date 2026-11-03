/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Stub FSP BSP initialisation data. TF-M owns startup, so these are minimal.
 */

/* stdint.h first: bsp_linker_info.h declares uint32_t-typed tables without including it,
 * and this file is compiled into fsp_bsp, so it must be self-sufficient. */
#include <stdint.h>
#include "bsp_linker_info.h"

/* Empty tables: TF-M's startup does the C-runtime initialisation. */
static const bsp_init_zero_info_t zero_list[] = {
};

static const bsp_init_copy_info_t copy_list[] = {
};

static const bsp_mpu_nocache_info_t nocache_list[] = {
};

/* Weak, so a real g_init_info overrides it. */
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

/* g_main_stack is aliased onto the image's real stack, not a private array.
 *
 * FSP's system.c uses this symbol twice during SystemInit():
 *
 *   system.c:221  *(uint32_t *)&g_main_stack[BSP_CFG_STACK_MAIN_BYTES] = seal
 *   system.c:349  __set_MSPLIM((uint32_t)&g_main_stack[0])
 *
 * Both describe the stack the CPU runs on, which TF-M owns: the linker script places it and
 * the vector table's initial SP points at its limit. The 0x400 array this file once defined
 * was wrong in both directions:
 *
 *   tfm_s: array at 0x2000C578, real stack 0x20000E00..0x200015F8. MSPLIM landed 44 KB above
 *          the live SP, so the first push in SystemRuntimeInit() raised a stack-overflow
 *          UsageFault -> HardFault. The seal write went 0xC00 past the array, into .bss.
 *   bl2:   array at 0x200032E0, real stack 0x200036E0..0x20004EE0. MSPLIM landed just below
 *          the stack, so it did not fault, by luck. BL2 is a flat build, so
 *          BSP_TZ_SECURE_BUILD is 0 and it skips the seal write.
 *
 * Aliased to Image$$ARM_LIB_STACK$$ZI$$Base, MSPLIM protects the real stack and the seal lands
 * on the eight bytes the generated script reserves at __StackSeal. That needs FSP's stack size
 * to match TF-M's, which ra6m5_layout_checks.c asserts
 * (S_MSP_STACK_SIZE == BSP_CFG_STACK_MAIN_BYTES + STACKSEAL_SIZE).
 *
 * The alias is made at link time, in the platform CMakeLists:
 *
 *     -Wl,--defsym=g_main_stack=Image$$ARM_LIB_STACK$$ZI$$Base
 *
 * and deliberately not here: __attribute__((alias)) needs the target in the same translation
 * unit, and an assembler .set against a linker-defined symbol leaves g_main_stack undefined
 * at link time. The symbol comes from the link line for both the secure image and BL2.
 *
 * The NS image does not use this file; it keeps FSP's startup.c and bsp_linker.c, which
 * define g_main_stack the ordinary way.
 */

void g_hal_init(void);
uint32_t vPortAllocateSecureContext(uint32_t ulSecureStackSize);
void vPortFreeSecureContext(uint32_t *pulSecureContext);

/* Weak: FSP applications define their own g_hal_init(). */
__attribute__((weak)) void g_hal_init(void) {
}

/* Weak FreeRTOS TrustZone stubs, needed when the NS side runs FreeRTOS. */
__attribute__((weak)) uint32_t vPortAllocateSecureContext(uint32_t ulSecureStackSize) {
    (void)ulSecureStackSize;
    return 0;  /* Stub - real implementation in FreeRTOS port */
}

__attribute__((weak)) void vPortFreeSecureContext(uint32_t *pulSecureContext) {
    (void)pulSecureContext;
    /* Stub - real implementation in FreeRTOS port */
}
