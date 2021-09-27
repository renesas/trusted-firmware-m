/*
 * Copyright (c) 2009-2020 Arm Limited. All rights reserved.
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
 * This file is derivative of CMSIS V5.6.0 startup_ARMv81MML.c
 * Git SHA: b5f0603d6a584d1724d952fd8b0737458b90d62b
 */

#include "cmsis.h"
#include "region.h"

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void( *pFunc )( void );

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
#define __MSP_INITIAL_SP              REGION_NAME(Image$$, ARM_LIB_STACK_MSP, $$ZI$$Limit)
#define __MSP_STACK_LIMIT             REGION_NAME(Image$$, ARM_LIB_STACK_MSP, $$ZI$$Base)

extern uint32_t __MSP_INITIAL_SP;
extern uint32_t __MSP_STACK_LIMIT;
extern uint32_t __INITIAL_SP;
extern uint32_t __STACK_LIMIT;

extern void __PROGRAM_START(void) __NO_RETURN;

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
void Reset_Handler  (void) __NO_RETURN;

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
#define DEFAULT_IRQ_HANDLER(handler_name)  \
void handler_name(void); \
__WEAK void handler_name(void) { \
    while(1); \
}

/* Exceptions */
DEFAULT_IRQ_HANDLER(NMI_Handler)
DEFAULT_IRQ_HANDLER(HardFault_Handler)
DEFAULT_IRQ_HANDLER(MemManage_Handler)
DEFAULT_IRQ_HANDLER(BusFault_Handler)
DEFAULT_IRQ_HANDLER(UsageFault_Handler)
DEFAULT_IRQ_HANDLER(SecureFault_Handler)
DEFAULT_IRQ_HANDLER(SVC_Handler)
DEFAULT_IRQ_HANDLER(DebugMon_Handler)
DEFAULT_IRQ_HANDLER(PendSV_Handler)
DEFAULT_IRQ_HANDLER(SysTick_Handler)

DEFAULT_IRQ_HANDLER(WWDG_IRQHandler)


/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

extern const pFunc __VECTOR_TABLE[];
       const pFunc __VECTOR_TABLE[] __VECTOR_TABLE_ATTRIBUTE = {
  (pFunc)(&__MSP_INITIAL_SP),           /*      Initial Stack Pointer */
  Reset_Handler,                    /*      Reset Handler */
  NMI_Handler,                      /* -14: NMI Handler */
  HardFault_Handler,                /* -13: Hard Fault Handler */
  MemManage_Handler,                /* -12: MPU Fault Handler */
  BusFault_Handler,                 /* -11: Bus Fault Handler */
  UsageFault_Handler,               /* -10: Usage Fault Handler */
  SecureFault_Handler,              /*  -9: Secure Fault Handler */
  0,                                /*      Reserved */
  0,                                /*      Reserved */
  0,                                /*      Reserved */
  SVC_Handler,                      /*  -5: SVCall Handler */
  DebugMon_Handler,                 /*  -4: Debug Monitor Handler */
  0,                                /*      Reserved */
  PendSV_Handler,                   /*  -2: PendSV Handler */
  SysTick_Handler,                  /*  -1: SysTick Handler */
//  WWDG_IRQHandler,                  /*   0: Window WatchDog */
};


/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif


#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
void Reset_Handler(void)
{
  __disable_irq();
  __set_MSPLIM((uint32_t)(&__MSP_STACK_LIMIT));
#if defined ( __GNUC__ )
  __set_MSP((uint32_t)(&__MSP_INITIAL_SP));
#endif
  SystemInit();   /* CMSIS System Initialization */
  __set_PSP((uint32_t)(&__INITIAL_SP));
  __set_PSPLIM((uint32_t)(&__STACK_LIMIT));
  __ASM volatile("MRS     R0, control\n"    /* Get control value */
                 "ORR     R0, R0, #2\n"     /* Select switch to PSP */
                 "MSR     control, R0\n"    /* Load control register */
                 :
                 :
                 : "r0");
  __PROGRAM_START();                        /* Enter PreMain (C library entry point) */
}
