#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# RA8M2 CPU architecture for the non-secure build.
#
# ns_app/CMakeLists.txt includes this before project(). It has to be that early: the NS
# toolchain file builds -mcpu/-march from TFM_SYSTEM_PROCESSOR and TFM_SYSTEM_ARCHITECTURE,
# and CMake fixes the toolchain during project(). Included later, every compile gets a bare
# "-march=" and gcc stops with "missing argument to '-march='".
#
# The values must be the ones the secure side used, so this file includes cpuarch.cmake,
# installed beside it, rather than restating them.
#
# Two layouts: in the source tree this file is in ra8m2/ns/ with cpuarch.cmake one level up;
# installed, both are flattened into <api_ns>/platform/.
#-------------------------------------------------------------------------------

if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/cpuarch.cmake)
    # Installed tree: <api_ns>/platform/
    set(PLATFORM_DIR  ${CMAKE_CURRENT_LIST_DIR})
    include(${CMAKE_CURRENT_LIST_DIR}/cpuarch.cmake)
else()
    # Source tree: platform/ext/target/renesas/ra8m2/ns/
    set(PLATFORM_DIR  ${CMAKE_CURRENT_LIST_DIR}/..)
    include(${CMAKE_CURRENT_LIST_DIR}/../cpuarch.cmake)
endif()
set(PLATFORM_PATH ${PLATFORM_DIR})
