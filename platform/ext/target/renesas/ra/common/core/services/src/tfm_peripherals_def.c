/*
 * Copyright (c) 2018-2024, Arm Limited. All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_peripherals_def.h"

#include <stddef.h>
#include <stdint.h>

#include "common_target_cfg.h"
#include "target_cfg.h"
#include "tfm_hal_device_header.h"
#include "tfm_utils.h"

/*
 * Limit of the MPU region of a peripheral. The v8-M isolation HAL needs a base
 * aligned to 32 bytes and a limit whose low 5 bits are 0x1F.
 */
#define RA_PERIPHERAL_LIMIT(base, size) ((base) + (((size) + 0x1FU) & ~0x1FU) - 1U)

/* Platform data of an entry of RA_PERIPHERALS(X, SEP) */
#define RA_PERIPHERAL_DEFINE(name, secure, bank, bit, base, size)                      \
    _Static_assert((secure) == 0 || (secure) == 1, "secure of " #name " is not 0 or 1"); \
    _Static_assert(((base) & 0x1FU) == 0U, "base of " #name " is not 32 byte aligned"); \
    struct platform_data_t tfm_peripheral_##name = {                                   \
        .periph_start = (base),                                                        \
        .periph_limit = RA_PERIPHERAL_LIMIT(base, size),                               \
        .periph_ppc_bank = PPC_SP_PERIPH_##bank,                                       \
        .periph_ppc_mask = (bit),                                                      \
    }

RA_PERIPHERALS(RA_PERIPHERAL_DEFINE, ;)

/*
 * Entry of RA_PERIPHERALS(X, SEP) in the allowed named MMIO, selected at
 * compile time on its secure flag (1 or 0, see RA_PSAR_SECURE): a secure
 * peripheral gives "&<platform data>,", a peripheral that is not secure gives
 * nothing. The list holds the secure peripherals only, in the order of
 * RA_PERIPHERALS, so the index of one in the list is small.
 *
 * That matters under isolation level 3: the Armv8-M isolation HAL encodes the
 * index of a named MMIO in 3 bits, and tfm_hal_bind_boundary() fails for an
 * index above 7, so no more than 7 peripherals can be secure there.
 */
#define RA_PERIPHERAL_ENTRY(name, secure, ...) \
    RA_PERIPHERAL_CAT(RA_ENTRY_, secure)(name)
/* Separator of the list entries that has to be empty */
#define RA_NO_SEPARATOR
#define RA_ENTRY_1(name) (uintptr_t)&tfm_peripheral_##name,
#define RA_ENTRY_0(...)

/* Allowed named MMIO of this platform */
static const uintptr_t partition_named_mmio_list[] = {
    RA_PERIPHERALS(RA_PERIPHERAL_ENTRY, RA_NO_SEPARATOR)
};

void get_partition_named_mmio_list(const uintptr_t **list, size_t *length)
{
    *list = partition_named_mmio_list;
    *length = ARRAY_SIZE(partition_named_mmio_list);
}
