#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

include(${RENESAS_RA_SOC_SOURCE_DIR}/config.cmake)

#===================== Used non-secure clients =================================#
set(RA_CM85_NS_PRESENT ON CACHE BOOL "Serve a local TrustZone NS client on CPU0 (Cortex-M85)")
set(RA_CM33_NS_PRESENT ON CACHE BOOL "Serve a remote mailbox NS client on CPU1 (Cortex-M33)")

#===================== Secure image (SPE, CPU0 Cortex-M85) =====================#
set(FLASH_S_PARTITION_SIZE          "0x50000"             CACHE STRING "Secure image slot size")
set(FLASH_PS_AREA_SIZE              "0x8000"              CACHE STRING "Protected Storage area size")
set(FLASH_ITS_AREA_SIZE             "0x4000"              CACHE STRING "Internal Trusted Storage area size")
set(FLASH_OTP_NV_COUNTERS_AREA_SIZE "0x4000"              CACHE STRING "OTP/NV counters area size")
math(EXPR _S_STORAGE_TOTAL "${FLASH_PS_AREA_SIZE} + ${FLASH_ITS_AREA_SIZE} + ${FLASH_OTP_NV_COUNTERS_AREA_SIZE}")
set(FLASH_STORAGE_TOTAL_SIZE        "${_S_STORAGE_TOTAL}" CACHE INTERNAL "Total storage area size")

set(S_DATA_SIZE     "0x80000" CACHE STRING "Secure RAM size")
set(S_RAM_CODE_SIZE "0x1000"  CACHE STRING "Secure RAM size for RAM-resident MRAM programming code")

set(S_ROM_ALIAS_BASE "${FLASH_BASE_ADDRESS}" CACHE STRING "MRAM secure alias base")
set(S_RAM_ALIAS_BASE "${SRAM_BASE_ADDRESS}"  CACHE STRING "SRAM secure alias base")

math(EXPR _S_CHAIN_END "${FLASH_AREA_BL2_OFFSET} + ${FLASH_AREA_BL2_SIZE} + ${FLASH_S_PARTITION_SIZE} + ${FLASH_STORAGE_TOTAL_SIZE}")

#===================== NS image, CPU0 Cortex-M85 (TrustZone) ===================#
set(CPU0_NS_ROM_ALIAS_BASE                 "${RA_FLASH_NS_ALIAS}" CACHE STRING "CPU0 NS ROM alias base")
set(CPU0_NS_RAM_ALIAS_BASE                 "${RA_SRAM_NS_ALIAS}"  CACHE STRING "CPU0 NS RAM alias base")
set(CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET "${_S_CHAIN_END}"      CACHE STRING "CPU0 NS image offset within its ROM alias")
set(CPU0_NS_RAM_OFFSET                     "${S_DATA_SIZE}"       CACHE STRING "CPU0 NS RAM offset")

if(RA_CM85_NS_PRESENT)
    set(FLASH_CPU0_NS_PARTITION_SIZE       "0x60000"              CACHE STRING "CPU0 NS image slot size")
    set(CPU0_NS_RAM_DATA_SIZE              "0x80000"              CACHE STRING "CPU0 NS RAM data size")
else()
    set(FLASH_CPU0_NS_PARTITION_SIZE       "0x0")
    set(CPU0_NS_RAM_DATA_SIZE              "0x0")
endif()

math(EXPR _CPU1_ROM_OFFSET "${_S_CHAIN_END} + ${FLASH_CPU0_NS_PARTITION_SIZE}")
math(EXPR _CPU1_RAM_OFFSET "${S_DATA_SIZE}  + ${CPU0_NS_RAM_DATA_SIZE}")

#===================== NS image, CPU1 Cortex-M33 (Mailbox) =====================#
set(CPU1_NS_ROM_ALIAS_BASE                 "${RA_FLASH_NS_ALIAS}" CACHE STRING "CPU1 NS ROM alias base")
set(CPU1_NS_RAM_ALIAS_BASE                 "${RA_SRAM_NS_ALIAS}"  CACHE STRING "CPU1 NS RAM alias base")
set(CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET "${_CPU1_ROM_OFFSET}"  CACHE STRING "CPU1 NS image offset within its ROM alias")
set(CPU1_NS_RAM_OFFSET                     "${_CPU1_RAM_OFFSET}"  CACHE STRING "CPU1 NS RAM offset")

if(RA_CM33_NS_PRESENT)
    set(FLASH_CPU1_NS_PARTITION_SIZE       "0x40000"              CACHE STRING "CPU1 NS image slot size")
    set(CPU1_NS_RAM_DATA_SIZE              "0xA0000"              CACHE STRING "CPU1 NS RAM data size")
else()
    # No CPU1 image: keep the size at zero.
    set(FLASH_CPU1_NS_PARTITION_SIZE       "0x0")
    set(CPU1_NS_RAM_DATA_SIZE              "0x0")
endif()

#===============================================================================#
include(${RENESAS_RA_COMMON_SOURCE_DIR}/core/config.cmake)
