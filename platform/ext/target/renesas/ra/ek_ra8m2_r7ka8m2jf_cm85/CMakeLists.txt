#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Renesas RA FSP library definitions (SoC + board selectors used by the shared
# build logic in common/core/common.cmake, which everything below delegates
# to: FSP library wiring, map definitions, scatter file, platform sources and
# the NS-side install).
set(FSP_SOC_TARGET ra8m2)
set(FSP_BOARD_TARGET ra8m2_ek)

include(${RENESAS_RA_COMMON_SOURCE_DIR}/core/common.cmake)
