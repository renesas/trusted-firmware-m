#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# RA8M2-specific validation.
# MRAM programs through a page buffer, so FLASH_PROGRAM_UNIT -- the minimum
# programmable unit -- cannot exceed it (HW manual MRAM 59.4.2).
if(FLASH_PROGRAM_UNIT GREATER RA_FLASH_PAGE_BUFFER_SIZE OR FLASH_PROGRAM_UNIT LESS 1)
    message(FATAL_ERROR "${RA_SOC_NAME}: FLASH_PROGRAM_UNIT (${FLASH_PROGRAM_UNIT}) must be between 1 and ${RA_FLASH_PAGE_BUFFER_SIZE} bytes (MRAM page buffer size).")
endif()

include(${RENESAS_RA_COMMON_SOURCE_DIR}/core/check_config.cmake)
