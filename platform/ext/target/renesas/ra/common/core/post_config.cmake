#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Flash/RAM map derived from the slot and area sizes, included by each board's
# post_config.cmake.
#
# TF-M includes the board post_config.cmake after the board config.cmake and
# after the test configuration (tests/tfm_tests_config.cmake), so the values
# below follow the final sizes. They are normal variables, recomputed on every
# configure; an offset given on the command line (-D) is kept as is, e.g. to
# place an NS image at another offset of its external flash.

# An NS image is "external" when its ROM alias is not the on-chip MRAM one (e.g.
# -DCPU0_NS_ROM_ALIAS_BASE=0x88000000, an OSPI window). Its slots then live in
# the external flash (flash instance 1) instead of MRAM (flash instance 0).
set(RA_CPU0_NS_EXTERNAL OFF)
set(RA_CPU1_NS_EXTERNAL OFF)
math(EXPR _RA_MRAM_NS_ALIAS "${FLASH_BASE_NS}")
if(RA_CM85_NS_PRESENT)
    math(EXPR _ra_alias "${CPU0_NS_ROM_ALIAS_BASE}")
    if(NOT _ra_alias EQUAL _RA_MRAM_NS_ALIAS)
        set(RA_CPU0_NS_EXTERNAL ON)
    endif()
endif()
if(RA_CM33_NS_PRESENT)
    math(EXPR _ra_alias "${CPU1_NS_ROM_ALIAS_BASE}")
    if(NOT _ra_alias EQUAL _RA_MRAM_NS_ALIAS)
        set(RA_CPU1_NS_EXTERNAL ON)
    endif()
endif()

# The generic MCUboot flash map code (bl2/ext/mcuboot/flash_map_legacy.c,
# bl2/src/flash_map.c) uses one sector size and one program unit for every flash
# area, whichever flash device it is in. They are therefore the largest value
# of the flash instances that hold an image; check_config.cmake verifies that
# every instance's value divides them. The storage (PS/ITS/OTP) is always in
# MRAM and uses the instance 0 values directly.
set(FLASH_AREA_IMAGE_SECTOR_SIZE "${FLASH0_SECTOR_SIZE}")
set(FLASH_PROGRAM_UNIT           "${FLASH0_PROGRAM_UNIT}")
if(RA_CPU0_NS_EXTERNAL OR RA_CPU1_NS_EXTERNAL)
    if(FLASH1_SECTOR_SIZE GREATER FLASH_AREA_IMAGE_SECTOR_SIZE)
        set(FLASH_AREA_IMAGE_SECTOR_SIZE "${FLASH1_SECTOR_SIZE}")
    endif()
    if(FLASH1_PROGRAM_UNIT GREATER FLASH_PROGRAM_UNIT)
        set(FLASH_PROGRAM_UNIT "${FLASH1_PROGRAM_UNIT}")
    endif()
endif()

math(EXPR FLASH_STORAGE_TOTAL_SIZE "${FLASH_PS_AREA_SIZE} + ${FLASH_ITS_AREA_SIZE} + ${FLASH_OTP_NV_COUNTERS_AREA_SIZE}")

# The images are packed in MRAM in this order:
#   [BL2] S image [S secondary], storage areas, CPU0 NS image [CPU0 NS secondary],
#   CPU1 NS image
# except the NS images that are external (RA_CPU0_NS_EXTERNAL /
# RA_CPU1_NS_EXTERNAL), which are packed in their external flash window.
# With BL2 each of the S, CPU0 NS and CPU1 NS images has a secondary slot of the
# same size (overwrite-only upgrade, no scratch). BL2 only verifies the CPU1
# image, the SPE starts CPU1. RAM: S data, CPU0 NS data, CPU1 NS data.
if(BL2)
    set(FLASH_SLOTS_PER_IMAGE 2)
else()
    set(FLASH_SLOTS_PER_IMAGE 1)
endif()

# Size of the secure-attributed MRAM: BL2, the S slot(s) and the storage areas.
# This is the MRAM secure/non-secure boundary that must be programmed in the
# device (DLM: the boundary is not set at runtime).
math(EXPR FLASH_S_REGION_SIZE "${FLASH_AREA_BL2_OFFSET} + ${FLASH_AREA_BL2_SIZE} + ${FLASH_SLOTS_PER_IMAGE} * ${FLASH_S_PARTITION_SIZE} + ${FLASH_STORAGE_TOTAL_SIZE}" OUTPUT_FORMAT HEXADECIMAL)

math(EXPR _RA_S_CHAIN_END     "${FLASH_S_REGION_SIZE}")
math(EXPR _RA_CPU1_ROM_OFFSET "${_RA_S_CHAIN_END} + ${FLASH_SLOTS_PER_IMAGE} * ${FLASH_CPU0_NS_PARTITION_SIZE}")
math(EXPR _RA_CPU1_RAM_OFFSET "${S_DATA_SIZE} + ${CPU0_NS_RAM_DATA_SIZE}")

#===================== NS image, CPU0 Cortex-M85 (TrustZone) ===================#
# The offset is relative to CPU0_NS_ROM_ALIAS_BASE. An external image starts at
# the beginning of its window by default, so the MRAM left by it is not wasted.
if(NOT DEFINED CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET)
    if(RA_CPU0_NS_EXTERNAL)
        set(CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET "0x0")
    else()
        set(CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET "${_RA_S_CHAIN_END}")
    endif()
endif()
if(NOT DEFINED CPU0_NS_RAM_OFFSET)
    set(CPU0_NS_RAM_OFFSET "${S_DATA_SIZE}")
endif()

#===================== NS image, CPU1 Cortex-M33 (Mailbox) =====================#
# Packed after the CPU0 image when it is in the same flash window, otherwise at
# the start of its own: the end of the S chain in MRAM (CPU0 external), or the
# beginning of its external window.
if(NOT DEFINED CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET)
    if(RA_CPU1_NS_EXTERNAL)
        set(_ra_cpu1_offset "0x0")
        if(RA_CPU0_NS_EXTERNAL)
            math(EXPR _ra_cpu0_alias "${CPU0_NS_ROM_ALIAS_BASE}")
            math(EXPR _ra_cpu1_alias "${CPU1_NS_ROM_ALIAS_BASE}")
            if(_ra_cpu0_alias EQUAL _ra_cpu1_alias)
                math(EXPR _ra_cpu1_offset "${CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET} + ${FLASH_SLOTS_PER_IMAGE} * ${FLASH_CPU0_NS_PARTITION_SIZE}" OUTPUT_FORMAT HEXADECIMAL)
            endif()
        endif()
        set(CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET "${_ra_cpu1_offset}")
    elseif(RA_CPU0_NS_EXTERNAL)
        set(CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET "${_RA_S_CHAIN_END}")
    else()
        set(CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET "${_RA_CPU1_ROM_OFFSET}")
    endif()
endif()
if(NOT DEFINED CPU1_NS_RAM_OFFSET)
    set(CPU1_NS_RAM_OFFSET "${_RA_CPU1_RAM_OFFSET}")
endif()
