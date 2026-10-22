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

#===================== Flash instances =========================================#
#
# Instance 0 is the on-chip MRAM (FLASH0_*, the SoC config.cmake). Instance 1
# (FLASH1_*) is the flash an NS image is relocated to by moving its ROM alias
# off the MRAM alias, e.g. -DCPU0_NS_ROM_ALIAS_BASE=0x88000000 (see
# RA_NS_WINDOW_* in the SoC config.cmake). Which device it is (MRAM or OSPI
# flash) is a build time choice made through its driver and geometry. Its
# geometry defaults to the MRAM one and is overridable on the command line.
#
# FLASH1_DEV_NAME is the name of its CMSIS flash driver. While it is empty no
# driver exists for it, so the areas in it fall back to the MRAM driver
# (check_config.cmake warns about that with BL2).
set(FLASH1_DEV_NAME     ""                        CACHE STRING "CMSIS flash driver of flash instance 1 (empty: fall back to the MRAM driver)")
set(FLASH1_SECTOR_SIZE  "${FLASH0_SECTOR_SIZE}"   CACHE STRING "Flash instance 1 FS block granularity")
set(FLASH1_PROGRAM_UNIT "${FLASH0_PROGRAM_UNIT}"  CACHE STRING "Flash instance 1 minimum write size in bytes")

#===================== Platform-wide TF-M configuration ========================#

# BL2 (MCUboot) is opt-in. With BL2 every image is signed separately: S, the
# CPU0 NS image and, with RA_CM33_NS_PRESENT, the CPU1 NS image
# (MCUBOOT_IMAGE_NUMBER 2 or 3). BL2 only verifies the CPU1 image, the SPE
# starts CPU1. Only overwrite upgrades are supported: MRAM has no scratch area
# or bank swap, and the secondary slot of an image sits next to its primary
# slot, in the same flash instance. The default MCUboot flash map handles at
# most 2 images, so the platform provides its own
# (common/core/bl2/flash_map_bl2.c). check_config.cmake enforces this.
# BL2 and the SPE sign every image separately, so no combined S+NS image is
# built (see core/ns/common.cmake).
set(BL2                        OFF   CACHE BOOL   "Whether to build BL2")

if(BL2)
    set(FLASH_AREA_BL2_OFFSET   "0x0"     CACHE STRING "BL2 offset")
    set(FLASH_AREA_BL2_SIZE     "0x10000" CACHE STRING "BL2 size")

    if(RA_CM33_NS_PRESENT)
        set(MCUBOOT_IMAGE_NUMBER 3                CACHE STRING "Number of images managed by MCUboot (S, CPU0 NS, CPU1 NS)")
    else()
        set(MCUBOOT_IMAGE_NUMBER 2                CACHE STRING "Whether to combine S and NS into either 1 image, or sign each seperately")
    endif()
    set(DEFAULT_MCUBOOT_FLASH_MAP OFF             CACHE BOOL   "Whether to use the default flash map defined by TF-M project")
    set(MCUBOOT_UPGRADE_STRATEGY "OVERWRITE_ONLY" CACHE STRING "Upgrade strategy for images")
    set(MCUBOOT_SIGNATURE_TYPE   "EC-P256"        CACHE STRING "Algorithm to use for signature validation [RSA-2048, RSA-3072, EC-P256, EC-P384]")
    set(MCUBOOT_HW_KEY           ON               CACHE BOOL   "Whether to embed the entire public key in the image metadata instead of the hash only")
    # MCUboot trailer writes must be a multiple of the program unit of every
    # flash instance (FLASH_PROGRAM_UNIT, the largest one); check_config.cmake
    # enforces this.
    set(MCUBOOT_ALIGN_VAL        4                CACHE STRING "align option for mcuboot and build image with imgtool [1, 2, 4, 8, 16, 32]")
    # CPU1 NS image (MCUboot image 2). It is signed with the S key: the default
    # provisioning stores the S key hash in BL2_ROTPK_2.
    set(MCUBOOT_IMAGE_VERSION_CPU1_NS    0.0.0    CACHE STRING "Version number of the CPU1 NS image")
    set(MCUBOOT_SECURITY_COUNTER_CPU1_NS 1        CACHE STRING "Security counter for the CPU1 NS image. auto sets it to MCUBOOT_IMAGE_VERSION_CPU1_NS")
    # The FWU partition (TFM_PARTITION_FIRMWARE_UPDATE) writes the secondary
    # slots through the platform flash map; one FWU component per MCUboot image.
    set(PLATFORM_HAS_FIRMWARE_UPDATE_SUPPORT ON)
else()
    set(FLASH_AREA_BL2_OFFSET   "0x0"    CACHE STRING "BL2 offset")
    set(FLASH_AREA_BL2_SIZE     "0x0"    CACHE STRING "BL2 size")
endif()

set(CONFIG_TFM_SPM_BACKEND     IPC CACHE STRING "The SPM backend")
set(CONFIG_TFM_INCLUDE_STDLIBC OFF CACHE BOOL   "Include standard C libraries and startup code or not")
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
