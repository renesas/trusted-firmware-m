#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# RA8M2 SoC-level configuration, shared by every board built on this SoC.

#===================== SoC identity and attribution units ======================#

set(RA_SOC_NAME "RA8M2")

# Security-attribution granularity: the S/NS boundary can only be placed on a
# multiple of these (HW manual Security Features 51.3.3.5).
set(RA_ROM_ATTR_GRANULARITY  "0x8000")
set(RA_SRAM_ATTR_GRANULARITY "0x2000")

# Non-secure aliases of the on-chip MRAM and SRAM (HW manual Address Space 5.1).
set(RA_FLASH_NS_ALIAS "0x12000000")
set(RA_SRAM_NS_ALIAS  "0x32000000")

# MRAM programs data through a 32-byte page buffer, so the minimum programmable
# unit can never exceed it (HW manual MRAM 59.4.2).
set(RA_FLASH_PAGE_BUFFER_SIZE "0x20")

#===================== MRAM (code flash) geometry ==============================#
set(FLASH_BASE_ADDRESS "0x02000000" CACHE STRING "MRAM secure alias base")
set(FLASH_TOTAL_SIZE   "0x100000"   CACHE STRING "MRAM area size (R7KA8M2JF: 1 MB)")

# MRAM has no hardware erase-blocks (dual-bank and block-swap are not available;
# FSP r_mram "erase" is 32-byte page writes of 0xFF). This is a software FS block
# size chosen to match the 8 KB MRAM block unit; it is also the OTP NV counter
# sector size.
set(FLASH_AREA_IMAGE_SECTOR_SIZE "0x2000" CACHE STRING "FS block granularity (8 KB MRAM block unit)")

set(FLASH_PAGE_SIZE "${RA_FLASH_PAGE_BUFFER_SIZE}" CACHE STRING "MRAM page buffer size in bytes")

# MRAM supports programming 1-32 bytes at a time, TF-M requires a fixed program unit.
set(FLASH_PROGRAM_UNIT "0x04" CACHE STRING "MRAM minimum write size in bytes")

#===================== SRAM geometry ===========================================#

set(SRAM_BASE_ADDRESS "0x22000000" CACHE STRING "SRAM secure alias base")
set(TOTAL_RAM_SIZE    "0x1A0000"   CACHE STRING "SRAM size (SRAM0-3)")

#===================== Non-secure external address windows =====================#

# OSPI0/1 CS0/CS1 and SDRAM. The external address space is hardwired non-secure
# in the IDAU (HW manual OSPI 44.1) and this cannot be overridden by software;
# consumed by check_config.cmake to validate any image relocated off-chip.
set(RA_NS_WINDOW_STARTS "0x88000000" "0x98000000" "0x74000000" "0x7c000000" "0x6c000000" CACHE STRING "Start addresses of the non-secure external windows (OSPI0/1 CS0/CS1, SDRAM)")
set(RA_NS_WINDOW_ENDS   "0x8c000000" "0x9c000000" "0x76000000" "0x7e000000" "0x6e000000" CACHE STRING "End (exclusive) addresses of the non-secure external windows (must match RA_NS_WINDOW_STARTS)")
