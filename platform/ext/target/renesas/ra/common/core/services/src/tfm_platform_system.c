/*
 * Copyright (c) 2018-2024, Arm Limited. All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "bsp_api.h"
#include "common_target_cfg.h"
#include "tfm_hal_device_header.h"
#include "tfm_hal_platform.h"
#include "tfm_ioctl_api.h"
#include "tfm_platform_system.h"

#define ICU_BITS_PER_WORD    (32U)

/*
 * Returns true if the event was used by the Secure image.
 */
static bool tfm_platform_irq_is_secure_event(uint32_t event)
{
	uint32_t slot;
	uint32_t ielsr_count = sizeof(R_ICU->IELSR) / sizeof(R_ICU->IELSR[0]);
#if (BSP_CFG_CPU_CORE == 1)
	uint32_t volatile *p_icusar = &R_CPSCU->ICUSARJ;
#else
	uint32_t volatile *p_icusar = &R_CPSCU->ICUSARG;
#endif

	for (slot = 0U; slot < ielsr_count; slot++) {
		/* Check the ICU security attribution before reading IELSR. */
		if ((p_icusar[slot / ICU_BITS_PER_WORD] & (1U << (slot % ICU_BITS_PER_WORD))) != 0U) {
            /* This slot is non-secure. Skip it */
			continue;
		}

		if (R_ICU->IELSR_b[slot].IELS == event) {
            /* If the event is found in this slot, it's a secure event. */
			return true;
		}
	}

	return false;
}

void tfm_platform_hal_system_reset(void)
{
	__disable_irq();
	tfm_hal_system_reset(TFM_PLAT_SWSYN_DEFAULT);
}

/*
 * Selects the CPU to which an interrupt event is routed.
 */
static enum tfm_platform_err_t tfm_platform_hal_ns_irq_cpu_sel(const psa_invec *in_vec,
                                                                const psa_outvec *out_vec)
{
	struct tfm_irq_cpu_sel_service_args_t *args;
	struct tfm_irq_cpu_sel_service_out_t *out;
	uint32_t intselr_count = sizeof(R_ICU->INTSELR) / sizeof(R_ICU->INTSELR[0]);

	if ((in_vec == NULL) || (out_vec == NULL) || (in_vec->base == NULL) || (out_vec->base == NULL) ||
	    (in_vec->len != sizeof(*args)) || (out_vec->len != sizeof(*out))) {
		return TFM_PLATFORM_ERR_INVALID_PARAM;
	}

	args = (struct tfm_irq_cpu_sel_service_args_t *)in_vec->base;
	out = (struct tfm_irq_cpu_sel_service_out_t *)out_vec->base;
	out->result = -1;

	if ((args->cpu != TFM_IRQ_CPU_SEL_CPU0) && (args->cpu != TFM_IRQ_CPU_SEL_CPU1)) {
		return TFM_PLATFORM_ERR_INVALID_PARAM;
	}

	/* Event 0 means no event. */
	if ((args->event == 0U) || ((args->event >> 5) >= intselr_count)) {
		return TFM_PLATFORM_ERR_INVALID_PARAM;
	}

	if (tfm_platform_irq_is_secure_event(args->event)) {
		return TFM_PLATFORM_ERR_INVALID_PARAM;
	}

	if (args->cpu == TFM_IRQ_CPU_SEL_CPU1) {
		R_ICU->INTSELR[args->event >> 5] |= (1U << (args->event % ICU_BITS_PER_WORD));
	} else {
		R_ICU->INTSELR[args->event >> 5] &= ~(1U << (args->event % ICU_BITS_PER_WORD));
	}

	out->result = 0;

	return TFM_PLATFORM_ERR_SUCCESS;
}

enum tfm_platform_err_t tfm_platform_hal_ioctl(tfm_platform_ioctl_req_t request, psa_invec *in_vec,
					       psa_outvec *out_vec)
{
	switch (request) {
	case TFM_PLATFORM_IOCTL_IRQ_CPU_SEL_SERVICE:
		return tfm_platform_hal_ns_irq_cpu_sel(in_vec, out_vec);
	default:
		break;
	}

	return TFM_PLATFORM_ERR_NOT_SUPPORTED;
}
