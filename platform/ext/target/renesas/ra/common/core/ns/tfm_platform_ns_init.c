/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "Driver_USART.h"
#include "bsp_api.h"
#include "tfm_ioctl_api.h"
#include "uart_stdout.h"

/* Interrupt event table of this NS image, defined in vector_data.c. */
extern const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES];

#ifdef RA_NS_INTERFACE_TZ
/*
 * The Secure image routes every event to CPU1 at boot. Ask it to route this image's events back to CPU0.
 */
static enum tfm_platform_err_t ra_ns_route_events_cpu0(uint32_t ielsr_count)
{
    uint32_t result = 0U;
    enum tfm_platform_err_t err = TFM_PLATFORM_ERR_SUCCESS;

    for (uint32_t i = 0U; i < ielsr_count; i++) {
        if (g_interrupt_event_link_select[i] != 0U) {
            err = tfm_platform_irq_cpu_sel(TFM_IRQ_CPU_SEL_CPU0, (uint32_t) g_interrupt_event_link_select[i], &result);
            if (err != TFM_PLATFORM_ERR_SUCCESS) {
                return err;
            }
        }
    }

    return err;
}
#endif

/*
 * Initialize the TF-M non-secure platform.
 *
 * The CM85 NS image routes its events back to CPU0 through the Secure Service.
 * The CM33 NS image is already on CPU1 by default, so it does not route anything.
 */
int32_t tfm_ns_platform_init(void)
{
    stdio_init();

#if BSP_FEATURE_ICU_HAS_IELSR
    /* Calculate the number of IELSR registers that need to be initialized. */
    uint32_t ielsr_count = BSP_ICU_VECTOR_MAX_ENTRIES - BSP_FEATURE_ICU_FIXED_IELSR_COUNT;

    if (ielsr_count > BSP_ICU_VECTOR_NUM_ENTRIES) {
        ielsr_count = BSP_ICU_VECTOR_NUM_ENTRIES;
    }

    /* Initialize the IELSR registers for NS images via the Secure Service. */
    for (uint32_t i = 0U; i < ielsr_count; i++) {
        if (g_interrupt_event_link_select[i] != 0U) {
            R_ICU->IELSR[i] = (uint32_t) g_interrupt_event_link_select[i];
        }
    }

 #ifdef RA_NS_INTERFACE_TZ
    /* Route the interrupt events for local non-secure image. */
    enum tfm_platform_err_t err = ra_ns_route_events_cpu0(ielsr_count);

    if (err != TFM_PLATFORM_ERR_SUCCESS) {
        return ARM_DRIVER_ERROR;
    }
 #endif
#endif

    return ARM_DRIVER_OK;
}
