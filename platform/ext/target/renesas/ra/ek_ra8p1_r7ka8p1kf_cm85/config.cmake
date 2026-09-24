#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(CONFIG_TFM_USE_TRUSTZONE   ON          CACHE BOOL      "Use TrustZone")
set(CONFIG_TFM_ENABLE_FP       ON          CACHE BOOL      "Enable floating point support")
set(BL2                        OFF         CACHE BOOL      "Whether to build BL2")
set(CONFIG_TFM_INCLUDE_STDLIBC ON          CACHE BOOL      "Include standard C libraries and startup code or not")

if(NOT TFM_EXTRA_GENERATED_FILE_LIST_PATH)
    set(TFM_EXTRA_GENERATED_FILE_LIST_PATH "" CACHE PATH "List of Paths to extra generated file list. Appended to standard TFM generated file list.")
endif()

# Renesas specific generated files
list(APPEND TFM_EXTRA_GENERATED_FILE_LIST_PATH "${RENESAS_RA_COMMON_SOURCE_DIR}/core/generated_file_list.yaml")

#===================== RA8P1 flash/RAM map =====================================#
# Hardware facts per the RA8P1 HW manual: Overview 1.3 (Part Numbering),
# Address Space 5.1, Security Features 52.3.3.5 (MRAM/SRAM attribution
# granularity), MRAM 60.1/60.4.2 (32-byte page buffer, no erase blocks),
# OSPI 44.1 (external address space is hardwired non-secure).
#
# The EK-RA8P1 carries the R7KA8P1KF part (1 MB code MRAM, 1.6 MB on-chip
# SRAM SRAM0-3), so no per-variant preset is needed here (the 512 KB MRAM
# parts are single-core only); every individual size below remains
# independently overridable on the command line.

# ---- MRAM (code flash) geometry --------------------------------------------
set(FLASH_BASE_ADDRESS           "0x02000000"       CACHE STRING "MRAM secure alias base")
set(FLASH_TOTAL_SIZE             "0x100000"         CACHE STRING "MRAM user area size (R7KA8P1KF: 1 MB)")
# MRAM has no hardware erase-blocks (dual-bank and block-swap not available;
# FSP r_mram "erase" is 32-byte page writes of 0xFF). This is a software FS
# block size chosen to match the 8 KB MRAM block unit; it is also the OTP NV
# counter sector size (see FLASH_OTP_NV_COUNTERS_AREA_SIZE below).
set(FLASH_AREA_IMAGE_SECTOR_SIZE "0x2000"           CACHE STRING "FS block granularity (8 KB MRAM block unit)")
# MRAM programs data through a 32-byte page buffer. Erase operations are
# implemented by programming 0xFF in page-sized (32-byte) units.
set(FLASH_PAGE_SIZE              "0x20"             CACHE STRING "MRAM page buffer size in bytes")
# MRAM supports programming 1-32 bytes at a time, but TF-M requires a fixed
# power-of-two program unit.
set(FLASH_PROGRAM_UNIT           "0x04"             CACHE STRING "MRAM minimum write size in bytes")

# ---- Image partitions -------------------------------------------------------
# BL2 is OFF: no MRAM-capable bootloader flash driver exists yet, so the
# secondary/scratch areas collapse to zero (see flash_layout.h and
# check_config.cmake, which reject turning BL2 ON without such a driver).
set(FLASH_AREA_BL2_OFFSET   "0x0"    CACHE STRING "BL2 offset (BL2 unsupported)")
set(FLASH_AREA_BL2_SIZE     "0x0"    CACHE STRING "BL2 size (BL2 unsupported)")
set(FLASH_S_PARTITION_SIZE  "0x50000" CACHE STRING "Secure image slot size")
set(FLASH_NS_PARTITION_SIZE "0x64000" CACHE STRING "Non-secure image slot size")

# NS image offset within its ROM alias, independent of the secure chain so the
# NS image can be relocated off-chip (e.g. onto the EK-RA8P1's OSPI0 CS0) by
# overriding this (to 0x0) together with NS_ROM_ALIAS_BASE. Default keeps NS
# right after S within MRAM, matching FLASH_AREA_0_OFFSET + FLASH_S_PARTITION_SIZE.
math(EXPR _RA8P1_NS_DEFAULT_OFFSET "${FLASH_AREA_BL2_OFFSET} + ${FLASH_AREA_BL2_SIZE} + ${FLASH_S_PARTITION_SIZE}")
set(NS_IMAGE_PRIMARY_PARTITION_OFFSET "${_RA8P1_NS_DEFAULT_OFFSET}" CACHE STRING "NS image offset within its ROM alias")

# ---- Storage areas -----------------------------------------------------------
set(FLASH_PS_AREA_SIZE              "0x4000" CACHE STRING "Protected Storage area size")
set(FLASH_ITS_AREA_SIZE             "0x4000" CACHE STRING "Internal Trusted Storage area size")
set(FLASH_OTP_NV_COUNTERS_AREA_SIZE "0x4000" CACHE STRING "OTP/NV counters area size (2 x FLASH_AREA_IMAGE_SECTOR_SIZE)")

# ---- SRAM --------------------------------------------------------------------
set(SRAM_BASE_ADDRESS "0x22000000" CACHE STRING "SRAM secure alias base")
# 1.6 MB on-chip SRAM (SRAM0-3) = 0x2200_0000-0x221A_0000 (secure alias),
# per the RA8P1 HW manual, Address Space 5.1.
set(TOTAL_RAM_SIZE    "0x1A0000"   CACHE STRING "On-chip SRAM size (SRAM0-3)")
set(S_DATA_SIZE  "0x80000"  CACHE STRING "Secure RAM size")
set(NS_DATA_SIZE "0x80000"  CACHE STRING "Non-secure RAM size")
# Secure SRAM carved out for RAM-resident MRAM programming code (r_mram
# .ram_from_flash): MRAM cannot be read while being programmed, so the
# write/erase helpers must execute from SRAM (placed via .ER_CODE_SRAM).
set(S_RAM_CODE_SIZE "0x1000" CACHE STRING "Secure RAM size for RAM-resident MRAM programming code")

# ---- Address aliases -----------------------------------------------------
set(S_ROM_ALIAS_BASE  "${FLASH_BASE_ADDRESS}" CACHE STRING "MRAM secure alias base")
set(NS_ROM_ALIAS_BASE "0x12000000"            CACHE STRING "MRAM non-secure alias base (override to relocate NS image off-chip, e.g. 0x88000000 for OSPI0 CS0)")
set(S_RAM_ALIAS_BASE  "${SRAM_BASE_ADDRESS}"  CACHE STRING "SRAM secure alias base")
set(NS_RAM_ALIAS_BASE "0x32000000"            CACHE STRING "SRAM non-secure alias base")

# Non-secure external address windows (OSPI0/1 CS0/CS1, SDRAM). The external
# address space is hardwired non-secure in the IDAU (HW manual OSPI 44.1);
# consumed by check_config.cmake to validate any image relocated off-chip.
set(RA_NS_WINDOW_STARTS "0x88000000" "0x98000000" "0x74000000" "0x7c000000" "0x6c000000" CACHE STRING "Start addresses of the non-secure external windows (OSPI0/1 CS0/CS1, SDRAM)")
set(RA_NS_WINDOW_ENDS   "0x8c000000" "0x9c000000" "0x76000000" "0x7e000000" "0x6e000000" CACHE STRING "End (exclusive) addresses of the non-secure external windows (must match RA_NS_WINDOW_STARTS)")

include(${CMAKE_CURRENT_LIST_DIR}/check_config.cmake)
