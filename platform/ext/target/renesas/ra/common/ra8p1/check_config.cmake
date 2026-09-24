#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Hardware facts per the RA8P1 HW manual: Security Features 52.3.3.5 (32 KB
# MRAM / 8 KB SRAM security-attribution granularity), OSPI 44.1 (external
# address space is hardwired non-secure).
set(RA_SOC_NAME "RA8P1")
set(RA_ROM_ATTR_GRANULARITY "0x8000")
set(RA_SRAM_ATTR_GRANULARITY "0x2000")
set(RA_ROM_NS_ALIAS "0x12000000")

# The RA8P1 MRAM page buffer is 32 bytes (HW manual MRAM chapter), so
# FLASH_PROGRAM_UNIT -- the minimum programmable unit -- cannot exceed it.
if(FLASH_PROGRAM_UNIT GREATER 32 OR FLASH_PROGRAM_UNIT LESS 1)
    message(FATAL_ERROR "${RA_SOC_NAME}: FLASH_PROGRAM_UNIT (${FLASH_PROGRAM_UNIT}) must be between 1 and 32 bytes (MRAM page buffer size).")
endif()

include(${RENESAS_RA_COMMON_SOURCE_DIR}/core/check_config.cmake)
