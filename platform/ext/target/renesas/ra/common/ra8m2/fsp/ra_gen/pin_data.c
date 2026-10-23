/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <RTE_PinCfg.h>
#include <bsp_api.h>
#include <r_ioport.h>

const ioport_pin_cfg_t g_bsp_pin_cfg_data[] = {
#if _RTE_HAS_USART0
    RTE_PINS_UART(0)
#endif
#if _RTE_HAS_USART8
    RTE_PINS_UART(8)
#endif
#if _RTE_HAS_USART9
    RTE_PINS_UART(9)
#endif
};

const ioport_cfg_t g_bsp_pin_cfg = {
	.number_of_pins = sizeof(g_bsp_pin_cfg_data) / sizeof(ioport_pin_cfg_t),
	.p_pin_cfg_data = &g_bsp_pin_cfg_data[0],
};

#if BSP_TZ_SECURE_BUILD

void R_BSP_PinCfgSecurityInit(void);

/* Initialize SAR registers for secure pins. */
void R_BSP_PinCfgSecurityInit(void)
{
#if (2U == BSP_FEATURE_IOPORT_VERSION)
	uint32_t pmsar[BSP_FEATURE_BSP_NUM_PMSAR];
#else
	uint16_t pmsar[BSP_FEATURE_BSP_NUM_PMSAR];
#endif
	memset(pmsar, 0xFF, BSP_FEATURE_BSP_NUM_PMSAR * sizeof(R_PMISC->PMSAR[0]));

	for (uint32_t i = 0; i < g_bsp_pin_cfg.number_of_pins; i++) {
		uint32_t port_pin = g_bsp_pin_cfg.p_pin_cfg_data[i].pin;
		uint32_t port = port_pin >> 8U;
		uint32_t pin = port_pin & 0xFFU;
		pmsar[port] &= (uint16_t)~(1U << pin);
	}

	for (uint32_t i = 0; i < BSP_FEATURE_BSP_NUM_PMSAR; i++) {
#if (2U == BSP_FEATURE_IOPORT_VERSION)
#if BSP_SECONDARY_CORE_BUILD
		R_PMISC->PMSAR[i].PMSAR &= (uint16_t)pmsar[i];
#else
		R_PMISC->PMSAR[i].PMSAR = (uint16_t)pmsar[i];
#endif
#else
		R_PMISC->PMSAR[i].PMSAR = pmsar[i];
#endif
	}
}
#endif
