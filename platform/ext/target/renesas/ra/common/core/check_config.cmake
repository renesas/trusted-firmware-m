#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Configure-time validation of the RA flash/RAM map declared in config.cmake.
#
# Each board's check_config.cmake (<board>/check_config.cmake) will also have their specific
# check logic and will include its SoC-specific check_config.cmake.
#
# Each SoC's check_config.cmake (common/<soc>/check_config.cmake) will have their specific
# check logic for their specific hardware, and will include this common check file.

if(NOT RA_CM85_NS_PRESENT)
    message(FATAL_ERROR "${RA_SOC_NAME}: RA_CM85_NS_PRESENT OFF is not supported.")
endif()

if(DEFINED MCUBOOT_IMAGE_NUMBER AND MCUBOOT_IMAGE_NUMBER GREATER 3)
    message(FATAL_ERROR "${RA_SOC_NAME}: only MCUBOOT_IMAGE_NUMBER 1, 2 and 3 are supported.")
endif()

# The MCUboot options come from the SPE build; the NS build does not set them.
if(BL2 AND S_BUILD)
    # With the CPU1 image, S, CPU0 NS and CPU1 NS are separate images.
    if(RA_CM33_NS_PRESENT)
        set(_ra_expected_images 3)
        set(_ra_cpu1_msg ", CPU1 NS")
    else()
        set(_ra_expected_images 2)
        set(_ra_cpu1_msg "")
    endif()
    if(NOT MCUBOOT_IMAGE_NUMBER EQUAL _ra_expected_images)
        message(FATAL_ERROR "${RA_SOC_NAME}: BL2 requires MCUBOOT_IMAGE_NUMBER ${_ra_expected_images} (separate S, CPU0 NS${_ra_cpu1_msg} images), got ${MCUBOOT_IMAGE_NUMBER}.")
    endif()
    # Secondary slots live in MRAM next to the primary ones and MRAM has no
    # scratch area / bank swap: only overwrite upgrades are supported.
    if(NOT MCUBOOT_UPGRADE_STRATEGY STREQUAL "OVERWRITE_ONLY")
        message(FATAL_ERROR "${RA_SOC_NAME}: BL2 supports only MCUBOOT_UPGRADE_STRATEGY=OVERWRITE_ONLY (got ${MCUBOOT_UPGRADE_STRATEGY}).")
    endif()
    # Image trailer writes must be a multiple of the MRAM program unit.
    math(EXPR _align_rem "${MCUBOOT_ALIGN_VAL} % ${FLASH_PROGRAM_UNIT}")
    if(MCUBOOT_ALIGN_VAL LESS FLASH_PROGRAM_UNIT OR NOT _align_rem EQUAL 0)
        message(FATAL_ERROR "${RA_SOC_NAME}: MCUBOOT_ALIGN_VAL (${MCUBOOT_ALIGN_VAL}) must be a multiple of FLASH_PROGRAM_UNIT (${FLASH_PROGRAM_UNIT}).")
    endif()
    # The default MCUboot flash map handles at most 2 images; the platform
    # provides its own map (common/core/bl2/flash_map_bl2.c).
    if(DEFAULT_MCUBOOT_FLASH_MAP)
        message(FATAL_ERROR "${RA_SOC_NAME}: BL2 uses the platform flash map (DEFAULT_MCUBOOT_FLASH_MAP=OFF).")
    endif()
endif()

# The external OSPI/SDRAM/CS address space is hardwired non-secure in the
# IDAU and this cannot be overridden by software. Secure code must stay in
# on-chip ROM.
if(NOT S_ROM_ALIAS_BASE STREQUAL FLASH_BASE_ADDRESS)
    message(FATAL_ERROR "${RA_SOC_NAME}: S_ROM_ALIAS_BASE (${S_ROM_ALIAS_BASE}) must be the on-chip ROM secure alias (${FLASH_BASE_ADDRESS}) -- the external address space is hardwired non-secure and cannot host SPE code.")
endif()

