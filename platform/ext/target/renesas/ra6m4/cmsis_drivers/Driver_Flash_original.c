/*
 * Copyright (c) 2025 Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * CMSIS Flash Driver wrapper for Renesas FSP Flash HP
 */

#include "Driver_Flash.h"
#include "flash_layout.h"
#include "hal_data.h"
#include "r_flash_hp.h"
#include "bsp_feature.h"
#include <string.h>

/* RA6M4 Flash HP block size */
#define FLASH_HP_BLOCK_SIZE  BSP_FEATURE_FLASH_HP_CF_REGION0_BLOCK_SIZE

#define ARM_FLASH_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)

static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_FLASH_API_VERSION,
    ARM_FLASH_DRV_VERSION
};

static const ARM_FLASH_CAPABILITIES DriverCapabilities = {
    0, /* event_ready */
    2, /* data_width = 0:8-bit, 1:16-bit, 2:32-bit */
    1, /* erase_chip */
    0  /* reserved */
};

static ARM_FLASH_INFO FlashInfo = {
    .sector_info  = NULL,
    .sector_count = FLASH_TOTAL_SIZE / FLASH_AREA_IMAGE_SECTOR_SIZE,
    .sector_size  = FLASH_AREA_IMAGE_SECTOR_SIZE,
    .page_size    = 4,
    .program_unit = 1,
    .erased_value = 0xFF
};

typedef struct {
    ARM_FLASH_STATUS status;
} FLASH_DRIVER_STATE;

static FLASH_DRIVER_STATE flash_state = {0};

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
    (void)cb_event;

    fsp_err_t err = R_FLASH_HP_Open(&g_flash0_ctrl, &g_flash0_cfg);
    if (FSP_SUCCESS != err) {
        return ARM_DRIVER_ERROR;
    }

    flash_state.status.busy = 0;
    flash_state.status.error = 0;
    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_Uninitialize(void)
{
    R_FLASH_HP_Close(&g_flash0_ctrl);
    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_PowerControl(ARM_POWER_STATE state)
{
    switch (state) {
    case ARM_POWER_OFF:
    case ARM_POWER_FULL:
        return ARM_DRIVER_OK;
    case ARM_POWER_LOW:
    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
}

static int32_t ARM_Flash_ReadData(uint32_t addr, void *data, uint32_t cnt)
{
    if (addr >= FLASH_BASE_ADDRESS &&
        (addr + cnt) <= (FLASH_BASE_ADDRESS + FLASH_TOTAL_SIZE)) {
        memcpy(data, (void *)addr, cnt);
        return ARM_DRIVER_OK;
    }
    return ARM_DRIVER_ERROR_PARAMETER;
}

static int32_t ARM_Flash_ProgramData(uint32_t addr, const void *data, uint32_t cnt)
{
    if (!data || cnt == 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    flash_state.status.busy = 1;
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
    flash_state.status.busy = 1;

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

static ARM_FLASH_STATUS ARM_Flash_GetStatus(void)
{
    return flash_state.status;
}

static ARM_FLASH_INFO *ARM_Flash_GetInfo(void)
{
    return &FlashInfo;
}

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
/*
 * Data Flash Driver (Driver_FLASH1)
 * For OTP/NV Counters and ITS storage
 */

static ARM_FLASH_INFO DataFlashInfo = {
    .sector_info  = NULL,
    .sector_count = FLASH_DATA_FLASH_SIZE / FLASH_DATA_FLASH_SECTOR_SIZE,
    .sector_size  = FLASH_DATA_FLASH_SECTOR_SIZE,
    .page_size    = FLASH_DATA_FLASH_SECTOR_SIZE,
    .program_unit = 1,  /* 1 byte programming */
    .erased_value = 0xFF
};

static int32_t ARM_DataFlash_Initialize(ARM_Flash_SignalEvent_t cb_event)
{
    /* Data flash shares the same flash controller as code flash */
    return ARM_Flash_Initialize(cb_event);
}

static int32_t ARM_DataFlash_Uninitialize(void)
{
    /* Data flash shares initialization with code flash */
    return ARM_DRIVER_OK;
}

static int32_t ARM_DataFlash_ReadData(uint32_t addr, void *data, uint32_t cnt)
{
    /* Data flash base address */
    if (addr >= FLASH_DATA_FLASH_BASE &&
        (addr + cnt) <= (FLASH_DATA_FLASH_BASE + FLASH_DATA_FLASH_SIZE)) {
        memcpy(data, (void *)addr, cnt);
        return ARM_DRIVER_OK;
    }
    return ARM_DRIVER_ERROR_PARAMETER;
}

static int32_t ARM_DataFlash_ProgramData(uint32_t addr, const void *data, uint32_t cnt)
{
    if (!data || cnt == 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    
    /* Verify address is in data flash range */
    if (addr < FLASH_DATA_FLASH_BASE ||
        (addr + cnt) > (FLASH_DATA_FLASH_BASE + FLASH_DATA_FLASH_SIZE)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    flash_state.status.busy = 1;
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
    /* Verify address is in data flash range */
    if (addr < FLASH_DATA_FLASH_BASE ||
        addr >= (FLASH_DATA_FLASH_BASE + FLASH_DATA_FLASH_SIZE)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    flash_state.status.busy = 1;

    /* Data flash erase: 1 block = 64 bytes */
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
