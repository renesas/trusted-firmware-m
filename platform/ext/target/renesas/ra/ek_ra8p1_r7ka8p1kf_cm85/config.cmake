#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

include(${RENESAS_RA_SOC_SOURCE_DIR}/config.cmake)

# An NS image can be relocated off-chip (e.g. onto the OSPI0 CS0 of the EK-RA8P1)
# by overriding its ROM alias, e.g. -DCPU0_NS_ROM_ALIAS_BASE=0x88000000 (and
# CPU1_NS_ROM_ALIAS_BASE for the CPU1 image). Its slots are then in the external
# flash (flash instance 1, FLASH1_*), at offset 0 of the window unless
# CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET / CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET
# say otherwise. Set FLASH1_DEV_NAME to the CMSIS driver of the external
# flash; until there is one, BL2 falls back to the MRAM driver (see
# check_config.cmake).

#===================== Used non-secure clients =================================#
set(RA_CM85_NS_PRESENT ON CACHE BOOL "Serve a local TrustZone NS client on CPU0 (Cortex-M85)")
set(RA_CM33_NS_PRESENT ON CACHE BOOL "Serve a remote mailbox NS client on CPU1 (Cortex-M33)")

#===================== Secure image (SPE, CPU0 Cortex-M85) =====================#
# MRAM map, offsets from the MRAM base:
#   BL2 = ON  (MCUboot, OVERWRITE_ONLY, no scratch; 3 images with the CPU1 image):
#     0x00000 BL2 (64 KB) | 0x10000 S primary | 0x3C000 S secondary |
#     0x68000 PS / ITS / OTP+NV counters | 0x78000 secure boundary |
#     0x78000 CPU0 NS primary | 0x9A000 CPU0 NS secondary |
#     0xBC000 CPU1 (CM33) NS primary | 0xDE000 CPU1 NS secondary | 0x100000
#     (CPU0 and CPU1 NS slots are 0x22000 each; without the CPU1 image the CPU0
#     NS slots are 0x28000)
#   BL2 = OFF: S slot | PS / ITS / OTP+NV counters | CPU0 NS slot | CPU1 NS slot
# The secure boundary (FLASH_S_REGION_SIZE, derived in post_config.cmake)
# must match the MRAM secure/non-secure split programmed in the device.
if(BL2)
    set(FLASH_S_PARTITION_SIZE          "0x2C000"             CACHE STRING "Secure image slot size (primary and secondary)")
else()
    set(FLASH_S_PARTITION_SIZE          "0x50000"             CACHE STRING "Secure image slot size")
endif()
set(FLASH_PS_AREA_SIZE              "0x8000"              CACHE STRING "Protected Storage area size")
set(FLASH_ITS_AREA_SIZE             "0x4000"              CACHE STRING "Internal Trusted Storage area size")
set(FLASH_OTP_NV_COUNTERS_AREA_SIZE "0x4000"              CACHE STRING "OTP/NV counters area size")

set(S_DATA_SIZE     "0x80000" CACHE STRING "Secure RAM size")
set(S_RAM_CODE_SIZE "0x1000"  CACHE STRING "Secure RAM size for RAM-resident MRAM programming code")

set(S_ROM_ALIAS_BASE "${FLASH_BASE_ADDRESS}")
set(S_RAM_ALIAS_BASE "${SRAM_BASE_ADDRESS}")

#===================== NS image, CPU0 Cortex-M85 (TrustZone) ===================#
# CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET and CPU0_NS_RAM_OFFSET are derived in
# common/core/post_config.cmake (overridable on the command line).
set(CPU0_NS_ROM_ALIAS_BASE                 "${FLASH_BASE_NS}" CACHE STRING "CPU0 NS ROM alias base")
set(CPU0_NS_RAM_ALIAS_BASE                 "${RA_SRAM_NS_ALIAS}")

if(RA_CM85_NS_PRESENT)
    if(BL2)
        if(RA_CM33_NS_PRESENT)
            set(FLASH_CPU0_NS_PARTITION_SIZE "0x22000"            CACHE STRING "CPU0 NS image slot size (primary and secondary)")
        else()
            set(FLASH_CPU0_NS_PARTITION_SIZE "0x28000"            CACHE STRING "CPU0 NS image slot size (primary and secondary)")
        endif()
    else()
        set(FLASH_CPU0_NS_PARTITION_SIZE   "0x60000"              CACHE STRING "CPU0 NS image slot size")
    endif()
    set(CPU0_NS_RAM_DATA_SIZE              "0x80000"              CACHE STRING "CPU0 NS RAM data size")
else()
    set(FLASH_CPU0_NS_PARTITION_SIZE       "0x0")
    set(CPU0_NS_RAM_DATA_SIZE              "0x0")
endif()

#===================== NS image, CPU1 Cortex-M33 (Mailbox) =====================#
# CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET and CPU1_NS_RAM_OFFSET are derived in
# common/core/post_config.cmake (overridable on the command line).
set(CPU1_NS_ROM_ALIAS_BASE                 "${FLASH_BASE_NS}" CACHE STRING "CPU1 NS ROM alias base")
set(CPU1_NS_RAM_ALIAS_BASE                 "${RA_SRAM_NS_ALIAS}")

if(RA_CM33_NS_PRESENT)
    if(BL2)
        # Primary and secondary slot, like the CPU0 NS image.
        set(FLASH_CPU1_NS_PARTITION_SIZE   "0x22000"              CACHE STRING "CPU1 NS image slot size (primary and secondary)")
    else()
        set(FLASH_CPU1_NS_PARTITION_SIZE   "0x40000"              CACHE STRING "CPU1 NS image slot size")
    endif()
    set(CPU1_NS_RAM_DATA_SIZE              "0xA0000"              CACHE STRING "CPU1 NS RAM data size")
else()
    # No CPU1 image: keep the size at zero.
    set(FLASH_CPU1_NS_PARTITION_SIZE       "0x0")
    set(CPU1_NS_RAM_DATA_SIZE              "0x0")
endif()

#===================== Runtime Crypto partition (ML-DSA-87) ====================#
# PoC: enable ML-DSA-87 in the runtime Crypto partition through the PQCP driver.
# CRYPTO_STACK_SIZE is sized accordingly in config_tfm_target.h.
set(TFM_TF_PSA_CRYPTO_PLATFORM_EXTRA_CONFIG_PATH
    "${CMAKE_CURRENT_LIST_DIR}/crypto_extra_config.h"
    CACHE FILEPATH "Platform extra TF-PSA-Crypto config (ML-DSA-87)")

#===============================================================================#
#===================== Peripherals owned by the SPE ============================#
set(RENESAS_RA_PSAR_NS_SCI8 OFF CACHE BOOL "")
set(RENESAS_RA_IPCSAR_NS_SAIPCSEM0 OFF CACHE BOOL "" FORCE)
set(RENESAS_RA_IPCSAR_NS_SAIPCNMI0 OFF CACHE BOOL "" FORCE)
set(RENESAS_RA_IPCSAR_NS_SAIPCNMI1 OFF CACHE BOOL "" FORCE)

#===============================================================================#
include(${RENESAS_RA_COMMON_SOURCE_DIR}/core/config.cmake)
