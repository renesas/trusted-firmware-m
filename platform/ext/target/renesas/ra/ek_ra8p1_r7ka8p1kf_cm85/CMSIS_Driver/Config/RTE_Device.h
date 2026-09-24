/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RTE_DEVICE_H
#define RTE_DEVICE_H

/*
 * Flash channel configuration.
 *
 *   RTE_FLASH0 - 1 = enable, 0 = disable
 *
 * The flash geometry (program unit, sector size, base address) is configured
 * in config.cmake, not here.
 */
#define RTE_FLASH0          1

#endif /* RTE_DEVICE_H */