# Checks whether [_base, _base + _size) falls within one of the known
# non-secure external windows (OSPI0/1 CS0/CS1, SDRAM; RA_NS_WINDOW_STARTS/
# RA_NS_WINDOW_ENDS, set in config.cmake). Used to validate any image
# relocated off-chip (e.g. the NS image on external flash).
# Sets _out_var to TRUE/FALSE in the caller's scope.
function(_ra_check_in_ns_window _base _size _out_var)
    math(EXPR _base_dec "${_base}")
    math(EXPR _end_dec "${_base} + ${_size}")

    set(_in_window FALSE)
    list(LENGTH RA_NS_WINDOW_STARTS _window_count)
    math(EXPR _window_last "${_window_count} - 1")
    foreach(_win_idx RANGE ${_window_last})
        list(GET RA_NS_WINDOW_STARTS ${_win_idx} _win_start)
        list(GET RA_NS_WINDOW_ENDS ${_win_idx} _win_end)
        math(EXPR _win_start_dec "${_win_start}")
        math(EXPR _win_end_dec "${_win_end}")
        if(NOT (_base_dec LESS _win_start_dec) AND NOT (_end_dec GREATER _win_end_dec))
            set(_in_window TRUE)
        endif()
    endforeach()

    set(${_out_var} ${_in_window} PARENT_SCOPE)
endfunction()

# ---- Flash instances ---------------------------------------------------------
# The generic MCUboot flash map code uses FLASH_AREA_IMAGE_SECTOR_SIZE and
# FLASH_PROGRAM_UNIT, the largest values of the flash instances that hold an
# image (post_config.cmake), for every flash area. Each instance's own value
# must divide them, so that every area is a whole number of sectors and program
# units of the device it is in.
# Instance 0 is the MRAM (FLASH0_*), instance 1 the external flash (FLASH1_*),
# which only counts when an NS image is in it.
set(_ra_geom_checks
    "FLASH0_SECTOR_SIZE|FLASH_AREA_IMAGE_SECTOR_SIZE"
    "FLASH0_PROGRAM_UNIT|FLASH_PROGRAM_UNIT")
if(RA_CPU0_NS_EXTERNAL OR RA_CPU1_NS_EXTERNAL)
    list(APPEND _ra_geom_checks
        "FLASH1_SECTOR_SIZE|FLASH_AREA_IMAGE_SECTOR_SIZE"
        "FLASH1_PROGRAM_UNIT|FLASH_PROGRAM_UNIT")
endif()
foreach(_ra_check IN LISTS _ra_geom_checks)
    string(REPLACE "|" ";" _ra_check "${_ra_check}")
    list(GET _ra_check 0 _ra_name)
    list(GET _ra_check 1 _ra_global_name)
    set(_ra_val "${${_ra_name}}")
    set(_ra_global "${${_ra_global_name}}")
    if(_ra_val LESS 1)
        message(FATAL_ERROR "${RA_SOC_NAME}: ${_ra_name} (${_ra_val}) must be at least 1.")
    endif()
    math(EXPR _ra_rem "${_ra_global} % ${_ra_val}")
    if(NOT _ra_rem EQUAL 0)
        message(FATAL_ERROR "${RA_SOC_NAME}: ${_ra_name} (${_ra_val}) must divide ${_ra_global_name} (${_ra_global}), the largest value of the flash instances.")
    endif()
endforeach()

# With no external flash driver the external areas use the MRAM driver (see
# flash_layout.h): the build works, but BL2 cannot access the image in the
# external flash.
if(BL2 AND S_BUILD AND (RA_CPU0_NS_EXTERNAL OR RA_CPU1_NS_EXTERNAL) AND NOT FLASH1_DEV_NAME)
    message(WARNING "${RA_SOC_NAME}: an NS image is in the external flash but FLASH1_DEV_NAME is empty: BL2 falls back to the MRAM driver for it and cannot read it. Set FLASH1_DEV_NAME to the CMSIS driver of the external flash.")
endif()

# An NS image whose ROM alias is not the MRAM one (RA_CPU0_NS_EXTERNAL /
# RA_CPU1_NS_EXTERNAL, derived by post_config.cmake) lives in the external flash
# (flash instance 1). All its slots (primary and, with BL2, secondary) must fall
# within one non-secure OSPI/SDRAM window, and they are not counted against the
# MRAM budget below. The slot sizes can be changed on the command line, so the
# window is checked with the final values.
function(_ra_check_ext_image _name _alias _offset _slot_size _out_base _out_end)
    math(EXPR _base "${_alias} + ${_offset}")
    math(EXPR _size "${FLASH_SLOTS_PER_IMAGE} * ${_slot_size}")
    _ra_check_in_ns_window("${_base}" "${_size}" _in_window)
    if(NOT _in_window)
        message(FATAL_ERROR "${RA_SOC_NAME}: the ${_name} NS image (ROM alias ${_alias} + offset ${_offset}, ${FLASH_SLOTS_PER_IMAGE} slot(s) of ${_slot_size}) does not fall within a known non-secure external window (OSPI0/1 CS0/CS1 or SDRAM).")
    endif()
    math(EXPR _end "${_base} + ${_size}")
    set(${_out_base} ${_base} PARENT_SCOPE)
    set(${_out_end} ${_end} PARENT_SCOPE)
