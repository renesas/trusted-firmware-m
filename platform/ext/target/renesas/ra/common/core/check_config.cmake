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

if(BL2)
    message(FATAL_ERROR "${RA_SOC_NAME}: BL2 currently not supported. Keep BL2 OFF.")
endif()

if(NOT RA_CM85_NS_PRESENT)
    message(FATAL_ERROR "${RA_SOC_NAME}: RA_CM85_NS_PRESENT OFF is not supported.")
endif()

if(DEFINED MCUBOOT_IMAGE_NUMBER AND MCUBOOT_IMAGE_NUMBER GREATER 2)
    message(FATAL_ERROR "${RA_SOC_NAME}: only MCUBOOT_IMAGE_NUMBER 1 and 2 are supported.")
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

# When the NS image is relocated off on-chip ROM (e.g. onto external
# HyperFlash), it must land in one of the non-secure OSPI/SDRAM windows and
# must not be double-counted against the on-chip ROM budget below.
set(_RA_CPU0_NS_IN_ONCHIP_ROM ${RA_CM85_NS_PRESENT})
if(RA_CM85_NS_PRESENT AND NOT CPU0_NS_ROM_ALIAS_BASE STREQUAL "${RA_FLASH_NS_ALIAS}")
    set(_RA_CPU0_NS_IN_ONCHIP_ROM FALSE)

    _ra_check_in_ns_window("${CPU0_NS_ROM_ALIAS_BASE}" "${FLASH_CPU0_NS_PARTITION_SIZE}" _ra_ns_in_window)
    if(NOT _ra_ns_in_window)
        message(FATAL_ERROR "${RA_SOC_NAME}: CPU0_NS_ROM_ALIAS_BASE=${CPU0_NS_ROM_ALIAS_BASE} with FLASH_CPU0_NS_PARTITION_SIZE=${FLASH_CPU0_NS_PARTITION_SIZE} does not fall within a known non-secure external window (OSPI0/1 CS0/CS1 or SDRAM).")
    endif()
endif()

# ---- CPU1 (multi-core) map checks --------------------------------------------
# Keyed on RA_CM33_NS_PRESENT rather than TFM_MULTI_CORE_TOPOLOGY: the generated
# NS config.cmake forces TFM_MULTI_CORE_TOPOLOGY off for a CPU0 TrustZone NS
# build, but the CPU1 slots the SPE reserved still exist and must still be
# validated on that side.
set(_RA_CPU1_NS_IN_ONCHIP_ROM TRUE)
if(RA_CM33_NS_PRESENT)
    # When the CPU1 image is relocated off MRAM, it must land in a known
    # non-secure external window, and must not be double-counted against the
    # MRAM budget below. A secure CPU1 image is never off-chip (rejected
    # above), so this only applies to the non-secure case.
    if(NOT CPU1_NS_ROM_ALIAS_BASE STREQUAL "${RA_FLASH_NS_ALIAS}")
        set(_RA_CPU1_NS_IN_ONCHIP_ROM FALSE)

        _ra_check_in_ns_window("${CPU1_NS_ROM_ALIAS_BASE}" "${FLASH_CPU1_NS_PARTITION_SIZE}" _ra_cpu1_in_window)
        if(NOT _ra_cpu1_in_window)
            message(FATAL_ERROR "${RA_SOC_NAME}: CPU1_NS_ROM_ALIAS_BASE=${CPU1_NS_ROM_ALIAS_BASE} with FLASH_CPU1_NS_PARTITION_SIZE=${FLASH_CPU1_NS_PARTITION_SIZE} does not fall within a known non-secure external window (OSPI0/1 CS0/CS1 or SDRAM).")
        endif()
    endif()

    math(EXPR _cpu1_sector_rem "${FLASH_CPU1_NS_PARTITION_SIZE} % ${FLASH_AREA_IMAGE_SECTOR_SIZE}")
    if(NOT _cpu1_sector_rem EQUAL 0)
        message(FATAL_ERROR "${RA_SOC_NAME}: FLASH_CPU1_NS_PARTITION_SIZE (${FLASH_CPU1_NS_PARTITION_SIZE}) must be a multiple of FLASH_AREA_IMAGE_SECTOR_SIZE (${FLASH_AREA_IMAGE_SECTOR_SIZE}).")
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

