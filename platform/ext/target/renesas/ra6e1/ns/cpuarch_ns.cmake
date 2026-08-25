#-------------------------------------------------------------------------------
# Copyright (c) 2024, Renesas Electronics Corporation. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause
#
# Non-Secure CPU Architecture Configuration for RA6M4
#-------------------------------------------------------------------------------

# This file configures the non-secure side platform for RA6M4
# It supports using an external FSP-based FreeRTOS project as the NS application

set(PLATFORM_DIR  ${CMAKE_CURRENT_LIST_DIR}/..)
set(PLATFORM_PATH ${CMAKE_CURRENT_LIST_DIR}/..)

# Include parent platform CPU architecture (Cortex-M33 with TrustZone)
include(${CMAKE_CURRENT_LIST_DIR}/../cpuarch.cmake)
