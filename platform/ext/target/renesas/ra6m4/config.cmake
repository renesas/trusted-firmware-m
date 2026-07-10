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

############################ TF-M Secure Partitions #############################

# Enable TF-M secure services
set(TFM_PARTITION_CRYPTO                ON          CACHE BOOL      "Enable Crypto partition")
set(TFM_PARTITION_INTERNAL_TRUSTED_STORAGE ON       CACHE BOOL      "Enable Internal Trusted Storage partition")
set(TFM_PARTITION_PROTECTED_STORAGE     ON          CACHE BOOL      "Enable Protected Storage partition")
set(TFM_PARTITION_INITIAL_ATTESTATION   ON          CACHE BOOL      "Enable Initial Attestation partition")
set(TFM_PARTITION_PLATFORM              ON          CACHE BOOL      "Enable Platform partition")

# Logging configuration
set(TFM_PARTITION_LOG_LEVEL             TFM_PARTITION_LOG_LEVEL_INFO    CACHE STRING    "Partition log level")
set(TFM_SPM_LOG_LEVEL                   TFM_SPM_LOG_LEVEL_INFO          CACHE STRING    "SPM log level")

# stdout backend selection.
#   RA6M4_STDOUT_RTT ON  -> TF-M/MCUboot logs go to SEGGER RTT over J-Link
#                           (no UART wiring; the common uart_stdout.c is disabled
#                            and platform/.../rtt/rtt_stdout.c is compiled instead).
#   RA6M4_STDOUT_RTT OFF -> use the FSP SCI UART via the common uart_stdout.c.
# The FSP UART driver (cmsis_drivers/Driver_USART.c) is untouched in both cases;
# switching is just this one flag.
set(RA6M4_STDOUT_RTT                    ON                              CACHE BOOL      "Route TF-M/MCUboot stdout to SEGGER RTT instead of SCI UART")
if(RA6M4_STDOUT_RTT)
    set(PLATFORM_DEFAULT_UART_STDOUT    OFF                             CACHE BOOL      "Use UART for stdout" FORCE)
else()
    set(PLATFORM_DEFAULT_UART_STDOUT    ON                              CACHE BOOL      "Use UART for stdout" FORCE)
endif()

# Crypto configuration
set(CRYPTO_HW_ACCELERATOR               OFF         CACHE BOOL      "Use hardware crypto acceleration if available")
set(CRYPTO_TFM_BUILTIN_KEYS_DRIVER      ON          CACHE BOOL      "Enable TF-M builtin keys driver")

# Storage configuration
set(PS_CRYPTO_AEAD_ALG                  PSA_ALG_GCM CACHE STRING    "AEAD algorithm for Protected Storage")
set(PS_ENCRYPTION                       ON          CACHE BOOL      "Enable encryption for Protected Storage")

# Attestation configuration
set(SYMMETRIC_INITIAL_ATTESTATION       OFF         CACHE BOOL      "Use symmetric crypto for attestation (OFF = asymmetric)")
set(ATTEST_KEY_BITS                     256         CACHE STRING    "Size of attestation key in bits")
set(PSA_INITIAL_ATTEST_MAX_TOKEN_SIZE   0x250       CACHE STRING    "Maximum attestation token size")
set(PLATFORM_DEFAULT_ATTEST_HAL         OFF         CACHE BOOL      "Use platform-specific attestation HAL")

############################ BL2 ###############################################

# Use Renesas-specific MCUboot (includes RA6M4 optimizations)
# Set to "DOWNLOAD" to automatically fetch from Renesas GitHub
# Or set to a local path if you have it cloned already
if(NOT DEFINED MCUBOOT_PATH)
    set(MCUBOOT_PATH                    "DOWNLOAD"  CACHE PATH      "Path to MCUboot (or DOWNLOAD to fetch Renesas version automatically)")
endif()

