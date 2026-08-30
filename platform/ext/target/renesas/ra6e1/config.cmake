#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#-------------------------------------------------------------------------------
#
# RA6E1 platform configuration.
#
# MCUboot settings. TF-M builds its OWN BL2 from its own vendored MCUboot (bl2/ext/mcuboot)
# and signs with its own keys - the e2 ra6e1_mcuboot project contributes only FSP driver
# modules (FSP_MODULES_BL2 = bsp flash) and never its MCUboot. So these values, not the e2
# ones, decide what TF-M's bootloader verifies. The two bootloaders are separate binaries
# with different keys; images signed for one will not boot under the other.
#
# What DOES have to agree is anything the shared flash layout implies: the image count and
# the upgrade mode, because the solution partitioned the slots for them (two images, no
# scratch region). The header/trailer sizes are read straight from the solution below.
#
# ra6e1_mcuboot's own settings, for the standalone bootloader (2026-08-26):
#     signature      ECDSA P-256    - was "None" until 2026-08-26, so the standalone
#                                     bootloader had only ever hash-checked images
#     upgrade_mode   overwrite-only  (hence no scratch area)
#     validate_primary  enabled      - re-verifies the secure image every boot
#     MCUBOOT_IMAGE_NUMBER 2         - dual image
#
# NOTE: RA6E1 DOES have SCE9 (bsp_feature.h: BSP_FEATURE_RSIP_SCE9_SUPPORTED == 1), same
# as RA6M4. Ciphers still run in software here - only the TRNG is taken from hardware, see
# the crypto block below - so hardware acceleration remains a deliberate later project.
#-------------------------------------------------------------------------------

set(BL2                                 ON          CACHE BOOL      "Build BL2")

#-------------------------------------------------------------------------------
# MCUboot header/trailer size - MUST come from the solution, not TF-M's defaults.
#
# These are what imgtool is invoked with (-H, and the trailer reservation). TF-M defaults
# both to 0x400; this solution uses a 0x200 header and 0x100 trailer. Left at the default,
# imgtool pads the payload to start 0x400 into the slot while the image is LINKED for
# slot+0x200 - the signed image is then offset by 0x200 and does not boot. region_defs.h
# derives the right values for the C side, which is why this only shows up as a
# "BL2_HEADER_SIZE redefined" warning and a silently broken image.
#
# Read from the same generated file the layout comes from, so a repartition carries
# through. Cache values, so this needs a clean CMake cache to take effect.
#-------------------------------------------------------------------------------
set(_ra6e1_linker_info "${FSP_S_APP_DIR}/Debug/bsp_linker_info.h")
if(EXISTS "${_ra6e1_linker_info}")
    foreach(_pair "BL2_HEADER_SIZE;___BL_0_P_H_SIZE" "BL2_TRAILER_SIZE;___BL_0_P_T_SIZE")
        list(GET _pair 0 _tfm_var)
        list(GET _pair 1 _bsp_sym)
        file(STRINGS "${_ra6e1_linker_info}" _hit
             REGEX "^[ \t]*#define[ \t]+BSP_PARTITION${_bsp_sym}[ \t]")
        if(_hit)
            string(REGEX REPLACE ".*\\(([^)]+)\\).*" "\\1" _val "${_hit}")
            set(${_tfm_var} "${_val}" CACHE STRING "From the e2 solution partitioning")
        endif()
    endforeach()
else()
    message(WARNING
        "RA6E1: ${_ra6e1_linker_info} not found while reading the MCUboot header/trailer "
        "sizes. Falling back to TF-M's 0x400 defaults, which do NOT match this solution - "
        "the signed images will be offset in their slots. Build the secure project in e2 "
        "first, then configure with a clean CMake cache.")
endif()
set(MCUBOOT_IMAGE_NUMBER                2           CACHE STRING    "Dual image: S + NS")
set(MCUBOOT_SIGNATURE_TYPE              "EC-P256"   CACHE STRING    "Match the solution")
# TF-M requires PSA crypto for EC signatures (config/check_config.cmake:33). This also
# matches the solution, which stacks psa_crypto under rm_mcuboot_port.
set(MCUBOOT_USE_PSA_CRYPTO              ON          CACHE BOOL      "Required for EC-P256")
set(MCUBOOT_UPGRADE_STRATEGY            "OVERWRITE_ONLY" CACHE STRING "Match the solution")
# SHOULD be 128 - the RA6E1 code flash minimum write (BSP_FEATURE_FLASH_HP_CF_WRITE_SIZE),
# and it must equal the code-flash program_unit in Driver_Flash.c, which flash_map.c:422
# returns as flash_area_align(). Images are signed --align ${MCUBOOT_ALIGN_VAL} --pad, so
# signing and runtime derive the trailer from the same number; disagreement means BL2 hunts
# for the magic where imgtool did not put it.
#
# At 128 the OVERWRITE_ONLY trailer is max_align*2 + align_up(16,128) = 0x180, so the signed
# image must end by 0x98000-0x180 = 0x97E80. That did NOT fit until the solution was
# repartitioned on 2026-08-29: the NSC was 0x400 at 0x97C00, the veneers were pinned at its
# start, and the signed image ended at 0x97E91 - 17 bytes past where the trailer begins.
# imgtool failed the build loudly ("Image size (0x3fe91) + trailer (0x180) exceeds requested
# size 0x40000") rather than producing a silently unbootable image.
#
# The NSC is now 0x800 at 0x97800, so the veneers sit 0x800 from the slot end and the trailer
# fits with room to spare. This value therefore REQUIRES that layout - it is not independent
# of the solution. If the NSC ever shrinks back to 0x400, or the secure image grows into the
# space the trailer needs, imgtool fails the build again. That is the intended behaviour.
#
# Corresponding Partition Manager values: Code Secure 606 KB, NSC 2 KB, NS at 0x98000
# (608 KB). See RA6E1_SOLUTION.md.
#
# FLAG DAY: imgtool encodes max_align into the boot magic whenever it is not 8
# (image.py:189-202), so images signed at align 1 are NOT accepted by a BL2 built at 128.
# Reflash every slot rather than mixing.
set(MCUBOOT_ALIGN_VAL                   128         CACHE STRING    "RA6E1 code flash write unit; needs the 2 KB NSC layout")
set(MCUBOOT_HW_KEY                      OFF         CACHE BOOL      "")
set(MCUBOOT_MEASURED_BOOT               OFF         CACHE BOOL      "Disabled in the solution")
set(MCUBOOT_DATA_SHARING                OFF         CACHE BOOL      "Disabled in the solution")

