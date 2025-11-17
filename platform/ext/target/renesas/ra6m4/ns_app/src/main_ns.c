/*
 * Copyright (c) 2024, Renesas Electronics Corporation. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TF-M Non-Secure Application Main
 * FreeRTOS-based application with TF-M service testing
 */

#include <stdio.h>
#include "tfm_test_thread.h"
#include "FreeRTOS.h"
#include "task.h"
#include "tfm_ns_interface.h"

/* Task priorities */
#define TFM_TEST_TASK_PRIORITY      (tskIDLE_PRIORITY + 1)
#define TFM_TEST_TASK_STACK_SIZE    (4096)

/* Task handle */
static TaskHandle_t tfm_test_task_handle;

/**
 * @brief Non-Secure Application Entry Point
 *
 * Initializes FreeRTOS and creates the TF-M test task
 */
int main(void)
{
    BaseType_t xReturn;

    /* Create TF-M test task */
    xReturn = xTaskCreate(
        tfm_test_thread_entry,           /* Task function */
        "TFM_Test",                      /* Task name */
        TFM_TEST_TASK_STACK_SIZE,        /* Stack size (words) */
        NULL,                             /* Task parameters */
        TFM_TEST_TASK_PRIORITY,          /* Task priority */
        &tfm_test_task_handle            /* Task handle */
    );

    if (xReturn != pdPASS)
    {
        /* Task creation failed */
        while (1);
    }

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    /* Should never reach here */
    while (1);
}

/**
 * @brief FreeRTOS stack overflow hook
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;

    /* Stack overflow detected */
    while (1);
}

/**
 * @brief FreeRTOS malloc failed hook
 */
void vApplicationMallocFailedHook(void)
{
    /* Malloc failed */
    while (1);
}

/**
 * @brief FreeRTOS idle hook
 */
void vApplicationIdleHook(void)
{
    /* Idle processing */
}

/**
 * @brief FreeRTOS tick hook
 */
void vApplicationTickHook(void)
{
    /* Tick processing */
}
