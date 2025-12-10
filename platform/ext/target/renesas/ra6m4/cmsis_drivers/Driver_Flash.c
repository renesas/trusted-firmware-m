/*
 * Copyright (c) 2025 Renesas Electronics Corporation. All rights reserved.
 * Copyright (c) 2018-2020 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Enhanced CMSIS Flash Driver for Renesas RA6M4 using FSP Flash HP
 * Based on RA8 TF-M port implementation with RA6M4-specific adaptations
 */

#include "Driver_Flash.h"
#include "flash_layout.h"
#include "hal_data.h"
#include "r_flash_hp.h"
#include "bsp_feature.h"
#include <string.h>

#ifndef ARG_UNUSED
#define ARG_UNUSED(arg)  (void)(arg)
#endif

/* RA6M4 Flash HP block size */
#define FLASH_HP_BLOCK_SIZE  BSP_FEATURE_FLASH_HP_CF_REGION0_BLOCK_SIZE

#define ARM_FLASH_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,1) /* Enhanced version */

/* Driver version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_FLASH_API_VERSION,
    ARM_FLASH_DRV_VERSION
};

/* Driver capabilities */
static const ARM_FLASH_CAPABILITIES DriverCapabilities = {
    0, /* event_ready */
    2, /* data_width = 0:8-bit, 1:16-bit, 2:32-bit */
    1, /* erase_chip */
    0  /* reserved */
};

/* Code flash info structure */
static ARM_FLASH_INFO FlashInfo = {
    .sector_info  = NULL,
    .sector_count = FLASH_TOTAL_SIZE / FLASH_AREA_IMAGE_SECTOR_SIZE,
    .sector_size  = FLASH_AREA_IMAGE_SECTOR_SIZE,
    .page_size    = 4,
    .program_unit = 1,
    .erased_value = 0xFF
};

/* Data flash info structure */
static ARM_FLASH_INFO DataFlashInfo = {
    .sector_info  = NULL,
    .sector_count = FLASH_DATA_FLASH_SIZE / FLASH_DATA_FLASH_SECTOR_SIZE,
    .sector_size  = FLASH_DATA_FLASH_SECTOR_SIZE,
    .page_size    = FLASH_DATA_FLASH_SECTOR_SIZE,
    .program_unit = 1,  /* 1 byte programming */
    .erased_value = 0xFF
};

/* Driver state */
typedef struct {
    ARM_FLASH_STATUS status;
    uint8_t initialized;
} FLASH_DRIVER_STATE;

static FLASH_DRIVER_STATE flash_state = {0};

/*===========================================================================*/
/* Common Driver Functions                                                   */
/*===========================================================================*/

static ARM_DRIVER_VERSION ARM_Flash_GetVersion(void)
{
    return DriverVersion;
}

static ARM_FLASH_CAPABILITIES ARM_Flash_GetCapabilities(void)
{
    return DriverCapabilities;
}

static ARM_FLASH_STATUS ARM_Flash_GetStatus(void)
{
    return flash_state.status;
}

/*===========================================================================*/
/* Code Flash Driver (Driver_FLASH0)                                         */
/*===========================================================================*/

