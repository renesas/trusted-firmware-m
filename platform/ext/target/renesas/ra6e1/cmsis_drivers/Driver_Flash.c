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
#include "r_flash_hp.h"
#include "bsp_feature.h"
#include <string.h>

/* The two generated instance objects this driver drives, declared directly rather than by
 * including ra_gen/hal_data.h. That header pulls ra_gen/common_data.h, which - once the
 * Crypto stack is present in the e2 project - includes FSP's own psa/crypto.h and would
 * shadow TF-M's for every file in platform_s. Their types come from r_flash_hp.h above.
 * The definitions live in ra_gen/hal_data.c, built as part of the fsp_bsp module. */
extern flash_hp_instance_ctrl_t g_flash0_ctrl;
extern const flash_cfg_t        g_flash0_cfg;

#ifndef ARG_UNUSED
#define ARG_UNUSED(arg)  (void)(arg)
#endif

/* RA6M4 Flash HP code-flash block size for the MCUboot-managed region.
 * Region 0 (0x0-0xFFFF) uses 8KB blocks; region 1 (0x10000+) uses 32KB blocks.
 * All MCUboot slots are in region 1, so use the region-1 (32KB) block size -
 * this must match FLASH_AREA_IMAGE_SECTOR_SIZE so erases are full 32KB blocks. */
#define FLASH_HP_BLOCK_SIZE  BSP_FEATURE_FLASH_HP_CF_REGION1_BLOCK_SIZE

#define ARM_FLASH_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,1) /* Enhanced version */

/* Driver version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_FLASH_API_VERSION,
    ARM_FLASH_DRV_VERSION
};

/* Driver capabilities.
 *
 * data_width MUST stay 0 (8-bit). In CMSIS the `cnt` argument of ReadData/ProgramData is a
 * count of DATA ITEMS of data_width bytes, not a byte count, and every TF-M consumer divides
 * a byte length by data_width before calling - bl2/src/flash_map.c, its_flash_nor.c,
 * its_flash_nand.c and flash_otp_nv_counters_backend.c all do. The functions below are
 * implemented in BYTES throughout (memcpy(.., cnt), R_FLASH_HP_Write(.., cnt), and the
 * addr + cnt range checks), so 8-bit items make items == bytes and the two agree.
 *
 * This was 2 (32-bit), which made every transfer move a quarter of the requested data. It
 * showed up as init_otp_nv_counters_flash() failing on every boot: it reads the 4-byte
 * init_value, the backend asked for 4/4 = 1 item, the driver copied 1 byte, and the
 * remaining three bytes were uninitialised stack - so the area never looked initialised and
 * the repair path then wrote at quarter size too.
 *
 * Honest as well as convenient: RA flash is memory-mapped and byte-readable. Write
 * granularity is a separate concern and is carried by the *_PROGRAM_UNIT macros in
 * flash_layout.h (4 for data flash, 128 for code flash), not by data_width.
 */
static const ARM_FLASH_CAPABILITIES DriverCapabilities = {
    0, /* event_ready */
    0, /* data_width = 0:8-bit, 1:16-bit, 2:32-bit - see above, must be 0 */
    1, /* erase_chip */
    0  /* reserved */
};

/* Code flash info structure.
 *
 * program_unit is the real 128-byte RA6E1 code flash write size
 * (BSP_FEATURE_FLASH_HP_CF_WRITE_SIZE, bsp_feature.h:300). It was 1 until 2026-08-29.
 *
 * This value is load-bearing in two places that must agree:
 *   - bl2/src/flash_map.c:422 returns it as flash_area_align(), which MCUboot uses to size
 *     and place the image trailer, and to pad every write it makes to a slot.
 *   - bl2/ext/mcuboot/CMakeLists.txt:204 signs with --align ${MCUBOOT_ALIGN_VAL} --pad, so
 *     imgtool computes the same trailer geometry ahead of time.
 * config.cmake therefore sets MCUBOOT_ALIGN_VAL to 128 alongside this. Change the two
 * together or BL2 looks for the trailer magic where imgtool did not put it.
 *
 * Raising it needed three list edits, because 128 was outside the range TF-M's signing
 * path would accept - see scripts/wrapper/wrapper.py and mcuboot_default_config.cmake for
 * the reasoning. Note the constraint is NOT imgtool's own --align choice list: wrapper.py
 * builds imgtool.image.Image() directly and never invokes imgtool's CLI, and Image() only
 * requires a power of two. For OVERWRITE_ONLY the trailer is
 * max_align*2 + align_up(16, max_align) = 384 bytes, and _trailer_size()'s
 * "write_size not in [1,2,4,8,16,32]" rejection sits on the swap path, which this port
 * does not use.
 *
 * On the runtime side mcuboot already handles a write unit larger than the 16-byte magic:
 * boot_write_trailer() pads from ALIGN_DOWN(off, BOOT_MAX_ALIGN), and the >=8 && <=32
 * _Static_assert in bootutil_public.c is guarded on the SWAP modes only.
 *
 * NOT YET EXERCISED ON HARDWARE. It only takes effect once BL2 WRITES code flash, on an
 * upgrade; validate-and-boot reads only, which is why the port booted fine while this was
 * still 1. Proving it needs a real two-version upgrade with a populated secondary slot. */
static ARM_FLASH_INFO FlashInfo = {
    .sector_info  = NULL,
    .sector_count = FLASH_TOTAL_SIZE / FLASH_AREA_IMAGE_SECTOR_SIZE,
    .sector_size  = FLASH_AREA_IMAGE_SECTOR_SIZE,
    .page_size    = 4,
    .program_unit = TFM_HAL_CODE_FLASH_PROGRAM_UNIT,
    .erased_value = 0xFF
};

/* Data flash info structure.
 *
 * program_unit and page_size are DERIVED, not literals. ITS and PS both compare the
 * advertised program_unit against their compile-time TFM_HAL_*_PROGRAM_UNIT and refuse to
 * start on a mismatch - tfm_internal_trusted_storage.c:210 and :246, returning
 * PSA_ERROR_PROGRAMMER_ERROR (-129) out of init_its_fs_cfg(). This said 1 while
 * flash_layout.h said 4, so ITS failed at spm_init_function() and PS would have followed.
 *
 * 4 is the RA6E1 data flash write size (BSP_FEATURE_FLASH_HP_DF_WRITE_SIZE); the 64-byte
 * figure is the ERASE block, which is sector_size. page_size is the programming page, so
 * it is the write size too - not the erase size it used to hold. */
static ARM_FLASH_INFO DataFlashInfo = {
    .sector_info  = NULL,
    .sector_count = FLASH_DATA_FLASH_SIZE / FLASH_DATA_FLASH_SECTOR_SIZE,
    .sector_size  = FLASH_DATA_FLASH_SECTOR_SIZE,
    .page_size    = TFM_HAL_DATA_FLASH_PROGRAM_UNIT,
    .program_unit = TFM_HAL_DATA_FLASH_PROGRAM_UNIT,
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
