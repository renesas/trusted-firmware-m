/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 * Copyright (c) 2019-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_peripherals_def.h"
#include "common_target_cfg.h"

/* RA6M4 does not have MPC/PPC, so platform_data structures are minimal */

/* Named MMIO list for partition isolation */
const uintptr_t partition_named_mmio_list[] = {
    /* Currently empty - can be populated with peripheral addresses as needed */
};

const size_t partition_named_mmio_list_size =
    sizeof(partition_named_mmio_list) / sizeof(partition_named_mmio_list[0]);

void get_partition_named_mmio_list(const uintptr_t** list, size_t* length)
{
    *list = partition_named_mmio_list;
    *length = partition_named_mmio_list_size;
}
