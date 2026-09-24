#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Build TF-M's crypto from FSP's Mbed TLS instead of downloading upstream.
#
# FSP ships Mbed TLS with its own changes to the PSA core - the GCM finish/verify handling its
# the GCM ALT relies on, vendor key plumbing, ML-DSA/ML-KEM - and its rm_psa_crypto *_ALT
# sources are written against that core. Pairing the ALT sources with upstream Mbed TLS
# broke multi-part GCM and exposed contract gaps FSP's own cipher layer normally covers
# (DECISIONS D041/D042). So the port uses FSP's tree.
#
# FSP's tree is include/ + library/ only - no CMake, no scripts/config.py, no 3rdparty/p256-m -
# and TF-M consumes Mbed TLS through add_subdirectory(${MBEDCRYPTO_PATH}) and config.py. So
# this builds an OVERLAY in the build directory:
#
#   upstream Mbed TLS at FSP's version  - scaffolding: CMake, scripts, framework, p256-m
#   + FSP's include/ and library/        - copied over it, line endings normalised to LF
#   + the TF-M patches FSP lacks          - ../mbedtls/*.patch, applied in order
#
# FSP's tree already carries TF-M's builtin-key-loader patch (0001) and the CC3XX one (0005),
# so only 0003, 0004, 0006 and 0007 are applied here - the TF-M 2.2.2 versions, which match
# 3.6.5+. 0002 (code sharing) is not used on this port. GEN_FILES is OFF in TF-M's crypto
# CMake, so FSP's pre-generated driver wrappers are compiled as shipped.
#
# The overlay is rebuilt on every configure from FSP_S_APP_DIR, so a regenerated e2 project
# flows through. The upstream clone is fetched once per build directory.
#-------------------------------------------------------------------------------

set(_fsp_mbedtls "${FSP_S_APP_DIR}/ra/arm/mbedtls")
if(NOT EXISTS "${_fsp_mbedtls}/library/psa_crypto.c")
    message(FATAL_ERROR
        "RA8M2: FSP's Mbed TLS is not in ${_fsp_mbedtls}.\n"
        "The port builds TF-M's crypto from it. Add the Crypto stack (rm_psa_crypto) to the "
        "secure e2 project, regenerate and build once - or configure with "
        "-DRA8M2_FSP_MBEDTLS=OFF to use upstream Mbed TLS.")
endif()

# Scaffolding version = FSP's version. The overlay replaces include/ and library/ wholesale,
# so the CMake source lists have to describe the same release.
file(STRINGS "${_fsp_mbedtls}/include/mbedtls/build_info.h" _ver_line
     REGEX "^#define[ \t]+MBEDTLS_VERSION_STRING[ \t]+\"")
string(REGEX REPLACE ".*\"([0-9.]+)\".*" "\\1" _fsp_mbedtls_version "${_ver_line}")
if(NOT _fsp_mbedtls_version)
    message(FATAL_ERROR "RA8M2: could not read MBEDTLS_VERSION_STRING from FSP's build_info.h")
endif()
set(_tag "mbedtls-${_fsp_mbedtls_version}")

set(_upstream "${CMAKE_BINARY_DIR}/lib/ext/fsp_mbedtls_upstream-${_fsp_mbedtls_version}")
set(_overlay  "${CMAKE_BINARY_DIR}/lib/ext/fsp_mbedtls")

find_package(Git REQUIRED)

# The platform config is processed BEFORE config/config_base.cmake, so MBEDCRYPTO_GIT_REMOTE
# is not set yet unless it came from the command line.
if(NOT MBEDCRYPTO_GIT_REMOTE)
    set(MBEDCRYPTO_GIT_REMOTE "https://github.com/Mbed-TLS/mbedtls.git")
endif()

if(NOT EXISTS "${_upstream}/CMakeLists.txt")
    message(STATUS "RA8M2: fetching ${_tag} scaffolding for FSP's Mbed TLS")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} clone -q --depth 1 --branch ${_tag}
                ${MBEDCRYPTO_GIT_REMOTE} "${_upstream}"
        RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "RA8M2: cloning ${_tag} from ${MBEDCRYPTO_GIT_REMOTE} failed")
    endif()
    execute_process(
        COMMAND ${GIT_EXECUTABLE} -C "${_upstream}" submodule update -q --init --depth 1 framework
        RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "RA8M2: fetching the Mbed TLS framework submodule failed")
    endif()
endif()

# Fresh overlay every configure.
file(REMOVE_RECURSE "${_overlay}")
foreach(_item CMakeLists.txt cmake include library 3rdparty scripts framework pkgconfig)
    if(EXISTS "${_upstream}/${_item}")
        file(COPY "${_upstream}/${_item}" DESTINATION "${_overlay}")
    endif()
endforeach()

# FSP's include/ and library/ over it. LF-normalised so the patches below apply; FSP ships
# CRLF.
file(GLOB_RECURSE _fsp_files RELATIVE "${_fsp_mbedtls}"
     "${_fsp_mbedtls}/include/*" "${_fsp_mbedtls}/library/*")
foreach(_f ${_fsp_files})
    file(READ "${_fsp_mbedtls}/${_f}" _content)
    string(REPLACE "\r\n" "\n" _content "${_content}")
    file(WRITE "${_overlay}/${_f}" "${_content}")
endforeach()

# TF-M patches FSP's tree lacks.
#
# GIT_CEILING_DIRECTORIES stops git discovering the enclosing TF-M checkout - the build
# directory is inside it - which would make it resolve the patch paths against the TF-M work
# tree instead of the overlay. Running outside a repository, git apply works relative to the
# working directory, and the overlay stays a plain directory (a .git here would survive
# file(REMOVE_RECURSE) on Windows and stale the next regeneration).
file(GLOB _patches "${CMAKE_CURRENT_LIST_DIR}/../mbedtls/*.patch")
list(SORT _patches)
get_filename_component(_overlay_parent "${_overlay}" DIRECTORY)
foreach(_p ${_patches})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env "GIT_CEILING_DIRECTORIES=${_overlay_parent}"
                ${GIT_EXECUTABLE} apply --whitespace=nowarn "${_p}"
        WORKING_DIRECTORY "${_overlay}"
        RESULT_VARIABLE _rc
        ERROR_VARIABLE _err)
    if(NOT _rc EQUAL 0)
        get_filename_component(_pn "${_p}" NAME)
        message(FATAL_ERROR
            "RA8M2: ${_pn} does not apply to FSP's Mbed TLS ${_fsp_mbedtls_version}:\n${_err}\n"
            "A newer FSP may already carry it, or have moved its context - check "
            "${CMAKE_CURRENT_LIST_DIR}/../mbedtls.")
    endif()
endforeach()

set(MBEDCRYPTO_PATH "${_overlay}" CACHE PATH "FSP's Mbed TLS (overlay, see fsp_mbedtls.cmake)" FORCE)
set(MBEDCRYPTO_VERSION "${_tag}-fsp" CACHE STRING "FSP's Mbed TLS" FORCE)
message(STATUS "RA8M2: crypto from FSP's Mbed TLS ${_fsp_mbedtls_version} - ${_overlay}")
