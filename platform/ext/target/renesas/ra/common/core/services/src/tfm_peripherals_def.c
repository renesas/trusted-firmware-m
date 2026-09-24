/*
 * Copyright (c) 2018-2024, Arm Limited. All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_peripherals_def.h"
#include "target_cfg.h"
#include "common_target_cfg.h"
#include <stddef.h>
#include <stdint.h>
#include "tfm_hal_device_header.h"
#include <stdint.h>
#include "tfm_utils.h"

/* TODO: Provide struct platform_data_t for each peripheral when it is available */

/* Allowed named MMIO of this platform */
static const uintptr_t partition_named_mmio_list[] = {
    /* TODO: Provide the allowed named MMIO addresses when available */
};

void get_partition_named_mmio_list(const uintptr_t **list, size_t *length)
{
    *list = partition_named_mmio_list;
    *length = ARRAY_SIZE(partition_named_mmio_list);
}
