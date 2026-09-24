/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RTE_PIN_CFG_H_
#define RTE_PIN_CFG_H_

#include <bsp_api.h>
#include <r_ioport.h>
#include <RTE_Device.h>

/*
 * Helpers to construct FSP ioport_pin_cfg_t entries from the raw
 * port/pin numbers defined in RTE_Device.h.
 *
 *   BSP_IO_PORT_xx_PIN_yy  = (port << 8) | pin      e.g. 0x0602 -> port 6, pin 2
 */
#define RTE_PORT_PIN(port, pin)  (((uint32_t)(port) << 8) | (uint32_t)(pin))

/* Convert raw PSEL number to FSP peripheral value */
#define RTE_PSEL(val)  ((uint32_t)(val) << IOPORT_PRV_PFS_PSEL_OFFSET)

/* Generic pin entry - psel is the final FSP IOPORT_PERIPHERAL_* value */
#define RTE_PIN_CFG(port, _pin, psel) \
    {.pin = RTE_PORT_PIN(port, _pin), \
     .pin_cfg = ((uint32_t) IOPORT_CFG_PERIPHERAL_PIN | (uint32_t)(psel))},

#endif /* RTE_PIN_CFG_H_ */
