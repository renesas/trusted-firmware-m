/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 * Copyright (c) 2018-2020 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * CMSIS Flash Driver for Renesas RA8M2 - MRAM, via FSP's r_mram.
 *
 * Two driver instances over ONE device:
 *
 *   Driver_FLASH0 - the whole 1 MB MRAM. BL2's flash map (the four MCUboot slots).
 *   Driver_FLASH1 - windowed onto DF_EMULATION only. ITS, PS and the NV counters.
 *
 * That is not the arrangement RA6E1/RA6M5 use, and the difference is the reason this file
 * needed rewriting rather than renaming. There, FLASH0 was code flash (r_flash_hp) and
 * FLASH1 was a genuinely separate data-flash device on the same controller. RA8M2 HAS NO
 * DATA FLASH (flash_layout.h), so FLASH1 is a second view of the same MRAM array, bounded to
 * the DF_EMULATION partition. Their address ranges therefore OVERLAP by design: FLASH0's
 * range spans the whole device, DF_EMULATION included. Nothing arbitrates that - BL2 only
 * ever hands FLASH0 offsets inside a slot, and DF_EMULATION is deliberately outside every
 * slot (asserted in ra8m2_layout_checks.c), so the two never write the same bytes.
 *
 * ============================ THE OFFSET / ADDRESS TRAP ============================
 *
 * The two instances are called with DIFFERENT conventions, and on RA6M5 that distinction was
 * invisible because its code flash is based at 0x00000000 so offsets and addresses were the
 * same number. On RA8M2 MRAM is based at 0x02000000 and they are 32 MB apart.
 *
 *   FLASH0 receives a flat OFFSET into the device. bl2/src/flash_map.c calls
 *     ReadData(area->fa_off + off, ...) where fa_off is FLASH_AREA_n_OFFSET, already
 *     alias-stripped by flash_layout.h. The same file forms an address for its DMA path as
 *     "FLASH_BASE_ADDRESS + area->fa_off + off" (flash_map.c:147), which settles it.
 *     => every FLASH0 entry point converts with MRAM_ADDR() before touching memory.
 *
 *   FLASH1 receives an absolute ADDRESS. its_flash_nor.c and
 *     flash_otp_nv_counters_backend.c pass the platform's own
 *     TFM_HAL_ITS_FLASH_AREA_ADDR / TFM_NV_COUNTERS_AREA_OFFSET, and flash_layout.h defines
 *     those from BSP_PARTITION_DF_EMULATION_START, which is 0x02010000.
 *     => FLASH1 uses the value as given.
 *
 * Getting FLASH0 wrong is not a fault, it is a wild access: offset 0x20000 without the base
 * lands in CPU0's secure DTCM.
 * ==================================================================================
 */

#include "Driver_Flash.h"
#include "flash_layout.h"
#include "r_mram.h"
#include "bsp_feature.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* The generated MRAM instance, declared directly rather than by including
 * ra_gen/common_data.h. That header includes FSP's own psa/crypto.h once the Crypto stack is
 * present in the e2 project, and would shadow TF-M's for every file in platform_s. The types
 * come from r_mram.h above; the definitions live in ra_gen/common_data.c, built as part of
 * the fsp_bsp module. (On RA6M5 the equivalent externs came from hal_data.c - FSP emits the
 * flash instance into common_data.c on this part.) */
extern mram_instance_ctrl_t g_mram0_ctrl;
extern const flash_cfg_t    g_mram0_cfg;

#ifndef ARG_UNUSED
#define ARG_UNUSED(arg)  (void)(arg)
#endif

/* Offset -> address, for FLASH0 only. See THE OFFSET / ADDRESS TRAP above. */
#define MRAM_ADDR(off)   ((uint32_t)(FLASH_BASE_ADDRESS) + (uint32_t)(off))

/* R_MRAM_Erase() counts blocks of BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES, i.e. 32 bytes -
 * NOT of FLASH_AREA_IMAGE_SECTOR_SIZE. mram_erase_blocks() writes 0xFF over
 * num_blocks * PROGRAMMING_SIZE bytes, and R_MRAM_InfoGet() reports the same 32 as
 * block_size. So an erase of one MCUboot sector is 0x8000 / 32 = 1024 blocks, and one
 * DF_EMULATION sector is 64 / 32 = 2.
 *
 * This is the same trap as the sector size in flash_layout.h from the other side: 32 is the
 * write granularity, 0x8000 is the MCUboot sector, and MRAM has no erase granularity of its
 * own - its "erase" is a normal programming pass writing the erased value. The RA6M5 driver
 * this came from computed its data-flash block count as SECTOR_SIZE / SECTOR_SIZE, which is
 * 1 and was right only because a data-flash block happened to equal its sector. */
