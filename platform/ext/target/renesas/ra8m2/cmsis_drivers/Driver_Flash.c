/*
 * Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
 * Copyright (c) 2018-2020 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * CMSIS flash driver for Renesas RA8M2 MRAM, over FSP's r_mram.
 *
 * Two driver instances over one device:
 *
 *   Driver_FLASH0 - the whole 1 MB MRAM. BL2's flash map (the four MCUboot slots).
 *   Driver_FLASH1 - a window onto DF_EMULATION only. ITS, PS and the NV counters.
 *
 * RA8M2 has no data flash (flash_layout.h), so FLASH1 is a second view of the same MRAM
 * array, bounded to the DF_EMULATION partition; on RA6M5 FLASH1 is a separate data-flash
 * device. The two address ranges therefore overlap by design: FLASH0 spans the whole
 * device, DF_EMULATION included. Nothing arbitrates them. BL2 only passes FLASH0 offsets
 * inside a slot, and DF_EMULATION is outside every slot (asserted in
 * ra8m2_layout_checks.c), so the two never write the same bytes.
 *
 * ================================ Offset/address trap ================================
 *
 * The two instances take different arguments. MRAM is based at 0x02000000, so an offset
 * and its address are 32 MB apart; on RA6M5 code flash is based at 0x00000000 and the two
 * are the same number.
 *
 *   FLASH0 receives a flat offset into the device. bl2/src/flash_map.c calls
 *     ReadData(area->fa_off + off, ...), where fa_off is FLASH_AREA_n_OFFSET, already
 *     alias-stripped by flash_layout.h. The same file forms an address for its DMA path as
 *     "FLASH_BASE_ADDRESS + area->fa_off + off" (flash_map.c:147), which confirms it.
 *     => every FLASH0 entry point converts with MRAM_ADDR() before touching memory.
 *
 *   FLASH1 receives an absolute address. its_flash_nor.c and
 *     flash_otp_nv_counters_backend.c pass the platform's own
 *     TFM_HAL_ITS_FLASH_AREA_ADDR / TFM_NV_COUNTERS_AREA_OFFSET, and flash_layout.h defines
 *     those from BSP_PARTITION_DF_EMULATION_START, which is 0x02010000.
 *     => FLASH1 uses the value as given.
 *
 * A FLASH0 offset used with no base is a wild access, not a diagnostic: offset 0x20000
 * lands in CPU0's secure DTCM. The right base with the wrong alias is a HardFault instead,
 * and only on a partitioned part; see MRAM_ADDR() below.
 * ===================================================================================
 */

#include "Driver_Flash.h"
#include "flash_layout.h"
#include "r_mram.h"
#include "bsp_feature.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* The generated MRAM instance, declared here instead of by including
 * ra_gen/common_data.h. That header includes FSP's own psa/crypto.h once the Crypto stack
 * is in the e2 project, and would shadow TF-M's for every file in platform_s. The types
 * come from r_mram.h above; the definitions are in ra_gen/common_data.c, built as part of
 * the fsp_bsp module. FSP emits the flash instance into common_data.c on this part, not
 * hal_data.c as on RA6M5. */
extern mram_instance_ctrl_t g_mram0_ctrl;
extern const flash_cfg_t    g_mram0_cfg;

#ifndef ARG_UNUSED
#define ARG_UNUSED(arg)  (void)(arg)
#endif

