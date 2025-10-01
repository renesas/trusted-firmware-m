/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TARGET_CFG_H__
#define __TARGET_CFG_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Configures the system
 *
 * \return Returns error code as specified in \ref tfm_plat_err_t
 */
enum tfm_plat_err_t target_cfg_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __TARGET_CFG_H__ */