endfunction()

if(RA_CPU0_NS_EXTERNAL)
    _ra_check_ext_image("CPU0" "${CPU0_NS_ROM_ALIAS_BASE}" "${CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET}"
                        "${FLASH_CPU0_NS_PARTITION_SIZE}" _ra_cpu0_ext_base _ra_cpu0_ext_end)
endif()

# ---- CPU1 (multi-core) map checks --------------------------------------------
# Keyed on RA_CM33_NS_PRESENT rather than TFM_MULTI_CORE_TOPOLOGY: the generated
# NS config.cmake forces TFM_MULTI_CORE_TOPOLOGY off for a CPU0 TrustZone NS
# build, but the CPU1 slots the SPE reserved still exist and must still be
# validated on that side.
if(RA_CM33_NS_PRESENT)
    if(RA_CPU1_NS_EXTERNAL)
        _ra_check_ext_image("CPU1" "${CPU1_NS_ROM_ALIAS_BASE}" "${CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET}"
                            "${FLASH_CPU1_NS_PARTITION_SIZE}" _ra_cpu1_ext_base _ra_cpu1_ext_end)
        if(RA_CPU0_NS_EXTERNAL AND (_ra_cpu1_ext_base LESS _ra_cpu0_ext_end) AND (_ra_cpu0_ext_base LESS _ra_cpu1_ext_end))
            message(FATAL_ERROR "${RA_SOC_NAME}: the CPU0 and CPU1 NS images overlap in the external flash (CPU0 [${_ra_cpu0_ext_base}, ${_ra_cpu0_ext_end}), CPU1 [${_ra_cpu1_ext_base}, ${_ra_cpu1_ext_end})).")
        endif()
    endif()

    # CPU1's SRAM window must not overlap the S/NS data regions it shares the
    # bank with. S_RAM_ALIAS_BASE and CPU0_NS_RAM_ALIAS_BASE (and CPU1_NS_RAM_ALIAS_BASE)
    # are different *aliases* of the same physical SRAM (HW manual Address
    # Space 5.1), so comparisons must be done in bank-relative offsets, not
    # raw aliased addresses.
    set(_s_ram_offset 0)
    math(EXPR _s_ram_end_offset "${_s_ram_offset} + ${S_DATA_SIZE}")
    set(_ns_cpu0_ram_offset "${CPU0_NS_RAM_OFFSET}")
    math(EXPR _ns_cpu0_ram_end_offset "${_ns_cpu0_ram_offset} + ${CPU0_NS_RAM_DATA_SIZE}")
    set(_ns_cpu1_ram_offset "${CPU1_NS_RAM_OFFSET}")
    math(EXPR _ns_cpu1_ram_end_offset "${_ns_cpu1_ram_offset} + ${CPU1_NS_RAM_DATA_SIZE}")

    if((_ns_cpu1_ram_offset LESS _s_ram_end_offset) AND (_ns_cpu1_ram_end_offset GREATER _s_ram_offset))
        message(FATAL_ERROR "${RA_SOC_NAME}: CPU1 RAM window (bank offset [${_ns_cpu1_ram_offset}, +${CPU1_NS_RAM_DATA_SIZE})) overlaps the secure data region (bank offset [${_s_ram_offset}, +${S_DATA_SIZE})).")
    endif()
    if((_ns_cpu1_ram_offset LESS _ns_cpu0_ram_end_offset) AND (_ns_cpu1_ram_end_offset GREATER _ns_cpu0_ram_offset))
        message(FATAL_ERROR "${RA_SOC_NAME}: CPU1 RAM window (bank offset [${_ns_cpu1_ram_offset}, +${CPU1_NS_RAM_DATA_SIZE})) overlaps the non-secure data region (bank offset [${_ns_cpu0_ram_offset}, +${CPU0_NS_RAM_DATA_SIZE})).")
    endif()

endif()

# ---- Alignment checks --------------------------------------------------------