# Override MCUboot repository and version to use Renesas fork
set(MCUBOOT_GIT_REPOSITORY              "https://github.com/renesas/mcuboot.git" CACHE STRING "MCUboot repository URL")
set(MCUBOOT_VERSION                     "2.1.0+renesas.3" CACHE STRING "MCUboot version tag")


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

# Keep the stdout backend consistent with RA6M4_STDOUT_RTT set above
# (do not force UART stdout on when RTT is selected).
if(NOT RA6M4_STDOUT_RTT)
    set(PLATFORM_DEFAULT_UART_STDOUT    ON)
endif()
set(UART0_BASE_S                        0x40070000) # SCI0 base address
set(PLATFORM_DEFAULT_SYSTEM_RESET       ON)

############################ Flash Layout ######################################

# Renesas RA6M4: 1MB Flash, 256KB RAM
# Flash layout depends on whether BL2 is enabled
#
# IMPORTANT: For the ra6m4 platform, the AUTHORITATIVE flash/RAM layout is the
# static header flash_layout.h (+ region_defs.h), which MCUboot and TF-M use.
# The FLASH_*_PARTITION_* cache variables below are NOT consumed by this platform
# or by common code (only mps3 platforms read them) - they are kept only to
# describe intent and MUST be kept consistent with flash_layout.h. The values
# here were previously stale; they now match flash_layout.h:
#   BL2:              0x00000 - 0x1FFFF (128KB)
#   Secure Primary:   0x20000 - 0x4FFFF (192KB)  [FLASH_AREA_0]
#   NS Primary:       0x50000 - 0x6FFFF (128KB)  [FLASH_AREA_1]
#   Secure Secondary: 0x70000 - 0x9FFFF (192KB)  [FLASH_AREA_2]
#   NS Secondary:     0xA0000 - 0xBFFFF (128KB)  [FLASH_AREA_3]
#   Scratch:          0xC0000 - 0xFFFFF (256KB)
# NS code runs at NS_CODE_START = 0x50000 + BL2_HEADER 0x400 = 0x50400.

set(FLASH_AREA_BL2_OFFSET               0x0         CACHE STRING    "BL2 area offset")
set(FLASH_AREA_BL2_SIZE                 0x20000     CACHE STRING    "BL2 area size (128KB)")

if(BL2)
    set(FLASH_S_PARTITION_OFFSET        0x20000     CACHE STRING    "Secure partition offset")
    set(FLASH_S_PARTITION_SIZE          0x30000     CACHE STRING    "Secure partition size (192KB)")
    set(FLASH_NS_PARTITION_OFFSET       0x50000     CACHE STRING    "Non-secure partition offset")
    set(FLASH_NS_PARTITION_SIZE         0x20000     CACHE STRING    "Non-secure partition size (128KB)")
else()
    # Without BL2:
    #   Secure:     0x00000000 - 0x0009FFFF (640KB)
    #   Non-Secure: 0x000A0000 - 0x000FFFFF (384KB)
    set(FLASH_S_PARTITION_OFFSET        0x0         CACHE STRING    "Secure partition offset")
    set(FLASH_S_PARTITION_SIZE          0xA0000     CACHE STRING    "Secure partition size (640KB)")
    set(FLASH_NS_PARTITION_OFFSET       0xA0000     CACHE STRING    "Non-secure partition offset")
    set(FLASH_NS_PARTITION_SIZE         0x60000     CACHE STRING    "Non-secure partition size (384KB)")
endif()

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

############################ MbedTLS Configuration ##############################

# Disable pthread search in MbedTLS (not applicable for bare-metal)
set(THREADS_PREFER_PTHREAD_FLAG         OFF         CACHE BOOL      "Disable pthread for MbedTLS")
set(CMAKE_THREAD_LIBS_INIT              ""          CACHE STRING    "No threading library for bare-metal")
set(CMAKE_HAVE_THREADS_LIBRARY          FALSE       CACHE BOOL      "No threads library available")
set(CMAKE_USE_PTHREADS_INIT             FALSE       CACHE BOOL      "Do not use pthreads")