# The S/NS flash boundary (FLASH_S_PARTITION_SIZE) must land on one of the
# on-chip ROM security-attribution granular boundaries (HW manual Security
# Features).
math(EXPR _s_attr_rem "${FLASH_S_PARTITION_SIZE} % ${RA_ROM_ATTR_GRANULARITY}")
if(NOT _s_attr_rem EQUAL 0)
    message(FATAL_ERROR "${RA_SOC_NAME}: FLASH_S_PARTITION_SIZE (${FLASH_S_PARTITION_SIZE}) must be a multiple of ${RA_ROM_ATTR_GRANULARITY} (on-chip ROM security-attribution granularity).")
endif()

# The FS block needs to align to the on-chip ROM attribution granularity when it is at
# least as large as that granularity.
if(FLASH_AREA_IMAGE_SECTOR_SIZE GREATER_EQUAL RA_ROM_ATTR_GRANULARITY)
    math(EXPR _sector_attr_rem "${FLASH_AREA_IMAGE_SECTOR_SIZE} % ${RA_ROM_ATTR_GRANULARITY}")
    if(NOT _sector_attr_rem EQUAL 0)
        message(FATAL_ERROR "${RA_SOC_NAME}: FLASH_AREA_IMAGE_SECTOR_SIZE (${FLASH_AREA_IMAGE_SECTOR_SIZE}) must be a multiple of ${RA_ROM_ATTR_GRANULARITY} (on-chip ROM security-attribution granularity).")
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

foreach(_area_size IN ITEMS FLASH_S_PARTITION_SIZE FLASH_CPU0_NS_PARTITION_SIZE FLASH_PS_AREA_SIZE FLASH_ITS_AREA_SIZE FLASH_OTP_NV_COUNTERS_AREA_SIZE)
    math(EXPR _rem "${${_area_size}} % ${FLASH_AREA_IMAGE_SECTOR_SIZE}")
    if(NOT _rem EQUAL 0)
        message(FATAL_ERROR "${RA_SOC_NAME}: ${_area_size} (${${_area_size}}) must be a multiple of FLASH_AREA_IMAGE_SECTOR_SIZE (${FLASH_AREA_IMAGE_SECTOR_SIZE}).")
    endif()
endforeach()

# ---- Fit checks --------------------------------------------------------------

# BL2 is OFF (enforced above), so only the primary S (+ NS, if it lives in
# on-chip ROM) slot occupies on-chip ROM -- no secondary/scratch chain. The
# on-chip ROM chain is: S slot (code + CMSE veneers), then the storage areas,
# then NS starts at CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET. Rather than summing
# sizes (which ignores user-set gaps), each on-chip ROM region is validated
# against its real [offset, offset+size) end, so an offset larger than the
# preceding chain cannot silently overflow on-chip ROM, and a smaller offset
# cannot silently overlap the chain before it.
math(EXPR _ns_chain_end "${FLASH_AREA_BL2_OFFSET} + ${FLASH_AREA_BL2_SIZE} + ${FLASH_S_PARTITION_SIZE} + ${FLASH_STORAGE_TOTAL_SIZE}")
if(_RA_CPU0_NS_IN_ONCHIP_ROM)
    if(CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET LESS _ns_chain_end)
        message(FATAL_ERROR "${RA_SOC_NAME}: CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET (${CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET}) is before the end of the S slot/storage areas (${_ns_chain_end}); the NS image would overlap S/storages.")
    endif()
    math(EXPR _ns_chain_end "${_ns_chain_end} + ${FLASH_CPU0_NS_PARTITION_SIZE}")
endif()

# The CMSE veneer region is placed by the linker script inside the S slot.
# Secure-side only: FLASH_CPU0_C_SIZE is produced by fsp/s_ns_memory.cmake
# during the platform build, so it is only in scope here because TF-M includes
# this chain after add_subdirectory(platform). The NS build includes the chain
# from its config.cmake, long before any FSP CMake has run.
if(S_BUILD AND FLASH_CPU0_C_SIZE GREATER FLASH_S_PARTITION_SIZE)
    message(FATAL_ERROR "${RA_SOC_NAME}: the CMSE veneer region (${FLASH_CPU0_C_SIZE} bytes) exceeds the S slot (${FLASH_S_PARTITION_SIZE}).")
endif()

set(_rom_end "${_ns_chain_end}")
if(RA_CM33_NS_PRESENT AND _RA_CPU1_NS_IN_ONCHIP_ROM)
    if(CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET LESS _ns_chain_end)
        message(FATAL_ERROR "${RA_SOC_NAME}: CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET (${CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET}) is before the end of the S+NS chain (${_ns_chain_end}); CPU1 would overlap.")
    endif()
    math(EXPR _cpu1_end "${CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET} + ${FLASH_CPU1_NS_PARTITION_SIZE}")
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