# The S/NS flash boundary (end of BL2, S slot(s) and storage, i.e.
# FLASH_S_REGION_SIZE) must land on one of the on-chip ROM
# security-attribution granular boundaries (HW manual Security Features).
math(EXPR _s_attr_rem "${FLASH_S_REGION_SIZE} % ${RA_ROM_ATTR_GRANULARITY}")
if(NOT _s_attr_rem EQUAL 0)
    message(FATAL_ERROR "${RA_SOC_NAME}: the secure MRAM region (BL2 + S slots + storage = ${FLASH_S_REGION_SIZE}) must be a multiple of ${RA_ROM_ATTR_GRANULARITY} (on-chip ROM security-attribution granularity).")
endif()

# The MRAM FS block needs to align to the on-chip ROM attribution granularity when
# it is at least as large as that granularity.
if(FLASH0_SECTOR_SIZE GREATER_EQUAL RA_ROM_ATTR_GRANULARITY)
    math(EXPR _sector_attr_rem "${FLASH0_SECTOR_SIZE} % ${RA_ROM_ATTR_GRANULARITY}")
    if(NOT _sector_attr_rem EQUAL 0)
        message(FATAL_ERROR "${RA_SOC_NAME}: FLASH0_SECTOR_SIZE (${FLASH0_SECTOR_SIZE}) must be a multiple of ${RA_ROM_ATTR_GRANULARITY} (on-chip ROM security-attribution granularity).")
    endif()
endif()

# SRAM security attribution is expressed in RA_SRAM_ATTR_GRANULARITY units
# (BA units, HW manual Security Features).
math(EXPR _sdata_attr_rem "${S_DATA_SIZE} % ${RA_SRAM_ATTR_GRANULARITY}")
if(NOT _sdata_attr_rem EQUAL 0)
    message(FATAL_ERROR "${RA_SOC_NAME}: S_DATA_SIZE (${S_DATA_SIZE}) must be a multiple of ${RA_SRAM_ATTR_GRANULARITY} (SRAM security-attribution granularity).")
endif()

# The RAM-resident on-chip ROM programming code carve-out (S_RAM_CODE_SIZE,
# used by .ER_CODE_SRAM) must be word-aligned and leave room for the secure
# data. Secure-side only: the carve-out lives inside the secure data region and
# only the secure linker script places it.
if(S_BUILD)
    math(EXPR _ram_code_rem "${S_RAM_CODE_SIZE} % 4")
    if(NOT _ram_code_rem EQUAL 0)
        message(FATAL_ERROR "${RA_SOC_NAME}: S_RAM_CODE_SIZE (${S_RAM_CODE_SIZE}) must be a multiple of 4.")
    endif()
    if(S_RAM_CODE_SIZE GREATER_EQUAL S_DATA_SIZE)
        message(FATAL_ERROR "${RA_SOC_NAME}: S_RAM_CODE_SIZE (${S_RAM_CODE_SIZE}) must be smaller than S_DATA_SIZE (${S_DATA_SIZE}).")
    endif()
endif()

math(EXPR _punit_sector_rem "${FLASH_AREA_IMAGE_SECTOR_SIZE} % ${FLASH_PROGRAM_UNIT}")
if(NOT _punit_sector_rem EQUAL 0)
    message(FATAL_ERROR "${RA_SOC_NAME}: FLASH_AREA_IMAGE_SECTOR_SIZE (${FLASH_AREA_IMAGE_SECTOR_SIZE}) must be a multiple of FLASH_PROGRAM_UNIT (${FLASH_PROGRAM_UNIT}).")
endif()

# The image slots are a whole number of the sectors the MCUboot flash map code
# uses; BL2 and the storage areas are in MRAM, so they follow its FS block.
foreach(_area_size IN ITEMS FLASH_S_PARTITION_SIZE FLASH_CPU0_NS_PARTITION_SIZE FLASH_CPU1_NS_PARTITION_SIZE)
    math(EXPR _rem "${${_area_size}} % ${FLASH_AREA_IMAGE_SECTOR_SIZE}")
    if(NOT _rem EQUAL 0)
        message(FATAL_ERROR "${RA_SOC_NAME}: ${_area_size} (${${_area_size}}) must be a multiple of FLASH_AREA_IMAGE_SECTOR_SIZE (${FLASH_AREA_IMAGE_SECTOR_SIZE}).")
    endif()
endforeach()

foreach(_area_size IN ITEMS FLASH_AREA_BL2_SIZE FLASH_PS_AREA_SIZE FLASH_ITS_AREA_SIZE FLASH_OTP_NV_COUNTERS_AREA_SIZE)
    math(EXPR _rem "${${_area_size}} % ${FLASH0_SECTOR_SIZE}")
    if(NOT _rem EQUAL 0)
        message(FATAL_ERROR "${RA_SOC_NAME}: ${_area_size} (${${_area_size}}) must be a multiple of FLASH0_SECTOR_SIZE (${FLASH0_SECTOR_SIZE}).")
    endif()
