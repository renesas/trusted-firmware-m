/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bsp_api.h>

/* Do not build these data structures if no interrupts are currently allocated because IAR will have
 * build errors.
 */

/* Skip -Wmissing-prototypes for the weak default IRQ handlers. They have no prototypes because the
 * WEAK_IRQ_HANDLER macro in vector_data.h defines them only.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
IRQ_FOREACH(WEAK_IRQ_HANDLER)
#pragma GCC diagnostic pop

BSP_DONT_REMOVE const
	fsp_vector_t BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) g_vector_table[BSP_ICU_VECTOR_NUM_ENTRIES] = {
#ifdef RA_NS_INTERFACE_TZ
    /* Interrupt handler of local NSPE */

#elif defined(RA_NS_INTERFACE_MAILBOX)
    /* Interrupt handler of remote NSPE */
#else
	IRQ_FOREACH(IRQ_VECTOR_ENTRY)
#endif
};

#if BSP_FEATURE_ICU_HAS_IELSR
const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] = {
#ifdef RA_NS_INTERFACE_TZ
    /* Interrupt event for local NSPE */
#endif
#ifdef RA_NS_INTERFACE_MAILBOX
    /* Interrupt event for remote NSPE */
#endif
};

#endif
