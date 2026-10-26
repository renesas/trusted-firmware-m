#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

include(${RENESAS_RA_SOC_SOURCE_DIR}/config.cmake)

# The EK-RA8M2 carries the R7KA8M2JF part (1 MB code MRAM, 1.6 MB on-chip
# SRAM SRAM0-3, dual-core CM85 + CM33), so no per-variant preset is needed here
# (the 512 KB MRAM parts are single-core only); every individual size below
# remains independently overridable on the command line.
#
# The CPU0 NS image can be relocated off-chip (e.g. onto the EK-RA8M2's OSPI0
# CS0) by overriding CPU0_NS_ROM_ALIAS_BASE (e.g. 0x88000000) together with
# CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET (e.g. 0x0).

#===================== Used non-secure clients =================================#
set(RA_CM85_NS_PRESENT ON CACHE BOOL "Serve a local TrustZone NS client on CPU0 (Cortex-M85)")
set(RA_CM33_NS_PRESENT ON CACHE BOOL "Serve a remote mailbox NS client on CPU1 (Cortex-M33)")

#===================== Secure image (SPE, CPU0 Cortex-M85) =====================#
set(FLASH_S_PARTITION_SIZE          "0x50000"             CACHE STRING "Secure image slot size")
set(FLASH_PS_AREA_SIZE              "0x8000"              CACHE STRING "Protected Storage area size")
set(FLASH_ITS_AREA_SIZE             "0x4000"              CACHE STRING "Internal Trusted Storage area size")
set(FLASH_OTP_NV_COUNTERS_AREA_SIZE "0x4000"              CACHE STRING "OTP/NV counters area size")

set(S_DATA_SIZE     "0x80000" CACHE STRING "Secure RAM size")
set(S_RAM_CODE_SIZE "0x1000"  CACHE STRING "Secure RAM size for RAM-resident MRAM programming code")

set(S_ROM_ALIAS_BASE "${FLASH_BASE_ADDRESS}" CACHE STRING "MRAM secure alias base")
set(S_RAM_ALIAS_BASE "${SRAM_BASE_ADDRESS}"  CACHE STRING "SRAM secure alias base")

#===================== NS image, CPU0 Cortex-M85 (TrustZone) ===================#
# CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET and CPU0_NS_RAM_OFFSET are derived in
# common/core/post_config.cmake (overridable on the command line).
set(CPU0_NS_ROM_ALIAS_BASE                 "${RA_FLASH_NS_ALIAS}" CACHE STRING "CPU0 NS ROM alias base")
set(CPU0_NS_RAM_ALIAS_BASE                 "${RA_SRAM_NS_ALIAS}"  CACHE STRING "CPU0 NS RAM alias base")

if(RA_CM85_NS_PRESENT)
    set(FLASH_CPU0_NS_PARTITION_SIZE       "0x60000"              CACHE STRING "CPU0 NS image slot size")
    set(CPU0_NS_RAM_DATA_SIZE              "0x80000"              CACHE STRING "CPU0 NS RAM data size")
else()
    set(FLASH_CPU0_NS_PARTITION_SIZE       "0x0")
    set(CPU0_NS_RAM_DATA_SIZE              "0x0")
endif()

#===================== NS image, CPU1 Cortex-M33 (Mailbox) =====================#
# CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET and CPU1_NS_RAM_OFFSET are derived in
# common/core/post_config.cmake (overridable on the command line).
set(CPU1_NS_ROM_ALIAS_BASE                 "${RA_FLASH_NS_ALIAS}" CACHE STRING "CPU1 NS ROM alias base")
set(CPU1_NS_RAM_ALIAS_BASE                 "${RA_SRAM_NS_ALIAS}"  CACHE STRING "CPU1 NS RAM alias base")

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