endforeach()

# ---- Fit checks --------------------------------------------------------------

# The on-chip ROM chain is: BL2, S slot(s) (code + CMSE veneers), the storage
# areas (together FLASH_S_REGION_SIZE), then CPU0 NS starts at
# CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET (primary, plus secondary with BL2),
# then the CPU1 NS image. Each on-chip ROM region is validated against its real
# [offset, offset+size) end, so an offset larger than the preceding chain cannot
# silently overflow on-chip ROM, and a smaller offset cannot silently overlap
# the chain before it.
set(_ns_chain_end "${FLASH_S_REGION_SIZE}")
if(NOT RA_CPU0_NS_EXTERNAL)
    if(CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET LESS _ns_chain_end)
        message(FATAL_ERROR "${RA_SOC_NAME}: CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET (${CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET}) is before the end of the S slots/storage areas (${_ns_chain_end}); the NS image would overlap S/storages.")
    endif()
    math(EXPR _ns_chain_end "${CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET} + ${FLASH_SLOTS_PER_IMAGE} * ${FLASH_CPU0_NS_PARTITION_SIZE}")
endif()

# The CMSE veneer region is placed by the linker script at the top of the S code area.
# Secure-side only: FLASH_CPU0_C_SIZE is produced by fsp/s_ns_memory.cmake
# during the platform build, so it is only in scope here because TF-M includes
# this chain after add_subdirectory(platform). The NS build includes the chain
# from its config.cmake, long before any FSP CMake has run.
if(S_BUILD AND FLASH_CPU0_C_SIZE GREATER FLASH_S_PARTITION_SIZE)
    message(FATAL_ERROR "${RA_SOC_NAME}: the CMSE veneer region (${FLASH_CPU0_C_SIZE} bytes) exceeds the S slot (${FLASH_S_PARTITION_SIZE}).")
endif()

set(_rom_end "${_ns_chain_end}")
if(RA_CM33_NS_PRESENT AND NOT RA_CPU1_NS_EXTERNAL)
    if(CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET LESS _ns_chain_end)
        message(FATAL_ERROR "${RA_SOC_NAME}: CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET (${CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET}) is before the end of the S+NS chain (${_ns_chain_end}); CPU1 would overlap.")
    endif()
    math(EXPR _cpu1_end "${CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET} + ${FLASH_SLOTS_PER_IMAGE} * ${FLASH_CPU1_NS_PARTITION_SIZE}")
    if(_cpu1_end GREATER _rom_end)
        set(_rom_end "${_cpu1_end}")
    endif()
endif()

if(_rom_end GREATER FLASH_TOTAL_SIZE)
    math(EXPR _shortfall "${_rom_end} - ${FLASH_TOTAL_SIZE}")
    message(FATAL_ERROR "${RA_SOC_NAME}: flash map does not fit on-chip ROM: high-water mark ${_rom_end} (${_rom_end} decimal), have FLASH_TOTAL_SIZE=${FLASH_TOTAL_SIZE}, short by ${_shortfall} bytes.")
endif()

math(EXPR _ram_used "${S_DATA_SIZE} + ${CPU0_NS_RAM_DATA_SIZE}")
if(RA_CM33_NS_PRESENT)
    math(EXPR _ram_used "${_ram_used} + ${CPU1_NS_RAM_DATA_SIZE}")
endif()
if(_ram_used GREATER TOTAL_RAM_SIZE)
    math(EXPR _ram_shortfall "${_ram_used} - ${TOTAL_RAM_SIZE}")
    if(RA_CM33_NS_PRESENT)
        message(FATAL_ERROR "${RA_SOC_NAME}: S_DATA_SIZE + CPU0_NS_RAM_DATA_SIZE + CPU1_NS_RAM_DATA_SIZE (${_ram_used}) exceeds TOTAL_RAM_SIZE (${TOTAL_RAM_SIZE}), short by ${_ram_shortfall} bytes.")
    else()
        message(FATAL_ERROR "${RA_SOC_NAME}: S_DATA_SIZE + CPU0_NS_RAM_DATA_SIZE (${_ram_used}) exceeds TOTAL_RAM_SIZE (${TOTAL_RAM_SIZE}), short by ${_ram_shortfall} bytes.")
    endif()
endif()