# Ciphers in software (TF-M's own mbedcrypto). CRYPTO_HW_ACCELERATOR stays OFF: turning it
# on means FSP's mbedTLS + rm_psa_crypto *_ALT stack, which is a separate project.
set(CRYPTO_HW_ACCELERATOR               OFF         CACHE BOOL      "SCE ciphers not wired yet")
set(TFM_CRYPTO_TEST_ALG_CFB             OFF         CACHE BOOL      "")

# Entropy from the SCE9 TRNG via PSA's external-RNG hook (sce_trng.c), NOT from a stored
# NV seed. The NV-seed path seeds from PLAT_OTP_ID_ENTROPY_SEED, whose default provisioning
# value is a hard-coded constant shared by every device - unusable for attestation or PS.
# CRYPTO_EXT_RNG / CRYPTO_NV_SEED are set in config_tfm_target.h; these are the CMake half.
set(PLATFORM_DEFAULT_NV_SEED            OFF         CACHE BOOL      "SCE9 TRNG instead")
set(TFM_MBEDCRYPTO_PLATFORM_EXTRA_CONFIG_PATH
    ${CMAKE_CURRENT_LIST_DIR}/mbedtls_extra_config.h CACHE PATH
    "Appended to TF-M's mbedcrypto config; enables MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG")

set(TFM_ISOLATION_LEVEL                 1           CACHE STRING    "Isolation level")
set(CONFIG_TFM_SPM_BACKEND              "SFN"       CACHE STRING    "SFN - no IPC overhead")

# Partitions
set(TFM_PARTITION_CRYPTO                ON          CACHE BOOL      "")
set(TFM_PARTITION_INTERNAL_TRUSTED_STORAGE ON       CACHE BOOL      "")
set(TFM_PARTITION_PROTECTED_STORAGE     ON          CACHE BOOL      "")
set(TFM_PARTITION_INITIAL_ATTESTATION   ON          CACHE BOOL      "")
set(TFM_PARTITION_PLATFORM              ON          CACHE BOOL      "")
set(TFM_PARTITION_FIRMWARE_UPDATE       OFF         CACHE BOOL      "")

# Platform HALs this port replaces.
set(PLATFORM_DEFAULT_ATTEST_HAL         OFF         CACHE BOOL      "Port supplies attest_hal.c")
set(PLATFORM_DEFAULT_NV_COUNTERS        ON          CACHE BOOL      "")
set(PLATFORM_DEFAULT_CRYPTO_KEYS        ON          CACHE BOOL      "")
set(PLATFORM_DEFAULT_OTP                ON          CACHE BOOL      "")
set(PLATFORM_DEFAULT_PROVISIONING       ON          CACHE BOOL      "")
set(PLATFORM_DEFAULT_SYSTEM_RESET_HALT  OFF         CACHE BOOL      "Port supplies it")

# Console. RTT avoids UART wiring and S/NS peripheral contention; each image gets its
# own control block. OFF routes stdout to the FSP SCI UART instead.
set(RA6E1_STDOUT_RTT                    ON          CACHE BOOL      "stdout over SEGGER RTT")
if(RA6E1_STDOUT_RTT)
    set(PLATFORM_DEFAULT_UART_STDOUT    OFF         CACHE BOOL      "RTT backend instead")
else()
    set(PLATFORM_DEFAULT_UART_STDOUT    ON          CACHE BOOL      "")
endif()

# Debug aid for first flash on a new board: spin at the top of BL2 main() so FAWMON /
# FSPR can be read back before MCUboot runs. See DESIGN.md 8.4.
set(RA6E1_BL2_HALT_AT_MAIN              OFF         CACHE BOOL      "BL2 spins at main()")

set(PS_ENCRYPTION                       ON          CACHE BOOL      "AES-GCM")
set(PS_RAM_FS                           OFF         CACHE BOOL      "")
set(ITS_RAM_FS                          OFF         CACHE BOOL      "")

set(CONFIG_TFM_USE_TRUSTZONE            ON          CACHE BOOL      "")
set(TFM_MULTI_CORE_TOPOLOGY             OFF         CACHE BOOL      "")
set(CONFIG_TFM_ENABLE_CP10CP11          OFF         CACHE BOOL      "Soft float, matches TF-M")
