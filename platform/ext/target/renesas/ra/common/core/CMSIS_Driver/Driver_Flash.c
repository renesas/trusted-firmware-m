/*
 * Copyright (c) 2013-2020 Arm Limited. All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cmsis_compiler.h>
#include <string.h>

#include "Driver_Flash.h"
#include "RTE_Device.h"
#include "r_flash_api.h"

#ifndef ARG_UNUSED
#define ARG_UNUSED(arg) (void)arg
#endif

/* CMSIS Flash driver version */
#define ARM_FLASH_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)

/**
 * Event ready values for ARM_FLASH_CAPABILITIES::event_ready
 * \ref ARM_FLASH_CAPABILITIES
 */
enum {
    EVENT_READY_NOT_AVAILABLE = 0u,
    EVENT_READY_AVAILABLE
};

/**
 * Data width values for ARM_FLASH_CAPABILITIES::data_width
 * \ref ARM_FLASH_CAPABILITIES
 */
enum {
    DATA_WIDTH_8BIT = 0u,
    DATA_WIDTH_16BIT,
    DATA_WIDTH_32BIT,
    DATA_WIDTH_ENUM_SIZE
};

/**
 * Erase chip values for ARM_FLASH_CAPABILITIES::erase_chip
 * \ref ARM_FLASH_CAPABILITIES
 */
enum {
    CHIP_ERASE_NOT_SUPPORTED = 0u,
    CHIP_ERASE_SUPPORTED
};

/**
 * Flash busy values flash status
 * \ref ARM_FLASH_STATUS
 */
enum {
    DRIVER_STATUS_IDLE = 0u,
    DRIVER_STATUS_BUSY
};

/**
 * Flash error values flash status
 * \ref ARM_FLASH_STATUS
 */
enum {
    DRIVER_STATUS_NO_ERROR = 0u,
    DRIVER_STATUS_ERROR
};

/* CMSIS Flash device context */
struct arm_flash_dev_t {
    flash_instance_t const *instance; /*!< FLASH memory device structure */
    ARM_FLASH_INFO         *info;     /*!< FLASH memory device info */
    ARM_FLASH_STATUS       *status;
};

/* Check program-unit alignment. */
static bool is_write_aligned(const struct arm_flash_dev_t *flash_dev, uint32_t value)
{
    return ((value % flash_dev->info->program_unit) == 0U);
}

/* Check sector alignment. */
static bool is_sector_aligned(const struct arm_flash_dev_t *flash_dev, uint32_t offset)
{
    return ((offset % flash_dev->info->sector_size) == 0U);
}

/* Check that a range lies within flash boundaries. */
static bool is_range_valid(const struct arm_flash_dev_t *flash_dev, uint32_t offset, uint32_t size)
{
    uint64_t flash_size;
    uint64_t end;

    flash_size = (uint64_t)flash_dev->info->sector_count * (uint64_t)flash_dev->info->sector_size;
    end = (uint64_t)offset + (uint64_t)size;

    return (end <= flash_size);
}

/* Convert FSP errors to CMSIS-Driver return codes. */
static int32_t flash_error_to_cmsis(fsp_err_t err)
{
    switch (err) {
    case FSP_SUCCESS:
        return ARM_DRIVER_OK;
    case FSP_ERR_INVALID_ADDRESS:
    case FSP_ERR_INVALID_SIZE:
    case FSP_ERR_INVALID_BLOCKS:
        return ARM_DRIVER_ERROR_PARAMETER;
    case FSP_ERR_IN_USE:
        return ARM_DRIVER_ERROR_BUSY;
    default:
        return ARM_DRIVER_ERROR;
    }
}

#if (RTE_FLASH0)

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_FLASH_API_VERSION,
    ARM_FLASH_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_FLASH_CAPABILITIES DriverCapabilities = {
    .event_ready = EVENT_READY_NOT_AVAILABLE,
    .data_width  = DATA_WIDTH_8BIT,
    .erase_chip  = CHIP_ERASE_NOT_SUPPORTED
};

static ARM_FLASH_INFO FlashInfo = {
    .sector_info  = NULL, /* Uniform sector layout */
    .sector_count = FLASH_TOTAL_SIZE / FLASH_AREA_IMAGE_SECTOR_SIZE,
    .sector_size  = FLASH_AREA_IMAGE_SECTOR_SIZE,
    .page_size    = FLASH_PAGE_SIZE,
    .program_unit = FLASH_PROGRAM_UNIT,
    .erased_value = 0xFF
};

