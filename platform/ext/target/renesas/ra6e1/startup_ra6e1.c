/*
 * Copyright (c) 2025 Renesas Electronics Corporation. All rights reserved.
 * Copyright (c) 2009-2024 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * TF-M startup code for Renesas RA6M4 (Cortex-M33)
 * Based on ARM MPS2 AN521 startup implementation
 */

#include <stdint.h>
/* Use CMSIS Core headers directly to avoid FSP dependencies in startup */
#include "tfm_hal_device_header.h"

/* Forward declaration of SystemInit */
void SystemInit(void);
/*----------------------------------------------------------------------------
  Type Definitions
 *----------------------------------------------------------------------------*/
typedef void(*VECTOR_TABLE_Type)(void);


/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
extern uint32_t __INITIAL_SP;
extern uint32_t __STACK_LIMIT;

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
extern uint64_t __STACK_SEAL;
#endif

extern __NO_RETURN void __PROGRAM_START(void);

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler(void);
void Default_Handler(void);

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Exceptions */
void NMI_Handler(void)                      __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)                __attribute__((weak));
void MemManage_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)                 __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void)               __attribute__((weak, alias("Default_Handler")));
void SecureFault_Handler(void)              __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)                      __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)                 __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)                   __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)                  __attribute__((weak, alias("Default_Handler")));

/* External interrupts for RA6M4 - using default handler */
/* Total 480 external interrupts supported by RA6M4 */
void PORT_IRQ0_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void PORT_IRQ1_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void PORT_IRQ2_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void PORT_IRQ3_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void PORT_IRQ4_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void PORT_IRQ5_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void PORT_IRQ6_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void PORT_IRQ7_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void PORT_IRQ8_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void PORT_IRQ9_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void PORT_IRQ10_Handler(void)               __attribute__((weak, alias("Default_Handler")));
void PORT_IRQ11_Handler(void)               __attribute__((weak, alias("Default_Handler")));
void PORT_IRQ12_Handler(void)               __attribute__((weak, alias("Default_Handler")));
void PORT_IRQ13_Handler(void)               __attribute__((weak, alias("Default_Handler")));
void PORT_IRQ14_Handler(void)               __attribute__((weak, alias("Default_Handler")));
void PORT_IRQ15_Handler(void)               __attribute__((weak, alias("Default_Handler")));

/* Add more external interrupt handlers as needed */
/* For now, we'll provide a minimal set - the weak aliases ensure any undefined handlers
 * will use Default_Handler */

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

extern const VECTOR_TABLE_Type __VECTOR_TABLE[496];
       const VECTOR_TABLE_Type __VECTOR_TABLE[496] __VECTOR_TABLE_ATTRIBUTE = {
    (VECTOR_TABLE_Type)(&__INITIAL_SP),       /*     Initial Stack Pointer */
    Reset_Handler,                            /*     Reset Handler */
    NMI_Handler,                              /* -14 NMI Handler */
    HardFault_Handler,                        /* -13 Hard Fault Handler */
    MemManage_Handler,                        /* -12 MPU Fault Handler */
    BusFault_Handler,                         /* -11 Bus Fault Handler */
    UsageFault_Handler,                       /* -10 Usage Fault Handler */
    SecureFault_Handler,                      /*  -9 Secure Fault Handler */
    0,                                        /*  -8 Reserved */
    0,                                        /*  -7 Reserved */
    0,                                        /*  -6 Reserved */
    SVC_Handler,                              /*  -5 SVCall Handler */
    DebugMon_Handler,                         /*  -4 Debug Monitor Handler */
    0,                                        /*  -3 Reserved */
    PendSV_Handler,                           /*  -2 PendSV Handler */
    SysTick_Handler,                          /*  -1 SysTick Handler */

    /* External interrupts for RA6M4 (480 total) */
    PORT_IRQ0_Handler,                        /*   0 */
    PORT_IRQ1_Handler,                        /*   1 */
    PORT_IRQ2_Handler,                        /*   2 */
    PORT_IRQ3_Handler,                        /*   3 */
    PORT_IRQ4_Handler,                        /*   4 */
    PORT_IRQ5_Handler,                        /*   5 */
    PORT_IRQ6_Handler,                        /*   6 */
    PORT_IRQ7_Handler,                        /*   7 */
    PORT_IRQ8_Handler,                        /*   8 */
    PORT_IRQ9_Handler,                        /*   9 */
    PORT_IRQ10_Handler,                       /*  10 */
    PORT_IRQ11_Handler,                       /*  11 */
    PORT_IRQ12_Handler,                       /*  12 */
    PORT_IRQ13_Handler,                       /*  13 */
    PORT_IRQ14_Handler,                       /*  14 */
    PORT_IRQ15_Handler,                       /*  15 */
    /* IRQs 16-479: Initialize remaining with Default_Handler */
    [16 ... 479] = Default_Handler,
};

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler(void)
{
#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    __disable_irq();
#endif

    __set_PSP((uint32_t)(&__INITIAL_SP));

    __set_MSPLIM((uint32_t)(&__STACK_LIMIT));
    __set_PSPLIM((uint32_t)(&__STACK_LIMIT));

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    __TZ_set_STACKSEAL_S((uint32_t *)(&__STACK_SEAL));
#endif

    SystemInit();                             /* CMSIS System Initialization */
    __PROGRAM_START();                        /* Enter PreMain (C library entry point) */
}

/*----------------------------------------------------------------------------
  Hard Fault Handler
 *----------------------------------------------------------------------------*/
void HardFault_Handler(void)
{
    while(1);
}

/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void)
{
    while(1);
}