#define MRAM_ERASE_BLOCK_SIZE    (BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES)
#define MRAM_BLOCKS_PER(sector)  ((sector) / MRAM_ERASE_BLOCK_SIZE)

#define ARM_FLASH_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,1)

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
 * implemented in BYTES throughout (memcpy(.., cnt), R_MRAM_Write(.., cnt), and the
 * addr + cnt range checks), so 8-bit items make items == bytes and the two agree.
 *
 * On RA6M5 this was briefly 2 (32-bit), which made every transfer move a quarter of the
 * requested data. It showed up as init_otp_nv_counters_flash() failing on every boot: it
 * reads the 4-byte init_value, the backend asked for 4/4 = 1 item, the driver copied 1 byte,
 * and the remaining three bytes were uninitialised stack - so the area never looked
 * initialised and the repair path then wrote at quarter size too. Kept at 0 here for the
 * same reason, not by inheritance.
 *
 * Honest as well as convenient: MRAM is memory-mapped and byte-readable. Write granularity
 * is a separate concern, carried by the *_PROGRAM_UNIT macros in flash_layout.h (32 for
 * both instances on this part), not by data_width.
 */
static const ARM_FLASH_CAPABILITIES DriverCapabilities = {
    0, /* event_ready */
    0, /* data_width = 0:8-bit, 1:16-bit, 2:32-bit - see above, must be 0 */
    1, /* erase_chip */
    0  /* reserved */
};

/* MRAM info, as BL2 sees it (Driver_FLASH0).
 *
 * program_unit is the MRAM write size, BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES = 32. It is
 * load-bearing in two places that must agree:
 *   - bl2/src/flash_map.c:422 returns it as flash_area_align(), which MCUboot uses to size
 *     and place the image trailer, and to pad every write it makes to a slot.
 *   - bl2/ext/mcuboot/CMakeLists.txt signs with --align ${MCUBOOT_ALIGN_VAL} --pad, so
 *     imgtool computes the same trailer geometry ahead of time.
 * config.cmake therefore sets MCUBOOT_ALIGN_VAL to 32 alongside this. Change the two
 * together or BL2 looks for the trailer magic where imgtool did not put it. FSP agrees
 * independently: its generated mcuboot_config.h sets MCUBOOT_BOOT_MAX_ALIGN from the same
 * BSP feature.
 *
 * 32 needs none of the special handling RA6M5's 128 did: it is inside the [1,2,4,8,16,32]
 * range that MCUboot's _trailer_size() and the bootutil_public.c _Static_assert accept, so
 * the wrapper.py and mcuboot_default_config.cmake list edits that port needed do not apply
 * here.
 *
 * sector_size is the MCUboot sector, which is NOT the same as program_unit - see
 * MRAM_ERASE_BLOCK_SIZE above and flash_layout.h.
 *
 * NOT YET EXERCISED ON HARDWARE. It only takes effect once BL2 WRITES MRAM, on an upgrade;
 * validate-and-boot reads only. Proving it needs a real two-version upgrade with a populated
 * secondary slot.
 */
static ARM_FLASH_INFO FlashInfo = {
    .sector_info  = NULL,
    .sector_count = FLASH_TOTAL_SIZE / FLASH_AREA_IMAGE_SECTOR_SIZE,
    .sector_size  = FLASH_AREA_IMAGE_SECTOR_SIZE,
    .page_size    = TFM_HAL_FLASH_PROGRAM_UNIT,
    .program_unit = TFM_HAL_FLASH_PROGRAM_UNIT,
    .erased_value = 0xFF
};

/* DF_EMULATION info, as ITS/PS/OTP see it (Driver_FLASH1).
 *
 * program_unit and page_size are DERIVED, not literals. ITS and PS both compare the
 * advertised program_unit against their compile-time TFM_HAL_*_PROGRAM_UNIT and refuse to
 * start on a mismatch - tfm_internal_trusted_storage.c:210 and :246, returning
 * PSA_ERROR_PROGRAMMER_ERROR (-129) out of init_its_fs_cfg(). On RA6M5 this said 1 while
 * flash_layout.h said 4, and ITS failed at spm_init_function() with PS to follow.
 *
 * 32 is the MRAM write size - the same as FLASH0's, because it is the same array. The
 * 64-byte figure is the LOGICAL sector the flash filesystem rotates on, which is
 * sector_size; MRAM imposes no erase block of its own, so 64 is a choice carried over from
 * RA6M5's data flash and must stay a multiple of 32.
 *
 * erased_value 0xFF matches mram_erase_blocks(), which writes UINT8_MAX bytes.
 */
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
/* Common                                                                    */
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

