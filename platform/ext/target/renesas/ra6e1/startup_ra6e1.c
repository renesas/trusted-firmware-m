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

/*
 * No named external-interrupt handlers.
 *
 * On RA the ICU slots are EVENT-LINKED: which peripheral event reaches NVIC slot N is
 * whatever the RASC/e2 configuration maps there, not a fixed function of N. So a static
 * table naming slot 0 "PORT_IRQ0" only holds for the one project it was written against
 * - it came from ra6m4 and means nothing here. The secure project currently links no
 * events at all (ra_gen/vector_data.h: VECTOR_DATA_IRQ_COUNT 0).
 *
 * Every ICU slot therefore points at Default_Handler. To service an interrupt, configure
 * the event in e2 and define the handler that ra_gen/vector_data.c names for it; the
 * weak Default_Handler binding is overridden at link time.
 */

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

/*
 * RA6E1: 112 entries = 16 Cortex exceptions + 96 ICU slots.
 *
 * Source of truth is the generated BSP config, ra_cfg/fsp_cfg/bsp/bsp_mcu_family_cfg.h:
 *     BSP_CORTEX_VECTOR_TABLE_ENTRIES  (16U)
 *     BSP_VECTOR_TABLE_MAX_ENTRIES    (112U)
 * Not included directly - this file deliberately keeps to CMSIS headers - so re-check
 * these two macros if the device ever changes.
 *
 * 112 * 4 = 0x1C0, which must fit S_CODE_VECTOR_TABLE_SIZE (0x200) in region_defs.h.
 * This was 496 (RA6M4's table, 0x7C0) and overflowed .TFM_VECTORS into .ER_UNPRIV_CODE.
 */
#define RA6E1_VECTOR_TABLE_ENTRIES  (112)

extern const VECTOR_TABLE_Type __VECTOR_TABLE[RA6E1_VECTOR_TABLE_ENTRIES];
       const VECTOR_TABLE_Type __VECTOR_TABLE[RA6E1_VECTOR_TABLE_ENTRIES] __VECTOR_TABLE_ATTRIBUTE = {
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

    /* ICU slots 0..95 - event-linked, see the note above the declarations */
    [16 ... (RA6E1_VECTOR_TABLE_ENTRIES - 1)] = Default_Handler,
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