/* Offset to address, for FLASH0 only. See "Offset/address trap" in the file header.
 *
 * The alias is part of the address, and it is not constant across the device.
 *
 * MRAM answers at two bases, 0x02000000 secure and 0x12000000 non-secure,
 * BSP_FEATURE_TZ_NS_OFFSET apart. The TrustZone boundary programmed with the Device
 * Partition Manager decides which alias reaches a byte: below it, the secure alias only; at
 * or above it, the non-secure alias only. Only the offset is alias-independent, which is
 * what lets one Driver_FLASH0 serve all four slots.
 *
 * Adding FLASH_BASE_ADDRESS unconditionally is correct for the secure areas and wrong for
 * the non-secure ones, and the error shows only on a partitioned part. On a virgin device
 * all of MRAM is secure, so the secure alias reaches everything. Once RDPM is programmed,
 * BL2 takes a HardFault in memcpy() on the first access at or above 0x020B0000: the NS
 * primary slot, written by the programmer at 0x120B0000 and read through an alias that does
 * not answer there. D073.
 *
 * RA6M5 has BSP_FEATURE_TZ_NS_OFFSET = 0 and one alias, so its driver adds a single base.
 *
 * Two constants, from two sources; neither substitutes for the other:
 *
 *   The delta between the aliases is BSP_FEATURE_TZ_NS_OFFSET, 0x10000000. It is a property
 *   of the device family, taken from FSP rather than restated. flash_layout.h spells it as
 *   the literal FLASH_NS_ALIAS_BASE because that header is preprocessed into ra8m2_bl2.ld
 *   and cannot reach bsp_feature.h; ra8m2_layout_checks.c asserts that the two agree.
 *
 *   The boundary is where secure MRAM ends. It is a property of this partitioning, not of
 *   the family: whatever RDPM programmed into CFSAMONA. It must move when the layout moves,
 *   or the driver picks the wrong alias. It is taken as the end of the NSC region, which is
 *   the boundary by definition here: RDPM can only express a contiguous Secure|NSC|NS
 *   triple, so the NSC is always the last secure region. ra8m2_layout_checks.c asserts
 *   that, and that no secure area sits above it.
 *
 *   FLASH_AREA_1_OFFSET gives the same number today but names the first non-secure slot,
 *   which coincides with the boundary only while the areas keep their order. D074. */
#define MRAM_NS_BASE                                                     \
    ((uint32_t)(FLASH_BASE_ADDRESS) + (uint32_t)(BSP_FEATURE_TZ_NS_OFFSET))

#define MRAM_NS_BOUNDARY                                                 \
    ((uint32_t)(TFM_MRAM_S_OFF(BSP_PARTITION_FLASH_CPU0_C_START)) +      \
     (uint32_t)(BSP_PARTITION_FLASH_CPU0_C_SIZE))

#define MRAM_ADDR(off)                                                   \
    ((uint32_t)(off) +                                                   \
     ((uint32_t)(off) >= MRAM_NS_BOUNDARY ? MRAM_NS_BASE                 \
                                          : (uint32_t)(FLASH_BASE_ADDRESS)))

/* R_MRAM_Erase() counts blocks of BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES, 32 bytes, not
 * of FLASH_AREA_IMAGE_SECTOR_SIZE. mram_erase_blocks() writes 0xFF over
 * num_blocks * PROGRAMMING_SIZE bytes, and R_MRAM_InfoGet() reports the same 32 as
 * block_size. One MCUboot sector is therefore 0x8000 / 32 = 1024 blocks, and one
 * DF_EMULATION sector is 64 / 32 = 2.
 *
 * None of the three sizes here is the write granularity, which is one byte
 * (mram_write_data() copies a byte at a time; see flash_layout.h):
 *   32      - the programming buffer / "erase" block, what R_MRAM_Erase counts
 *   0x8000  - the MCUboot sector, FSP's RM_MCUBOOT_MRAM_BLOCK_SIZE
 *   64      - DF_EMULATION's logical FS sector
 * MRAM has no erase granularity in the flash sense; its "erase" is a normal programming
 * pass that writes the erased value. */
