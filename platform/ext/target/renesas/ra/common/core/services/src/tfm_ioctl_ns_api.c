/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>
#include "tfm_ioctl_api.h"
#include "tfm_platform_api.h"

enum tfm_platform_err_t
tfm_platform_irq_cpu_sel(enum tfm_irq_cpu_sel_cpu_t cpu, uint32_t event,
                         uint32_t *result)
{
    enum tfm_platform_err_t ret;
    psa_invec in_vec;
    psa_outvec out_vec;
    struct tfm_irq_cpu_sel_service_args_t args = {0};
    struct tfm_irq_cpu_sel_service_out_t out = {0};

    if (result == NULL) {
        return TFM_PLATFORM_ERR_INVALID_PARAM;
    }

    args.cpu = cpu;
    args.event = event;

    in_vec.base = (const void *)&args;
    in_vec.len = sizeof(args);

    out_vec.base = (void *)&out;
    out_vec.len = sizeof(out);

    ret = tfm_platform_ioctl(TFM_PLATFORM_IOCTL_IRQ_CPU_SEL_SERVICE,
                             &in_vec,
                             &out_vec);

    *result = out.result;

    return ret;
}
