/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * This file is derivative of:
 *  - CMSIS V6 startup_ARMCM85.c V1.0.0 (Apache-2.0)
 *    Git SHA: 30a859f44ef8ab4dc8f84b03ed586fd16ccf9d74
 *  - Renesas FSP v6.4.0 CMSIS device startup (BSD-3-Clause)
 *    ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.c
 *
 * The C runtime is initialized through the CMSIS C environment start
 * (__PROGRAM_START), using the copy/zero tables from the TF-M linker scripts.
 * FSP BSP specifics: stack/heap are C objects (g_main_stack / g_heap), the
 * vector table lives in ".fixed_vectors", stack setup and sealing are done in
 * SystemInit(), and NMI is handled by the BSP (bsp_group_irq.c).
 */

#include "tfm_hal_device_header.h"

/*----------------------------------------------------------------------------
 * External References
 *----------------------------------------------------------------------------
 */
extern uint32_t __INITIAL_SP;
#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U) && defined(CONFIG_TFM_USE_TRUSTZONE)
extern uint64_t __STACK_SEAL;
#endif
extern __NO_RETURN void __PROGRAM_START(void);

/* NMI is handled by the FSP BSP (bsp_group_irq.c). */
extern void NMI_Handler(void);

/*----------------------------------------------------------------------------
 * Internal References
 *----------------------------------------------------------------------------
 */
__NO_RETURN void Reset_Handler(void);
            void Default_Handler(void);

/*----------------------------------------------------------------------------
 * Exception / Interrupt Handler
 *----------------------------------------------------------------------------
 */
#define DEFAULT_IRQ_HANDLER(handler_name)  \
void __WEAK handler_name(void); \
void handler_name(void) \
{ \
    Default_Handler(); \
}

/* Exceptions */
DEFAULT_IRQ_HANDLER(HardFault_Handler)
DEFAULT_IRQ_HANDLER(MemManage_Handler)
DEFAULT_IRQ_HANDLER(BusFault_Handler)
DEFAULT_IRQ_HANDLER(UsageFault_Handler)
DEFAULT_IRQ_HANDLER(SecureFault_Handler)
DEFAULT_IRQ_HANDLER(SVC_Handler)
DEFAULT_IRQ_HANDLER(DebugMon_Handler)
DEFAULT_IRQ_HANDLER(PendSV_Handler)
DEFAULT_IRQ_HANDLER(SysTick_Handler)

/*----------------------------------------------------------------------------
 * Exception / Interrupt Vector table
 *----------------------------------------------------------------------------
 */

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

/**
 * \brief Exception / Interrupt Handler Function Prototype
 */
typedef void (*VECTOR_TABLE_Type)(void);

extern const VECTOR_TABLE_Type __VECTOR_TABLE[BSP_CORTEX_VECTOR_TABLE_ENTRIES];
BSP_DONT_REMOVE const VECTOR_TABLE_Type __VECTOR_TABLE[BSP_CORTEX_VECTOR_TABLE_ENTRIES] BSP_PLACE_IN_SECTION(
    BSP_SECTION_FIXED_VECTORS) = {
  (VECTOR_TABLE_Type)(&__INITIAL_SP), /* Initial Stack Pointer */
  Reset_Handler,                     /*      Reset Handler */
  NMI_Handler,                       /* -14: NMI Handler */
  HardFault_Handler,                 /* -13: Hard Fault Handler */
  MemManage_Handler,                 /* -12: MPU Fault Handler */
  BusFault_Handler,                  /* -11: Bus Fault Handler */
  UsageFault_Handler,                /* -10: Usage Fault Handler */
  SecureFault_Handler,               /*  -9: Secure Fault Handler */
  0,                                 /*      Reserved */
  0,                                 /*      Reserved */
  0,                                 /*      Reserved */
  SVC_Handler,                       /*  -5: SVCall Handler */
  DebugMon_Handler,                  /*  -4: Debug Monitor Handler */
  0,                                 /*      Reserved */
  PendSV_Handler,                    /*  -2: PendSV Handler */
  SysTick_Handler,                   /*  -1: SysTick Handler */
};

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

/*----------------------------------------------------------------------------
 * Reset Handler called on controller reset
 *----------------------------------------------------------------------------
 */
void Reset_Handler(void)
{
    /* Stack pointers and stack limits are configured by the FSP BSP in SystemInit */
#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
#ifdef CONFIG_TFM_USE_TRUSTZONE
    __TZ_set_STACKSEAL_S((uint32_t *)(&__STACK_SEAL));
#else
    /* BL2: a flat project running in the secure state, built with the CMSE
     * flag only for this file (bl2.cmake), as CMSIS declares the SAU only for
     * CMSE builds.
     * With the SAU disabled and ALLNS clear (reset state) every address is
     * secure, so an access through a non-secure alias (e.g. the flash driver
     * on a non-secure image slot) would be a secure transaction to a
     * non-secure region. Setting ALLNS leaves the attribution to the IDAU
     * (address bit 28 selects the alias), as the FSP BSP does without SAU
     * regions. Done before SystemInit enables the caches.
     */
    SAU->CTRL = SAU_CTRL_ALLNS_Msk;
    __DSB();
    __ISB();
#endif
#endif

    SystemInit();                             /* CMSIS System Initialization */
    __PROGRAM_START();                        /* Enter PreMain (C library entry point) */
}

/*----------------------------------------------------------------------------
 * Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------
 */
void Default_Handler(void)
{
    /* An error has occurred. The user will need to investigate the cause. Common problems are stack corruption
     * or use of an invalid pointer. Use the Fault Status window in e2 studio or manually check the fault status
     * registers for more information.
     */
    BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0);

    while (1) {
        /* Infinite Loop. */
    }
}
