#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# This file is exported to NS side during CMake installation phase and renamed
# to CMakeLists.txt. It instructs how to build a platform on non-secure side.

# Renesas RA FSP library definitions
set(FSP_BOARD_TARGET ra8p1_ek)
set(FSP_SOC_TARGET ra8p1)

include(${RENESAS_RA_COMMON_SOURCE_DIR}/core/ns/common.cmake)
