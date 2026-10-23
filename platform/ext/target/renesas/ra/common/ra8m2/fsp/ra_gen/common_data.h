/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef COMMON_DATA_H_
#define COMMON_DATA_H_

#include <RTE_Device.h>
#include <stdint.h>
#include "bsp_api.h"
#include "bsp_pin_cfg.h"

#if (RTE_FLASH0 == 1)
#include "r_flash_api.h"
#include "r_mram.h"
#endif

#include "r_ioport.h"

FSP_HEADER

#if (RTE_FLASH0 == 1)

/* Flash on MRAM Instance */
extern const flash_instance_t g_mram0;

/* Access the MRAM instance using these structures when calling API functions directly (::p_api is
 * not used). */
extern mram_instance_ctrl_t g_mram0_ctrl;
extern const flash_cfg_t g_mram0_cfg;

#ifndef NULL
void NULL(flash_callback_args_t *p_args);
#endif

#endif /* end RTE_FLASH0 */

#define IOPORT_CFG_NAME g_bsp_pin_cfg
#define IOPORT_CFG_OPEN R_IOPORT_Open
#define IOPORT_CFG_CTRL g_ioport_ctrl

/* IOPORT Instance */
extern const ioport_instance_t g_ioport;

/* IOPORT control structure. */
extern ioport_instance_ctrl_t g_ioport_ctrl;
void g_common_init(void);

FSP_FOOTER
#endif /* COMMON_DATA_H_ */
