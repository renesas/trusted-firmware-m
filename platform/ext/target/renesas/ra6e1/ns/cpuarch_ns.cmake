#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# RA6E1 CPU architecture for the NON-SECURE build.
#
# Included by ns_app/CMakeLists.txt BEFORE project(). It has to be that early: the NS
# toolchain file builds -mcpu/-march from TFM_SYSTEM_PROCESSOR and TFM_SYSTEM_ARCHITECTURE,
# and CMake fixes the toolchain during project(). Included after, every compile gets a bare
# "-march=" and gcc stops with "missing argument to '-march='".
#
# The values must be the SAME ones the secure side used - this file exists to share them,
# not to restate them. cpuarch.cmake is installed beside this file for that reason.
#
# Two layouts, one file: in the source tree this lives in ra6e1/ns/ with cpuarch.cmake one
# level up; installed, both are flattened into <api_ns>/platform/. The previous version of
# this file only handled the first, so it could never have worked in the build it was
# written for.
#-------------------------------------------------------------------------------

if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/cpuarch.cmake)
    # Installed tree: <api_ns>/platform/
    set(PLATFORM_DIR  ${CMAKE_CURRENT_LIST_DIR})
    include(${CMAKE_CURRENT_LIST_DIR}/cpuarch.cmake)
else()
    # Source tree: platform/ext/target/renesas/ra6e1/ns/
    set(PLATFORM_DIR  ${CMAKE_CURRENT_LIST_DIR}/..)
    include(${CMAKE_CURRENT_LIST_DIR}/../cpuarch.cmake)
endif()
set(PLATFORM_PATH ${PLATFORM_DIR})