static ARM_FLASH_STATUS FlashStatus = {
    .busy     = DRIVER_STATUS_IDLE,
    .error    = DRIVER_STATUS_NO_ERROR,
    .reserved = 0,
};

extern flash_instance_t const *gp_tfm_flash_instance;

static struct arm_flash_dev_t arm_flash0_dev = {
    .instance = NULL,
    .info   = &FlashInfo,
    .status = &FlashStatus,
};

struct arm_flash_dev_t *FLASH0_DEV = &arm_flash0_dev;

static uintptr_t get_flash_access_addr(uint32_t offset)
{
    uintptr_t base_addr;

    if (offset < RA_HAL_FLASH_SECURE_BOUNDARY) {
        base_addr = S_ROM_ALIAS_BASE;
    } else {
        base_addr = NS_ROM_ALIAS_BASE;
    }

    return base_addr + offset;
}

static ARM_DRIVER_VERSION ARM_Flash_GetVersion(void)
{
    return DriverVersion;
}

static ARM_FLASH_CAPABILITIES ARM_Flash_GetCapabilities(void)
{
    return DriverCapabilities;
}

static int32_t ARM_Flash_Initialize(ARM_Flash_SignalEvent_t cb_event)
{
    ARG_UNUSED(cb_event);
    fsp_err_t err;
    uint32_t flash_size;
    flash_info_t info;

    if ((gp_tfm_flash_instance == NULL) ||
        (gp_tfm_flash_instance->p_ctrl == NULL) ||
        (gp_tfm_flash_instance->p_cfg == NULL)) {
        return ARM_DRIVER_ERROR;
    }

    FLASH0_DEV->instance = gp_tfm_flash_instance;

    err = FLASH0_DEV->instance->p_api->open(FLASH0_DEV->instance->p_ctrl, FLASH0_DEV->instance->p_cfg);

    if (err != FSP_SUCCESS && err != FSP_ERR_ALREADY_OPEN) {
        return ARM_DRIVER_ERROR;
    }

    /* Validate the flash geometry against the compile-time flash map.
     * Only the total flash size is checked because hardware block sizes may not
     * match the software sector size. block_section_end_addr is exclusive, so
     * no +1 is required when calculating the flash size.
     */
    err = FLASH0_DEV->instance->p_api->infoGet(FLASH0_DEV->instance->p_ctrl, &info);

    if (err != FSP_SUCCESS) {
        return ARM_DRIVER_ERROR;
    }

    flash_size =
        info.code_flash.p_block_array[info.code_flash.num_regions - 1].block_section_end_addr -
        info.code_flash.p_block_array[0].block_section_st_addr;

    if (flash_size != FLASH_TOTAL_SIZE) {
        FLASH0_DEV->instance->p_api->close(FLASH0_DEV->instance->p_ctrl);
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_Uninitialize(void)
{
    if ((FLASH0_DEV->instance == NULL)
        || (FLASH0_DEV->instance->p_api == NULL)
        || (FLASH0_DEV->instance->p_ctrl == NULL)) {
        return ARM_DRIVER_ERROR;
    }
    FLASH0_DEV->instance->p_api->close(FLASH0_DEV->instance->p_ctrl);
    FLASH0_DEV->instance = NULL;

    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_PowerControl(ARM_POWER_STATE state)
{
    switch (state) {
    case ARM_POWER_FULL:
        /* Nothing to be done */
        return ARM_DRIVER_OK;

    case ARM_POWER_OFF:
    case ARM_POWER_LOW:
    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
}

static int32_t ARM_Flash_ReadData(uint32_t addr, void *data, uint32_t cnt)
{
    FLASH0_DEV->status->error = DRIVER_STATUS_NO_ERROR;

    if (data == NULL) {
        FLASH0_DEV->status->error = DRIVER_STATUS_ERROR;
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (!is_range_valid(FLASH0_DEV, addr, cnt)) {
        FLASH0_DEV->status->error = DRIVER_STATUS_ERROR;
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    FLASH0_DEV->status->busy  = DRIVER_STATUS_BUSY;
    /* Access must use the alias matching the target area's security
     * attribution (see get_flash_access_addr()).
     */
    memcpy(data, (const void *)(get_flash_access_addr(addr)), cnt);

    FLASH0_DEV->status->busy = DRIVER_STATUS_IDLE;

    return (int32_t)cnt;
}

static int32_t ARM_Flash_ProgramData(uint32_t addr, const void *data, uint32_t cnt)
{
    fsp_err_t err;
    uint32_t primask;

    FLASH0_DEV->status->error = DRIVER_STATUS_NO_ERROR;

    if (data == NULL) {
        FLASH0_DEV->status->error = DRIVER_STATUS_ERROR;
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Only FLASH_PROGRAM_UNIT aligned writes are allowed. */
    if (!is_range_valid(FLASH0_DEV, addr, cnt)
        || !is_write_aligned(FLASH0_DEV, addr)
        || !is_write_aligned(FLASH0_DEV, cnt)) {
        FLASH0_DEV->status->error = DRIVER_STATUS_ERROR;
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    FLASH0_DEV->status->busy  = DRIVER_STATUS_BUSY;
    /* The FSP write helpers are placed in SRAM (.ram_from_flash),
     * and interrupts must stay disabled so that no ISR executes mid-write.
     */
    primask = __get_PRIMASK();

    __disable_irq();

    err = FLASH0_DEV->instance->p_api->write(FLASH0_DEV->instance->p_ctrl, (uintptr_t)data,
                       get_flash_access_addr(addr), cnt);

    /* Re-enable the interrupts */
    __set_PRIMASK(primask);

    FLASH0_DEV->status->busy = DRIVER_STATUS_IDLE;

    if (err != FSP_SUCCESS) {
        FLASH0_DEV->status->error = DRIVER_STATUS_ERROR;
        return flash_error_to_cmsis(err);
    }

    return (int32_t)cnt;
}

static int32_t ARM_Flash_EraseSector(uint32_t addr)
{
    fsp_err_t err;
    uint32_t primask;

    FLASH0_DEV->status->error = DRIVER_STATUS_NO_ERROR;

    if (!is_range_valid(FLASH0_DEV, addr, FLASH_AREA_IMAGE_SECTOR_SIZE)
        || !is_sector_aligned(FLASH0_DEV, addr)) {
        FLASH0_DEV->status->error = DRIVER_STATUS_ERROR;
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    FLASH0_DEV->status->busy  = DRIVER_STATUS_BUSY;
    /* The FSP write helpers are placed in SRAM (.ram_from_flash),
     * and interrupts must stay disabled so that no ISR executes mid-write.
     */
    primask = __get_PRIMASK();

    __disable_irq();
    /* Erase operates on hardware erase blocks, so FLASH_PAGE_SIZE must match
     * the device-reported erase block size. FLASH_AREA_IMAGE_SECTOR_SIZE is
     * converted to a block count by dividing it by FLASH_PAGE_SIZE; therefore,
     * any mismatch between FLASH_PAGE_SIZE and the actual erase block size may
     * result in an incorrect erase range.
     */
    err = FLASH0_DEV->instance->p_api->erase(FLASH0_DEV->instance->p_ctrl, get_flash_access_addr(addr),
                       FLASH_AREA_IMAGE_SECTOR_SIZE / FLASH_PAGE_SIZE);

    /* Re-enable the interrupts */
    __set_PRIMASK(primask);
    FLASH0_DEV->status->busy = DRIVER_STATUS_IDLE;

    if (err != FSP_SUCCESS) {
        FLASH0_DEV->status->error = DRIVER_STATUS_ERROR;
        return flash_error_to_cmsis(err);
    }

    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_EraseChip(void)
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_FLASH_STATUS ARM_Flash_GetStatus(void)
{
    return *FLASH0_DEV->status;
}

static ARM_FLASH_INFO *ARM_Flash_GetInfo(void)
{
    return FLASH0_DEV->info;
}

ARM_DRIVER_FLASH Driver_Flash0 = {
    ARM_Flash_GetVersion,
    ARM_Flash_GetCapabilities,
    ARM_Flash_Initialize,
    ARM_Flash_Uninitialize,
    ARM_Flash_PowerControl,
    ARM_Flash_ReadData,
    ARM_Flash_ProgramData,
    ARM_Flash_EraseSector,
    ARM_Flash_EraseChip,
    ARM_Flash_GetStatus,
    ARM_Flash_GetInfo
};

#endif /* RTE_FLASH0 */