static int32_t ARM_Flash_Initialize(ARM_Flash_SignalEvent_t cb_event)
{
    ARG_UNUSED(cb_event);

    flash_state.status.busy = 1;
    flash_state.status.error = 1;

    /* Open flash controller - handle already open case */
    fsp_err_t err = R_FLASH_HP_Open(&g_flash0_ctrl, &g_flash0_cfg);
    if (FSP_ERR_ALREADY_OPEN == err) {
        /* Close and reopen to reset state */
        R_FLASH_HP_Close(&g_flash0_ctrl);
        err = R_FLASH_HP_Open(&g_flash0_ctrl, &g_flash0_cfg);
    }

    if (FSP_SUCCESS != err) {
        flash_state.status.busy = 0;
        return ARM_DRIVER_ERROR;
    }

    /* Query flash info to validate configuration */
    flash_info_t info;
    if (FSP_SUCCESS != R_FLASH_HP_InfoGet(&g_flash0_ctrl, &info)) {
        R_FLASH_HP_Close(&g_flash0_ctrl);
        flash_state.status.busy = 0;
        return ARM_DRIVER_ERROR;
    }

    /* Validate flash parameters match our configuration
     * RA6M4 code flash: 1MB total, 8KB sector size */
    uint32_t page_size = info.code_flash.p_block_array[info.code_flash.num_regions - 1U].block_size;

    /* Validate against configured values */
    if (page_size != FLASH_AREA_IMAGE_SECTOR_SIZE) {
        R_FLASH_HP_Close(&g_flash0_ctrl);
        flash_state.status.busy = 0;
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    flash_state.status.busy = 0;
    flash_state.status.error = 0;
    flash_state.initialized = 1;
    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_Uninitialize(void)
{
    if (flash_state.initialized) {
        R_FLASH_HP_Close(&g_flash0_ctrl);
        flash_state.initialized = 0;
    }
    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_PowerControl(ARM_POWER_STATE state)
{
    switch (state) {
    case ARM_POWER_FULL:
        /* Flash is always powered in RA6M4 */
        return ARM_DRIVER_OK;
    case ARM_POWER_OFF:
    case ARM_POWER_LOW:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    default:
        return ARM_DRIVER_ERROR_PARAMETER;
    }
}

static int32_t ARM_Flash_ReadData(uint32_t addr, void *data, uint32_t cnt)
{
    if (!data || cnt == 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Verify address range is within code flash */
    if (addr < FLASH_BASE_ADDRESS ||
        (addr + cnt) > (FLASH_BASE_ADDRESS + FLASH_TOTAL_SIZE)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    flash_state.status.busy = 1;
    memcpy(data, (void *)addr, cnt);
    flash_state.status.busy = 0;

    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_ProgramData(uint32_t addr, const void *data, uint32_t cnt)
{
    if (!data || cnt == 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Verify address range is within code flash */
    if (addr < FLASH_BASE_ADDRESS ||
        (addr + cnt) > (FLASH_BASE_ADDRESS + FLASH_TOTAL_SIZE)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (!flash_state.initialized) {
        return ARM_DRIVER_ERROR;
    }

    flash_state.status.busy = 1;
    flash_state.status.error = 0;

    fsp_err_t err = R_FLASH_HP_Write(&g_flash0_ctrl, (uint32_t)data, addr, cnt);

    flash_state.status.busy = 0;

    if (FSP_SUCCESS != err) {
        flash_state.status.error = 1;
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_EraseSector(uint32_t addr)
{
    /* Verify address is within code flash and aligned to sector boundary */
    if (addr < FLASH_BASE_ADDRESS ||
        addr >= (FLASH_BASE_ADDRESS + FLASH_TOTAL_SIZE) ||
        (addr & (FLASH_AREA_IMAGE_SECTOR_SIZE - 1)) != 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (!flash_state.initialized) {
        return ARM_DRIVER_ERROR;
    }

    flash_state.status.busy = 1;
    flash_state.status.error = 0;

    /* Calculate number of blocks to erase for one sector */
    uint32_t num_blocks = FLASH_AREA_IMAGE_SECTOR_SIZE / FLASH_HP_BLOCK_SIZE;
    fsp_err_t err = R_FLASH_HP_Erase(&g_flash0_ctrl, addr, num_blocks);

    flash_state.status.busy = 0;

    if (FSP_SUCCESS != err) {
        flash_state.status.error = 1;
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_EraseChip(void)
{
    /* Erase entire code flash by erasing all sectors */
    uint32_t addr;
    for (addr = FLASH_BASE_ADDRESS;
         addr < (FLASH_BASE_ADDRESS + FLASH_TOTAL_SIZE);
         addr += FLASH_AREA_IMAGE_SECTOR_SIZE) {
        if (ARM_Flash_EraseSector(addr) != ARM_DRIVER_OK) {
            return ARM_DRIVER_ERROR;
        }
    }
    return ARM_DRIVER_OK;
}

static ARM_FLASH_INFO *ARM_Flash_GetInfo(void)
{
    return &FlashInfo;
}

/* Code Flash Driver Structure */
ARM_DRIVER_FLASH Driver_FLASH0 = {
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

/*===========================================================================*/
/* Data Flash Driver (Driver_FLASH1)                                         */
/* For OTP/NV Counters, ITS, and PS storage                                  */
/*===========================================================================*/

static int32_t ARM_DataFlash_Initialize(ARM_Flash_SignalEvent_t cb_event)
{
    /* Data flash shares the same flash controller as code flash */
    return ARM_Flash_Initialize(cb_event);
}

static int32_t ARM_DataFlash_Uninitialize(void)
{
    /* Data flash shares initialization with code flash
     * Don't uninitialize here as code flash may still need it */
    return ARM_DRIVER_OK;
}

static int32_t ARM_DataFlash_ReadData(uint32_t addr, void *data, uint32_t cnt)
{
    if (!data || cnt == 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Verify address range is within data flash */
    if (addr < FLASH_DATA_FLASH_BASE ||
        (addr + cnt) > (FLASH_DATA_FLASH_BASE + FLASH_DATA_FLASH_SIZE)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    flash_state.status.busy = 1;
    memcpy(data, (void *)addr, cnt);
    flash_state.status.busy = 0;

    return ARM_DRIVER_OK;
}

static int32_t ARM_DataFlash_ProgramData(uint32_t addr, const void *data, uint32_t cnt)
{
    if (!data || cnt == 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Verify address range is within data flash */
    if (addr < FLASH_DATA_FLASH_BASE ||
        (addr + cnt) > (FLASH_DATA_FLASH_BASE + FLASH_DATA_FLASH_SIZE)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (!flash_state.initialized) {
        return ARM_DRIVER_ERROR;
    }

    flash_state.status.busy = 1;
    flash_state.status.error = 0;

    fsp_err_t err = R_FLASH_HP_Write(&g_flash0_ctrl, (uint32_t)data, addr, cnt);

    flash_state.status.busy = 0;

    if (FSP_SUCCESS != err) {
        flash_state.status.error = 1;
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

static int32_t ARM_DataFlash_EraseSector(uint32_t addr)
{
    /* Verify address is within data flash and aligned to sector boundary */
    if (addr < FLASH_DATA_FLASH_BASE ||
        addr >= (FLASH_DATA_FLASH_BASE + FLASH_DATA_FLASH_SIZE) ||
        (addr & (FLASH_DATA_FLASH_SECTOR_SIZE - 1)) != 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (!flash_state.initialized) {
        return ARM_DRIVER_ERROR;
    }

    flash_state.status.busy = 1;
    flash_state.status.error = 0;

    /* Data flash: 1 block = 64 bytes, erase 1 sector */
    uint32_t num_blocks = FLASH_DATA_FLASH_SECTOR_SIZE / FLASH_DATA_FLASH_SECTOR_SIZE;
    fsp_err_t err = R_FLASH_HP_Erase(&g_flash0_ctrl, addr, num_blocks);

    flash_state.status.busy = 0;

    if (FSP_SUCCESS != err) {
        flash_state.status.error = 1;
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

static int32_t ARM_DataFlash_EraseChip(void)
{
    /* Erase entire data flash by erasing all sectors */
    uint32_t addr;
    for (addr = FLASH_DATA_FLASH_BASE;
         addr < (FLASH_DATA_FLASH_BASE + FLASH_DATA_FLASH_SIZE);
         addr += FLASH_DATA_FLASH_SECTOR_SIZE) {
        if (ARM_DataFlash_EraseSector(addr) != ARM_DRIVER_OK) {
            return ARM_DRIVER_ERROR;
        }
    }
    return ARM_DRIVER_OK;
}

static ARM_FLASH_INFO *ARM_DataFlash_GetInfo(void)
{
    return &DataFlashInfo;
}

/* Data Flash Driver Structure */
ARM_DRIVER_FLASH Driver_FLASH1 = {
    ARM_Flash_GetVersion,
    ARM_Flash_GetCapabilities,
    ARM_DataFlash_Initialize,
    ARM_DataFlash_Uninitialize,
    ARM_Flash_PowerControl,
    ARM_DataFlash_ReadData,
    ARM_DataFlash_ProgramData,
    ARM_DataFlash_EraseSector,
    ARM_DataFlash_EraseChip,
    ARM_Flash_GetStatus,
    ARM_DataFlash_GetInfo
};
