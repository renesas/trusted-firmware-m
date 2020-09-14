/* generated pin source file - do not edit */
#include "bsp_api.h"
#include "r_ioport_api.h"
#include "r_ioport.h"
#include "bsp_feature.h"
#include "renesas.h"

#if BSP_TZ_SECURE_BUILD

#define BSP_PRV_NUM_PMSAR  (sizeof(R_PMISC->PMSAR) / sizeof(R_PMISC->PMSAR[0]))

void R_BSP_PinCfgSecurityInit(void);

/* Initialize SAR registers for secure pins. */
void R_BSP_PinCfgSecurityInit(void)
{
	const ioport_pin_cfg_t g_bsp_pin_cfg_data[] = {
	            {
	                .pin     = BSP_IO_PORT_06_PIN_14, // RXD7
	                .pin_cfg = ((uint32_t) IOPORT_CFG_PERIPHERAL_PIN | (uint32_t) IOPORT_PERIPHERAL_SCI1_3_5_7_9)
	            },
	            {
	                .pin     = BSP_IO_PORT_06_PIN_13, // TXD7
	                .pin_cfg = ((uint32_t) IOPORT_CFG_PERIPHERAL_PIN | (uint32_t) IOPORT_PERIPHERAL_SCI1_3_5_7_9)
	            },
	            {
	                .pin = BSP_IO_PORT_01_PIN_08,
	                .pin_cfg = ((uint32_t) IOPORT_CFG_PERIPHERAL_PIN | (uint32_t) IOPORT_PERIPHERAL_DEBUG),
	            },
	            {
	                .pin = BSP_IO_PORT_01_PIN_09,
	                .pin_cfg = ((uint32_t) IOPORT_CFG_PERIPHERAL_PIN | (uint32_t) IOPORT_PERIPHERAL_DEBUG),
	            },
	            {
	                .pin = BSP_IO_PORT_01_PIN_10,
	                .pin_cfg = ((uint32_t) IOPORT_CFG_PERIPHERAL_PIN | (uint32_t) IOPORT_PERIPHERAL_DEBUG),
	            },
	            {
	                .pin = BSP_IO_PORT_03_PIN_00,
	                .pin_cfg = ((uint32_t) IOPORT_CFG_PERIPHERAL_PIN | (uint32_t) IOPORT_PERIPHERAL_DEBUG),
	            },
	    };

	const ioport_cfg_t g_bsp_pin_cfg = {
	    .number_of_pins = sizeof(g_bsp_pin_cfg_data)/sizeof(ioport_pin_cfg_t),
	    .p_pin_cfg_data = &g_bsp_pin_cfg_data[0],
	};

    uint16_t pmsar[BSP_PRV_NUM_PMSAR];
    memset(pmsar, 0xFF, BSP_PRV_NUM_PMSAR * sizeof(R_PMISC->PMSAR[0]));


    for(uint32_t i = 0; i < g_bsp_pin_cfg.number_of_pins; i++)
    {
        uint32_t port_pin = g_bsp_pin_cfg.p_pin_cfg_data[i].pin;
        uint32_t port = port_pin >> 8U;
        uint32_t pin = port_pin & 0xFFU;
        pmsar[port] &= (uint16_t) ~(1U << pin);
    }

    for(uint32_t i = 0; i < BSP_PRV_NUM_PMSAR; i++)
    {
        R_PMISC->PMSAR[i].PMSAR = pmsar[i];
    }
}
#endif
