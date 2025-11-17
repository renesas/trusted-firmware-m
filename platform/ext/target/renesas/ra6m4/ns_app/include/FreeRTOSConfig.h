/*
 * Copyright (c) 2024, Renesas Electronics Corporation. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * FreeRTOS Configuration for RA6M4 Non-Secure Application
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

/* Cortex-M33 with TrustZone non-secure side */
#define configENABLE_TRUSTZONE                          1
#define configRUN_FREERTOS_SECURE_ONLY                  0

/* Scheduler configuration */
#define configUSE_PREEMPTION                            1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION         0
#define configUSE_TICKLESS_IDLE                         0
#define configCPU_CLOCK_HZ                              ( 200000000UL )
#define configTICK_RATE_HZ                              ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES                            ( 5 )
#define configMINIMAL_STACK_SIZE                        ( ( uint16_t ) 128 )
#define configMAX_TASK_NAME_LEN                         ( 16 )
#define configUSE_16_BIT_TICKS                          0
#define configIDLE_SHOULD_YIELD                         1
#define configUSE_TASK_NOTIFICATIONS                    1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES           3
#define configUSE_MUTEXES                               1
#define configUSE_RECURSIVE_MUTEXES                     1
#define configUSE_COUNTING_SEMAPHORES                   1
#define configQUEUE_REGISTRY_SIZE                       8
#define configUSE_QUEUE_SETS                            0
#define configUSE_TIME_SLICING                          1
#define configUSE_NEWLIB_REENTRANT                      0
#define configENABLE_BACKWARD_COMPATIBILITY             0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS         5
#define configSTACK_DEPTH_TYPE                          uint32_t
#define configMESSAGE_BUFFER_LENGTH_TYPE                size_t

/* Memory allocation */
#define configSUPPORT_STATIC_ALLOCATION                 1
#define configSUPPORT_DYNAMIC_ALLOCATION                1
#define configTOTAL_HEAP_SIZE                           ( ( size_t ) ( 64 * 1024 ) )
#define configAPPLICATION_ALLOCATED_HEAP                0

/* Hook function configuration */
#define configUSE_IDLE_HOOK                             1
#define configUSE_TICK_HOOK                             1
#define configCHECK_FOR_STACK_OVERFLOW                  2
#define configUSE_MALLOC_FAILED_HOOK                    1
#define configUSE_DAEMON_TASK_STARTUP_HOOK              0

/* Run time and task stats gathering */
#define configGENERATE_RUN_TIME_STATS                   0
#define configUSE_TRACE_FACILITY                        0
#define configUSE_STATS_FORMATTING_FUNCTIONS            0

/* Co-routine configuration */
#define configUSE_CO_ROUTINES                           0
#define configMAX_CO_ROUTINE_PRIORITIES                 ( 2 )

/* Software timer configuration */
#define configUSE_TIMERS                                1
#define configTIMER_TASK_PRIORITY                       ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                        10
#define configTIMER_TASK_STACK_DEPTH                    ( configMINIMAL_STACK_SIZE * 2 )

/* Interrupt nesting behaviour configuration */
#define configKERNEL_INTERRUPT_PRIORITY                 ( 0xFF )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY            ( 0x20 )
#define configMAX_API_CALL_INTERRUPT_PRIORITY           ( configMAX_SYSCALL_INTERRUPT_PRIORITY )

/* Optional functions */
#define INCLUDE_vTaskPrioritySet                        1
#define INCLUDE_uxTaskPriorityGet                       1
#define INCLUDE_vTaskDelete                             1
#define INCLUDE_vTaskSuspend                            1
#define INCLUDE_xResumeFromISR                          1
#define INCLUDE_vTaskDelayUntil                         1
#define INCLUDE_vTaskDelay                              1
#define INCLUDE_xTaskGetSchedulerState                  1
#define INCLUDE_xTaskGetCurrentTaskHandle               1
#define INCLUDE_uxTaskGetStackHighWaterMark             1
#define INCLUDE_uxTaskGetStackHighWaterMark2            0
#define INCLUDE_xTaskGetIdleTaskHandle                  0
#define INCLUDE_eTaskGetState                           0
#define INCLUDE_xEventGroupSetBitFromISR                1
#define INCLUDE_xTimerPendFunctionCall                  1
#define INCLUDE_xTaskAbortDelay                         0
#define INCLUDE_xTaskGetHandle                          0
#define INCLUDE_xTaskResumeFromISR                      1

/* Cortex-M specific definitions */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS                             __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS                             3
#endif

/* Definitions that map the FreeRTOS port interrupt handlers to CMSIS handlers */
#define vPortSVCHandler                                 SVC_Handler
#define xPortPendSVHandler                              PendSV_Handler
#define xPortSysTickHandler                             SysTick_Handler

/* Assert configuration */
#define configASSERT( x )                               if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

#endif /* FREERTOS_CONFIG_H */
