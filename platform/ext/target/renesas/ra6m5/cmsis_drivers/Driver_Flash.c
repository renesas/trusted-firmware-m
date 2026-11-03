/*
 * Copyright (c) 2025 Renesas Electronics Corporation. All rights reserved.
 * Copyright (c) 2018-2020 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * CMSIS flash driver for Renesas RA6M5 over FSP Flash HP, derived from the RA8 TF-M port.
 */

#include "Driver_Flash.h"
#include "flash_layout.h"
#include "r_flash_hp.h"
#include "bsp_feature.h"
#include <string.h>

/* The two generated instance objects this driver uses, declared here instead of by
 * including ra_gen/hal_data.h. That header pulls in ra_gen/common_data.h, which includes
 * FSP's own psa/crypto.h once the Crypto stack is in the e2 project, and would shadow
 * TF-M's for every file in platform_s. The types come from r_flash_hp.h above; the
 * definitions are in ra_gen/hal_data.c, built as part of the fsp_bsp module. */
extern flash_hp_instance_ctrl_t g_flash0_ctrl;
extern const flash_cfg_t        g_flash0_cfg;

#ifndef ARG_UNUSED
#define ARG_UNUSED(arg)  (void)(arg)
#endif

/* RA6M5 code-flash block size for the MCUboot-managed region. Region 0 (0x0-0xFFFF) uses
 * 8KB blocks and region 1 (0x10000+) uses 32KB blocks. All MCUboot slots are in region 1,
 * so this is the region-1 (32KB) size. It must match FLASH_AREA_IMAGE_SECTOR_SIZE so that
 * erases are whole 32KB blocks. */
#define FLASH_HP_BLOCK_SIZE  BSP_FEATURE_FLASH_HP_CF_REGION1_BLOCK_SIZE

#define ARM_FLASH_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,1)

static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_FLASH_API_VERSION,
    ARM_FLASH_DRV_VERSION
};

/* Driver capabilities.
 *
 * data_width must stay 0 (8-bit). In CMSIS the `cnt` argument of ReadData/ProgramData
 * counts data items of data_width bytes, not bytes, and every TF-M consumer divides a byte
 * length by data_width before calling: bl2/src/flash_map.c, its_flash_nor.c,
 * its_flash_nand.c and flash_otp_nv_counters_backend.c. The functions below work in bytes
 * throughout (R_FLASH_HP_Write(.., cnt), memcpy(.., cnt) and the addr + cnt range checks),
 * so only 8-bit items keep items and bytes equal. A wider setting makes every transfer move
 * a fraction of the requested data; at 32-bit, init_otp_nv_counters_flash() fails on every
 * boot.
 *
 * RA flash is memory-mapped and byte-readable, so 8-bit is also accurate. Write granularity
 * is carried by the *_PROGRAM_UNIT macros in flash_layout.h (4 for data flash, 128 for code
 * flash), not by data_width.
 */
static const ARM_FLASH_CAPABILITIES DriverCapabilities = {
    0, /* event_ready */
    0, /* data_width = 0:8-bit, 1:16-bit, 2:32-bit - see above, must be 0 */
    1, /* erase_chip */
    0  /* reserved */
};

/* Code flash info.
 *
 * program_unit is the RA6M5 code flash write size, 128 bytes
 * (BSP_FEATURE_FLASH_HP_CF_WRITE_SIZE, bsp_feature.h:300). Two places depend on it and
 * must agree:
 *   - bl2/src/flash_map.c:422 returns it as flash_area_align(), which MCUboot uses to size
 *     and place the image trailer, and to pad every write it makes to a slot.
 *   - bl2/ext/mcuboot/CMakeLists.txt:204 signs with --align ${MCUBOOT_ALIGN_VAL} --pad, so
 *     imgtool computes the same trailer geometry ahead of time.
 * config.cmake therefore sets MCUBOOT_ALIGN_VAL to 128. Change the two together, or BL2
 * looks for the trailer magic where imgtool did not put it.
 *
 * TF-M's signing path accepts 128 only through three list edits; see
 * scripts/wrapper/wrapper.py and mcuboot_default_config.cmake for the reasoning. imgtool's
 * own --align choice list is not the constraint: wrapper.py builds imgtool.image.Image()
 * directly, which requires only a power of two. For OVERWRITE_ONLY the trailer is
 * max_align*2 + align_up(16, max_align) = 384 bytes. The "write_size not in
 * [1,2,4,8,16,32]" rejection in _trailer_size() is on the swap path, which this port does
 * not use.
 *
 * At runtime MCUboot handles a write unit larger than the 16-byte magic:
 * boot_write_trailer() pads from ALIGN_DOWN(off, BOOT_MAX_ALIGN), and the >=8 && <=32
 * _Static_assert in bootutil_public.c applies to the swap modes only.
 *
 * NOTE: not yet exercised on hardware. The value matters only when BL2 writes code flash,
 * during an upgrade; validate-and-boot only reads. Proving it needs a two-version upgrade
 * with a populated secondary slot. */
static ARM_FLASH_INFO FlashInfo = {
    .sector_info  = NULL,
    .sector_count = FLASH_TOTAL_SIZE / FLASH_AREA_IMAGE_SECTOR_SIZE,
    .sector_size  = FLASH_AREA_IMAGE_SECTOR_SIZE,
    .page_size    = 4,
    .program_unit = TFM_HAL_CODE_FLASH_PROGRAM_UNIT,
    .erased_value = 0xFF
};

/* Data flash info.
 *
 * program_unit and page_size are derived, not literals. ITS and PS compare the advertised
 * program_unit with their compile-time TFM_HAL_*_PROGRAM_UNIT and refuse to start on a
 * mismatch: tfm_internal_trusted_storage.c:210 and :246 return PSA_ERROR_PROGRAMMER_ERROR
 * (-129) from init_its_fs_cfg().
 *
 * 4 is the RA6M5 data flash write size (BSP_FEATURE_FLASH_HP_DF_WRITE_SIZE). The 64-byte
 * figure is the erase block, which is sector_size. page_size is the programming page, so
 * it is also the write size, not the erase size. */
static ARM_FLASH_INFO DataFlashInfo = {
    .sector_info  = NULL,
    .sector_count = FLASH_DATA_FLASH_SIZE / FLASH_DATA_FLASH_SECTOR_SIZE,
    .sector_size  = FLASH_DATA_FLASH_SECTOR_SIZE,
    .page_size    = TFM_HAL_DATA_FLASH_PROGRAM_UNIT,
    .program_unit = TFM_HAL_DATA_FLASH_PROGRAM_UNIT,
    .erased_value = 0xFF
};

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

    /* Check the last code-flash region's block size against the MCUboot sector size.
     * RA6M5 code flash: 2 MB total; 8 KB blocks in region 0, 32 KB in region 1. */
    uint32_t page_size = info.code_flash.p_block_array[info.code_flash.num_regions - 1U].block_size;

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
        /* Flash is always powered in RA6M5 */
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
    /* Does not close: data flash shares the controller with code flash, which may still
     * need it. */
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
