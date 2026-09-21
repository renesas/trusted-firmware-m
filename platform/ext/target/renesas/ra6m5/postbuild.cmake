#-------------------------------------------------------------------------------
# Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Post-build script to generate SREC files for RA6M4
# This file is included from the main build system after targets are defined

# Generate SREC files from ELF binaries for easier flashing with Renesas tools

# Generate SREC for BL2 bootloader
if(TARGET bl2 AND BL2)
    add_custom_command(TARGET bl2
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O srec $<TARGET_FILE:bl2> ${CMAKE_BINARY_DIR}/bin/bl2.srec
        COMMAND ${CMAKE_COMMAND} -E echo "[RA6M4] Generated SREC: bin/bl2.srec"
        COMMENT "Generating SREC file for BL2 bootloader"
        VERBATIM
    )
endif()

# Generate SREC for TF-M secure firmware
if(TARGET tfm_s)
    add_custom_command(TARGET tfm_s
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O srec $<TARGET_FILE:tfm_s> ${CMAKE_BINARY_DIR}/bin/tfm_s.srec
        COMMAND ${CMAKE_COMMAND} -E echo "[RA6M4] Generated SREC: bin/tfm_s.srec"
        COMMENT "Generating SREC file for TF-M secure firmware"
        VERBATIM
    )
endif()

# Generate SREC for signed secure firmware (for MCUboot)
# The signed binary is at 0x00020000, so we need to set the base address
if(BL2)
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/bin/tfm_s_signed.srec
        COMMAND ${CMAKE_OBJCOPY} -I binary -O srec
                --change-addresses=0x00020000
                ${CMAKE_BINARY_DIR}/bin/tfm_s_signed.bin
                ${CMAKE_BINARY_DIR}/bin/tfm_s_signed.srec
        COMMAND ${CMAKE_COMMAND} -E echo "[RA6M4] Generated SREC: bin/tfm_s_signed.srec (signed, base=0x00020000)"
        DEPENDS ${CMAKE_BINARY_DIR}/bin/tfm_s_signed.bin
        COMMENT "Generating SREC file for signed TF-M secure firmware"
        VERBATIM
    )

    # Create a target for the signed SREC
    add_custom_target(tfm_s_signed_srec ALL
        DEPENDS ${CMAKE_BINARY_DIR}/bin/tfm_s_signed.srec
    )
endif()

# Optional: Generate combined SREC with BL2 + signed secure image
# This is useful for initial programming of blank devices
if(TARGET bl2 AND BL2)
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/bin/tfm_combined.srec
        COMMAND ${CMAKE_COMMAND} -E echo "S00F000074666D5F636F6D62696E656421" > ${CMAKE_BINARY_DIR}/bin/tfm_combined.srec
        COMMAND ${CMAKE_OBJCOPY} -O srec $<TARGET_FILE:bl2> ${CMAKE_BINARY_DIR}/bin/bl2_temp.srec
        COMMAND cat ${CMAKE_BINARY_DIR}/bin/bl2_temp.srec | grep -v "^S[079]" >> ${CMAKE_BINARY_DIR}/bin/tfm_combined.srec || true
        COMMAND cat ${CMAKE_BINARY_DIR}/bin/tfm_s_signed.srec | grep -v "^S[079]" >> ${CMAKE_BINARY_DIR}/bin/tfm_combined.srec || true
        COMMAND ${CMAKE_COMMAND} -E echo "S9030000FC" >> ${CMAKE_BINARY_DIR}/bin/tfm_combined.srec
        COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/bin/bl2_temp.srec
        COMMAND ${CMAKE_COMMAND} -E echo "[RA6M4] Generated combined SREC: bin/tfm_combined.srec (BL2 + signed image)"
        DEPENDS ${CMAKE_BINARY_DIR}/bin/tfm_s_signed.srec bl2
        COMMENT "Generating combined SREC file (BL2 + signed secure firmware)"
        VERBATIM
    )

    add_custom_target(tfm_combined_srec ALL
        DEPENDS ${CMAKE_BINARY_DIR}/bin/tfm_combined.srec
    )
endif()