#define MRAM_ERASE_BLOCK_SIZE    (BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES)
#define MRAM_BLOCKS_PER(sector)  ((sector) / MRAM_ERASE_BLOCK_SIZE)

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
 * throughout (R_MRAM_Write(.., cnt), memcpy(.., cnt) and the addr + cnt range checks), so
 * only 8-bit items keep items and bytes equal. A wider setting makes every transfer move a
 * fraction of the requested data; on RA6M5, 32-bit made init_otp_nv_counters_flash() fail
 * on every boot.
 *
 * MRAM is memory-mapped and byte-readable, so 8-bit is also accurate. Write granularity is
 * carried by the *_PROGRAM_UNIT macros in flash_layout.h (32 for FLASH0 because FSP fixes
 * MCUboot's alignment there, 4 for FLASH1 so ITS and PS stay off the NAND path), not by
 * data_width.
 */
static const ARM_FLASH_CAPABILITIES DriverCapabilities = {
    0, /* event_ready */
    0, /* data_width = 0:8-bit, 1:16-bit, 2:32-bit - see above, must be 0 */
    1, /* erase_chip */
    0  /* reserved */
};

/* MRAM info as BL2 sees it (Driver_FLASH0).
 *
 * program_unit is the MRAM programming size, BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES = 32,
 * not the write granularity (see MRAM_ERASE_BLOCK_SIZE above). Two places depend on it and
 * must agree:
 *   - bl2/src/flash_map.c:422 returns it as flash_area_align(), which MCUboot uses to size
 *     and place the image trailer, and to pad every write it makes to a slot.
 *   - bl2/ext/mcuboot/CMakeLists.txt signs with --align ${MCUBOOT_ALIGN_VAL} --pad, so
 *     imgtool computes the same trailer geometry ahead of time.
 * config.cmake therefore sets MCUBOOT_ALIGN_VAL to 32. Change the two together, or BL2
 * looks for the trailer magic where imgtool did not put it. FSP's generated
 * mcuboot_config.h sets MCUBOOT_BOOT_MAX_ALIGN from the same BSP feature.
 *
 * 32 is inside the [1,2,4,8,16,32] range that MCUboot's _trailer_size() and the
 * bootutil_public.c _Static_assert accept, so the wrapper.py and
 * mcuboot_default_config.cmake list edits that RA6M5's 128 needs do not apply here.
 *
 * sector_size is the MCUboot sector, which differs from program_unit; see
 * MRAM_ERASE_BLOCK_SIZE above and flash_layout.h.
 *
 * BL2 writes MRAM only during an upgrade; validate-and-boot only reads. The upgrade path
 * has run on this part (TEST_RESULTS.md §4, D100).
 */
static ARM_FLASH_INFO FlashInfo = {
    .sector_info  = NULL,
    .sector_count = FLASH_TOTAL_SIZE / FLASH_AREA_IMAGE_SECTOR_SIZE,
    .sector_size  = FLASH_AREA_IMAGE_SECTOR_SIZE,
    .page_size    = TFM_HAL_FLASH_PROGRAM_UNIT,
    .program_unit = TFM_HAL_FLASH_PROGRAM_UNIT,
    .erased_value = 0xFF
};

/* DF_EMULATION info as ITS, PS and OTP see it (Driver_FLASH1).
 *
 * program_unit and page_size are derived, not literals. ITS and PS compare the advertised
 * program_unit with their compile-time TFM_HAL_*_PROGRAM_UNIT and refuse to start on a
 * mismatch: tfm_internal_trusted_storage.c:210 and :246 return PSA_ERROR_PROGRAMMER_ERROR
 * (-129) from init_its_fs_cfg().
 *
 * The value is 4, not FLASH0's 32, although the array is the same: MRAM writes single bytes,
 * and the two consumers want different things. MCUboot takes 32 because FSP's generated
 * mcuboot_config.h says so. ITS and PS take 4 because above 16 its_flash.c switches to a
 * NAND emulation that allocates about 62 KB of RAM buffers for no benefit. See
 * TFM_HAL_DATA_FLASH_PROGRAM_UNIT in flash_layout.h.
 *
 * The 64-byte sector_size is the logical sector the flash filesystem rotates on. MRAM has
 * no erase granularity of its own, so 64 is carried over from RA6M5's data flash and must
 * stay a multiple of the program unit.
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

    /* Check that the driver and this file agree on the erase block.
     *
     * R_MRAM_InfoGet() reports block_size = BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES for its
     * single code region. Compare it with MRAM_ERASE_BLOCK_SIZE, not the MCUboot sector
     * size: here they are 32 and 0x8000. The RA6M5 driver compares with the sector size,
     * which works there only because its region-1 erase block and its MCUboot sector are
     * both 32 KB. */
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

/* Shared write/erase bodies. `addr` is an absolute address by the time it reaches these. */
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

/* Offsets are bounded by the device size, not by an address range; see the trap note. */
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
    /* WARNING: erases the whole device, BL2 and DF_EMULATION included, so ITS, PS and the
     * NV counters are lost too. It exists because DriverCapabilities advertises erase_chip;
     * nothing in this port calls it. */
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
    /* Does not close: FLASH0 may still use the controller, and on this part the two are
     * the same device, not two peripherals. */
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
