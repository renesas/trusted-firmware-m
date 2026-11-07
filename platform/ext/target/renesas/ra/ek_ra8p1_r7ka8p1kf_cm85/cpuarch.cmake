#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(RENESAS_RA_COMMON_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../common)
set(RENESAS_RA_SOC_SOURCE_DIR    ${RENESAS_RA_COMMON_SOURCE_DIR}/ra8p1)

# Marks this as the secure-side build.
set(S_BUILD ON)

# The secure image (SPE) runs on CPU0 (Cortex-M85) only, so the core is fixed here.
# TODO: Support building the secure image for the CPU1 (cm33) core.
set(RA_CORE "cm85")

include(${RENESAS_RA_SOC_SOURCE_DIR}/cpuarch.cmake)
