/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __TFM_IOCTL_API__
#define __TFM_IOCTL_API__

#include <limits.h>
#include <stdint.h>
#include "tfm_platform_api.h"

#ifdef __cplusplus
extern "C" {
#endif

enum tfm_platform_ioctl_request_types_t {
    TFM_PLATFORM_IOCTL_IRQ_CPU_SEL_SERVICE = 0,
};

/*!
 * \enum tfm_irq_cpu_sel_cpu_t
 *
 * \brief CPU an interrupt event can be routed to (INTSELR)
 */
enum tfm_irq_cpu_sel_cpu_t {
    TFM_IRQ_CPU_SEL_CPU0 = 0,               /*!< CPU0 (CM85) */
    TFM_IRQ_CPU_SEL_CPU1,                   /*!< CPU1 (CM33) */
    TFM_IRQ_CPU_SEL_MAX = INT_MAX           /*!< Max to force enum max size */
};

/*!
 * \struct tfm_irq_cpu_sel_service_args_t
 *
 * \brief Argument list for the IRQ CPU select service
 */
struct tfm_irq_cpu_sel_service_args_t {
    enum tfm_irq_cpu_sel_cpu_t cpu;
    uint32_t event;
};

/*!
 * \struct tfm_irq_cpu_sel_service_out_t
 *
 * \brief Output list for the IRQ CPU select service
 */
struct tfm_irq_cpu_sel_service_out_t {
    uint32_t result;                        /*!< Return error value */
};

/*!
 * \brief Routes one interrupt event to the selected CPU
 *
 * \param[in]  cpu       CPU to route the event to
 * \param[in]  event    Interrupt event number (ELC_EVENT_*)
 * \param[out] result    Return error value
 *
 * \return Returns values as specified by the \ref tfm_platform_err_t
 */
enum tfm_platform_err_t
tfm_platform_irq_cpu_sel(enum tfm_irq_cpu_sel_cpu_t cpu, uint32_t event,
                         uint32_t *result);

#ifdef __cplusplus
}
#endif

#endif /* __TFM_IOCTL_API__ */
