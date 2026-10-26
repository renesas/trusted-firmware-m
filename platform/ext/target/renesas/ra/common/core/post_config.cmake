#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Flash/RAM map derived from the slot and area sizes, included by each board's
# post_config.cmake.
#
# TF-M includes the board post_config.cmake after the board config.cmake and
# after the test configuration (tests/tfm_tests_config.cmake), so the values
# below follow the final sizes. They are normal variables, recomputed on every
# configure; an offset given on the command line (-D) is kept as is, e.g. to
# relocate the CPU0 NS image off-chip.

math(EXPR FLASH_STORAGE_TOTAL_SIZE "${FLASH_PS_AREA_SIZE} + ${FLASH_ITS_AREA_SIZE} + ${FLASH_OTP_NV_COUNTERS_AREA_SIZE}")

# The images are packed in MRAM in this order: [BL2] S image, storage areas,
# CPU0 NS image, CPU1 NS image. RAM: S data, CPU0 NS data, CPU1 NS data.
math(EXPR _RA_S_CHAIN_END     "${FLASH_AREA_BL2_OFFSET} + ${FLASH_AREA_BL2_SIZE} + ${FLASH_S_PARTITION_SIZE} + ${FLASH_STORAGE_TOTAL_SIZE}")
math(EXPR _RA_CPU1_ROM_OFFSET "${_RA_S_CHAIN_END} + ${FLASH_CPU0_NS_PARTITION_SIZE}")
math(EXPR _RA_CPU1_RAM_OFFSET "${S_DATA_SIZE} + ${CPU0_NS_RAM_DATA_SIZE}")

#===================== NS image, CPU0 Cortex-M85 (TrustZone) ===================#
if(NOT DEFINED CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET)
    set(CPU0_NS_IMAGE_PRIMARY_PARTITION_OFFSET "${_RA_S_CHAIN_END}")
endif()
if(NOT DEFINED CPU0_NS_RAM_OFFSET)
    set(CPU0_NS_RAM_OFFSET "${S_DATA_SIZE}")
endif()

#===================== NS image, CPU1 Cortex-M33 (Mailbox) =====================#
if(NOT DEFINED CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET)
    set(CPU1_NS_IMAGE_PRIMARY_PARTITION_OFFSET "${_RA_CPU1_ROM_OFFSET}")
endif()
if(NOT DEFINED CPU1_NS_RAM_OFFSET)
    set(CPU1_NS_RAM_OFFSET "${_RA_CPU1_RAM_OFFSET}")
endif()
