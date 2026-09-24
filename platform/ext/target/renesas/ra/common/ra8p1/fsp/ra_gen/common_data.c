/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "common_data.h"

mram_instance_ctrl_t g_mram0_ctrl;

const flash_cfg_t g_mram0_cfg = {
	.data_flash_bgo = false,
	.p_callback = NULL,
	.p_context = NULL,
#if defined(VECTOR_NUMBER_MRAM_MRCRD)
	.err_irq = VECTOR_NUMBER_MRAM_MRCRD,
#else
	.err_irq = FSP_INVALID_VECTOR,
#endif
	.err_ipl = (BSP_IRQ_DISABLED),
	.irq = FSP_INVALID_VECTOR,
	.ipl = BSP_IRQ_DISABLED,
};

/* Instance structure to use this module. */
const flash_instance_t g_mram0 = {
	.p_ctrl = &g_mram0_ctrl,
	.p_cfg = &g_mram0_cfg,
	.p_api = &g_flash_on_mram,
};

flash_instance_t const *const gp_tfm_flash_instance = &g_mram0;

ioport_instance_ctrl_t g_ioport_ctrl;

const ioport_instance_t g_ioport = {
	.p_api = &g_ioport_on_ioport,
	.p_ctrl = &g_ioport_ctrl,
	.p_cfg = &g_bsp_pin_cfg,
};

void g_common_init(void)
{
}
