#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Configuration shared by every Renesas RA board, included last by each board's
# config.cmake

#===================== Which non-secure clients the SPE serves =================#
#
# These two are the platform's view of its topology, and everything else is
# derived from them:
#
#   RA_CM85_NS_PRESENT  CPU0 (Cortex-M85) runs a local non-secure image that
#                       calls into the SPE across the TrustZone transition.
#   RA_CM33_NS_PRESENT  CPU1 (Cortex-M33) runs a remote non-secure image that
#                       calls into the SPE over the mailbox.

#===================== Derived TF-M switches ===================================#

set(CONFIG_TFM_USE_TRUSTZONE   ${RA_CM85_NS_PRESENT} CACHE BOOL "Use TrustZone to transition between NSPE and SPE")
set(TFM_MULTI_CORE_TOPOLOGY    ${RA_CM33_NS_PRESENT} CACHE BOOL "Build for a dual-cpu architecture (CPU1 as a remote PSA client)")

#===================== Platform-wide TF-M configuration ========================#

# BL2 is unsupported: check_config.cmake rejects BL2=ON.
set(BL2                        OFF   CACHE BOOL   "Whether to build BL2")
set(FLASH_AREA_BL2_OFFSET      "0x0" CACHE STRING "BL2 offset (BL2 unsupported)")
set(FLASH_AREA_BL2_SIZE        "0x0" CACHE STRING "BL2 size (BL2 unsupported)")

set(CONFIG_TFM_SPM_BACKEND     IPC CACHE STRING "The SPM backend")
set(CONFIG_TFM_INCLUDE_STDLIBC ON  CACHE BOOL   "Include standard C libraries and startup code or not")
set(CONFIG_TFM_ENABLE_FP       ON  CACHE BOOL   "Enable floating point support")
set(TFM_PARTITION_PLATFORM     ON  CACHE BOOL   "Enable Platform partition" FORCE)

#===================== Generated sources =======================================#

# The RA linker-script templates are only generated for the secure image; the
# path below points into the TF-M source tree and is meaningless to an NS build.
if(S_BUILD)
    if(NOT TFM_EXTRA_GENERATED_FILE_LIST_PATH)
        set(TFM_EXTRA_GENERATED_FILE_LIST_PATH "" CACHE PATH "List of Paths to extra generated file list. Appended to standard TFM generated file list.")
    endif()

    list(APPEND TFM_EXTRA_GENERATED_FILE_LIST_PATH "${RENESAS_RA_COMMON_SOURCE_DIR}/core/generated_file_list.yaml")
endif()