static int32_t ARM_Flash_PowerControl(ARM_POWER_STATE state)
{
    switch (state) {
    case ARM_POWER_FULL:
        /* MRAM is always powered. */
        return ARM_DRIVER_OK;
    case ARM_POWER_OFF:
    case ARM_POWER_LOW:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    default:
        return ARM_DRIVER_ERROR_PARAMETER;
    }
}

/* One controller serves both instances, so opening is shared and idempotent. */
static int32_t mram_open_once(void)
{
    if (flash_state.initialized) {
        return ARM_DRIVER_OK;
    }

    flash_state.status.busy  = 1;
    flash_state.status.error = 1;

    fsp_err_t err = R_MRAM_Open(&g_mram0_ctrl, &g_mram0_cfg);
    if (FSP_ERR_ALREADY_OPEN == err) {
        /* Close and reopen to reset state. */
        R_MRAM_Close(&g_mram0_ctrl);
        err = R_MRAM_Open(&g_mram0_ctrl, &g_mram0_cfg);
    }
    if (FSP_SUCCESS != err) {
        flash_state.status.busy = 0;
        return ARM_DRIVER_ERROR;
    }

    /* Validate that the driver and this file agree on the write granularity.
     *
     * R_MRAM_InfoGet() reports block_size = BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES for its
     * single code region. Compare against the PROGRAM UNIT, not the MCUboot sector size -
     * the RA6M5 driver compared the reported block size against FLASH_AREA_IMAGE_SECTOR_SIZE,
     * which was right there only because its code-flash region-1 erase block and its MCUboot
     * sector were both 32 KB. Here they are 32 and 0x8000, so that test would always fail. */
    flash_info_t info;
    if (FSP_SUCCESS != R_MRAM_InfoGet(&g_mram0_ctrl, &info)) {
        R_MRAM_Close(&g_mram0_ctrl);
        flash_state.status.busy = 0;
        return ARM_DRIVER_ERROR;
    }
    if (0U == info.code_flash.num_regions) {
        R_MRAM_Close(&g_mram0_ctrl);
        flash_state.status.busy = 0;
        return ARM_DRIVER_ERROR;
    }
    if (info.code_flash.p_block_array[0].block_size != MRAM_ERASE_BLOCK_SIZE) {
        R_MRAM_Close(&g_mram0_ctrl);
        flash_state.status.busy = 0;
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    flash_state.status.busy  = 0;
    flash_state.status.error = 0;
    flash_state.initialized  = 1;
    return ARM_DRIVER_OK;
}

/* Shared write/erase bodies. `addr` is an ABSOLUTE address by the time it reaches these. */
static int32_t mram_program(uint32_t addr, const void *data, uint32_t cnt)
{
    if (!flash_state.initialized) {
        return ARM_DRIVER_ERROR;
    }

    flash_state.status.busy  = 1;
    flash_state.status.error = 0;

    fsp_err_t err = R_MRAM_Write(&g_mram0_ctrl, (uint32_t)data, addr, cnt);

    flash_state.status.busy = 0;
    if (FSP_SUCCESS != err) {
        flash_state.status.error = 1;
        return ARM_DRIVER_ERROR;
    }
    return ARM_DRIVER_OK;
}

static int32_t mram_erase(uint32_t addr, uint32_t sector_size)
{
    if (!flash_state.initialized) {
        return ARM_DRIVER_ERROR;
    }

    flash_state.status.busy  = 1;
    flash_state.status.error = 0;

    fsp_err_t err = R_MRAM_Erase(&g_mram0_ctrl, addr, MRAM_BLOCKS_PER(sector_size));

    flash_state.status.busy = 0;
    if (FSP_SUCCESS != err) {
        flash_state.status.error = 1;
        return ARM_DRIVER_ERROR;
    }
    return ARM_DRIVER_OK;
}

/*===========================================================================*/
/* Driver_FLASH0 - whole MRAM, addressed by OFFSET (BL2's flash map)         */
/*===========================================================================*/

static int32_t ARM_Flash_Initialize(ARM_Flash_SignalEvent_t cb_event)
{
    ARG_UNUSED(cb_event);
    return mram_open_once();
}

static int32_t ARM_Flash_Uninitialize(void)
{
    if (flash_state.initialized) {
        R_MRAM_Close(&g_mram0_ctrl);
        flash_state.initialized = 0;
    }
    return ARM_DRIVER_OK;
}

/* Offsets are bounded by the device SIZE, not by an address range - see the trap note. */
static bool flash0_range_ok(uint32_t off, uint32_t cnt)
{
    return (cnt <= FLASH_TOTAL_SIZE) && (off <= (FLASH_TOTAL_SIZE - cnt));
}

static int32_t ARM_Flash_ReadData(uint32_t addr, void *data, uint32_t cnt)
{
    if (!data || cnt == 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if (!flash0_range_ok(addr, cnt)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    flash_state.status.busy = 1;
    memcpy(data, (const void *)MRAM_ADDR(addr), cnt);
    flash_state.status.busy = 0;

    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_ProgramData(uint32_t addr, const void *data, uint32_t cnt)
{
    if (!data || cnt == 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if (!flash0_range_ok(addr, cnt)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    return mram_program(MRAM_ADDR(addr), data, cnt);
}

static int32_t ARM_Flash_EraseSector(uint32_t addr)
{
    if (!flash0_range_ok(addr, FLASH_AREA_IMAGE_SECTOR_SIZE) ||
        (addr % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0U) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    return mram_erase(MRAM_ADDR(addr), FLASH_AREA_IMAGE_SECTOR_SIZE);
}

static int32_t ARM_Flash_EraseChip(void)
{
    /* Erases the WHOLE device, DF_EMULATION and BL2 included. Present because
     * DriverCapabilities advertises erase_chip and nothing in this port calls it; if
     * something ever does, note that it destroys ITS, PS and the NV counters too. */
    for (uint32_t off = 0U; off < FLASH_TOTAL_SIZE; off += FLASH_AREA_IMAGE_SECTOR_SIZE) {
        if (ARM_Flash_EraseSector(off) != ARM_DRIVER_OK) {
            return ARM_DRIVER_ERROR;
        }
    }
    return ARM_DRIVER_OK;
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

/*===========================================================================*/
/* Driver_FLASH1 - DF_EMULATION window, addressed by ABSOLUTE ADDRESS        */
/* ITS, PS and the MCUboot NV counters                                       */
/*===========================================================================*/

static int32_t ARM_DataFlash_Initialize(ARM_Flash_SignalEvent_t cb_event)
{
    ARG_UNUSED(cb_event);
    /* Same controller and same array as FLASH0. */
    return mram_open_once();
}

static int32_t ARM_DataFlash_Uninitialize(void)
{
    /* Deliberately does NOT close: FLASH0 may still be using the controller, and on this
     * part they are the same device rather than two peripherals. */
    return ARM_DRIVER_OK;
}

static bool flash1_range_ok(uint32_t addr, uint32_t cnt)
{
    return (addr >= (uint32_t)FLASH_DATA_FLASH_BASE) &&
           (cnt <= (uint32_t)FLASH_DATA_FLASH_SIZE) &&
           (addr <= ((uint32_t)FLASH_DATA_FLASH_BASE + (uint32_t)FLASH_DATA_FLASH_SIZE - cnt));
}

static int32_t ARM_DataFlash_ReadData(uint32_t addr, void *data, uint32_t cnt)
{
    if (!data || cnt == 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if (!flash1_range_ok(addr, cnt)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    flash_state.status.busy = 1;
    memcpy(data, (const void *)addr, cnt);
    flash_state.status.busy = 0;

    return ARM_DRIVER_OK;
}

static int32_t ARM_DataFlash_ProgramData(uint32_t addr, const void *data, uint32_t cnt)
{
    if (!data || cnt == 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if (!flash1_range_ok(addr, cnt)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    return mram_program(addr, data, cnt);
}

static int32_t ARM_DataFlash_EraseSector(uint32_t addr)
{
    if (!flash1_range_ok(addr, FLASH_DATA_FLASH_SECTOR_SIZE) ||
        (addr % FLASH_DATA_FLASH_SECTOR_SIZE) != 0U) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    return mram_erase(addr, FLASH_DATA_FLASH_SECTOR_SIZE);
}

static int32_t ARM_DataFlash_EraseChip(void)
{
    /* DF_EMULATION only - never the whole device. */
    for (uint32_t addr = (uint32_t)FLASH_DATA_FLASH_BASE;
         addr < ((uint32_t)FLASH_DATA_FLASH_BASE + (uint32_t)FLASH_DATA_FLASH_SIZE);
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
