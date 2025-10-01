#-------------------------------------------------------------------------------
# Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# config.cmake is used for options that must be set by a specific platform. It
# is not meant to be a general configuration file

############################ Platform ##########################################

set(TFM_MULTI_CORE_TOPOLOGY    OFF         CACHE BOOL      "Whether to build for a dual-core platform")
set(CONFIG_TFM_USE_TRUSTZONE   ON          CACHE BOOL      "Enable use of TrustZone to transition between NSPE and SPE")

############################ BL2 ###############################################

if(BL2)
    set(BL2_TRAILER_SIZE                    0x800       CACHE STRING    "Trailer size")
else()
    # No header if no bootloader, but keep IMAGE_CODE_SIZE the same
    set(BL2_TRAILER_SIZE                    0x800       CACHE STRING    "Trailer size")
endif()

set(MCUBOOT_IMAGE_NUMBER                2           CACHE STRING    "Whether to combine S and NS into either 1 image, or sign each seperately")
set(MCUBOOT_ALIGN_VAL                   16          CACHE STRING    "Align option for images")

################################## Dependencies ################################

# Path to FSP project (relative to this file or absolute)
if(NOT DEFINED FSP_ROOT_DIR)
    # Default path - adjust as needed
    set(FSP_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../fsp_cmake/FSP_Project_ra6m4" CACHE STRING "FSP root directory")
endif()

############################ Platform Drivers ##################################

set(PLATFORM_DEFAULT_UART_STDOUT        ON)
set(UART0_BASE_S                        0x40070000) # SCI0 base address
set(PLATFORM_DEFAULT_SYSTEM_RESET       ON)

############################ Flash Layout ######################################

# Renesas RA6M4: 1MB Flash, 256KB RAM
# Flash layout:
#   BL2:        0x00000000 - 0x0001FFFF (128KB)
#   Secure:     0x00020000 - 0x0007FFFF (384KB)
#   Non-Secure: 0x00080000 - 0x000FFFFF (512KB)

set(FLASH_AREA_BL2_OFFSET               0x0         CACHE STRING    "BL2 area offset")
set(FLASH_AREA_BL2_SIZE                 0x20000     CACHE STRING    "BL2 area size (128KB)")

set(FLASH_S_PARTITION_OFFSET            0x20000     CACHE STRING    "Secure partition offset")
set(FLASH_S_PARTITION_SIZE              0x60000     CACHE STRING    "Secure partition size (384KB)")

set(FLASH_NS_PARTITION_OFFSET           0x80000     CACHE STRING    "Non-secure partition offset")
set(FLASH_NS_PARTITION_SIZE             0x80000     CACHE STRING    "Non-secure partition size (512KB)")

set(FLASH_BASE_ADDRESS                  0x00000000  CACHE STRING    "Flash base address")
set(FLASH_TOTAL_SIZE                    0x00100000  CACHE STRING    "Total flash size (1MB)")

# Data Flash for PS/ITS/NV
set(FLASH_DATA_BASE_ADDRESS             0x08000000  CACHE STRING    "Data flash base")
set(FLASH_DATA_SIZE                     0x2000      CACHE STRING    "Data flash size (8KB)")

############################ Memory Layout #####################################

set(S_RAM_CODE_START                    0x20000000  CACHE STRING    "Secure RAM code start")
set(S_RAM_CODE_SIZE                     0x20000     CACHE STRING    "Secure RAM size (128KB)")

set(NS_RAM_CODE_START                   0x20020000  CACHE STRING    "Non-secure RAM code start")
set(NS_RAM_CODE_SIZE                    0x20000     CACHE STRING    "Non-secure RAM size (128KB)")

########################## FSP Module Integration ##############################

# Use modular FSP libraries
set(USE_FSP_MODULES                     ON          CACHE BOOL      "Use modular FSP libraries")

# List of FSP modules to include
set(FSP_MODULES                         "bsp;uart;flash" CACHE STRING "FSP modules to link")