#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#-------------------------------------------------------------------------------
#
# RA8M2 platform configuration.
#
# TF-M builds its own BL2 from its vendored MCUboot (bl2/ext/mcuboot) and signs with its own
# keys. The e2 ra8m2_mcuboot project contributes only FSP driver modules (FSP_MODULES_BL2 =
# bsp flash), never its MCUboot, so these values decide what TF-M's BL2 verifies. The two
# bootloaders use different keys: images signed for one do not boot under the other.
#
# The values the shared flash layout implies must agree with the solution: the image count
# and the upgrade mode (two images, overwrite-only). The solution reserves a 32 KB secure
# UNUSED_SCRATCH block after BL2 for a later move to swap mode; nothing uses it yet (see
# flash_layout.h). The header and trailer sizes are read from the solution below.
#
# ra8m2_mcuboot's own settings, for the standalone bootloader:
#     signature         ECDSA P-256
#     upgrade_mode      overwrite-only (no scratch area)
#     validate_primary  enabled - re-verifies the secure image every boot
#     MCUBOOT_IMAGE_NUMBER 2 - dual image
#
# RA8M2 has RSIP-E50D, not SCE9 (bsp_feature.h: BSP_FEATURE_RSIP_RSIP_E50D_SUPPORTED == 1,
# BSP_FEATURE_RSIP_SCE9_SUPPORTED == 0). Ciphers, hashes and ECC run on it through FSP's
# rm_psa_crypto *_ALT sources, and BL2 hashes images there too. E50D adds P-521 and
# Curve25519 in hardware (D051). See the crypto block below.
#-------------------------------------------------------------------------------

set(BL2                                 ON          CACHE BOOL      "Build BL2")

#-------------------------------------------------------------------------------
# MCUboot header and trailer sizes come from the solution, not from TF-M's defaults.
#
# imgtool is invoked with these (-H and the trailer reservation). TF-M defaults both to
# 0x400; this solution uses a 0x200 header and a 0x100 trailer. At the default, imgtool starts
# the payload 0x400 into the slot while the image is linked for slot+0x200, so the signed
# image is offset by 0x200 and does not boot. region_defs.h derives the correct values for
# the C side, so the build reports only a "BL2_HEADER_SIZE redefined" warning.
#
# Read from the generated file the layout comes from, so a repartition carries through. These
# are cache values: a change needs a clean CMake cache.
#-------------------------------------------------------------------------------
set(_ra8m2_linker_info "${FSP_S_APP_DIR}/Debug/bsp_linker_info.h")
if(EXISTS "${_ra8m2_linker_info}")
    foreach(_pair "BL2_HEADER_SIZE;___BL_0_P_H_SIZE" "BL2_TRAILER_SIZE;___BL_0_P_T_SIZE")
        list(GET _pair 0 _tfm_var)
        list(GET _pair 1 _bsp_sym)
        file(STRINGS "${_ra8m2_linker_info}" _hit
             REGEX "^[ \t]*#define[ \t]+BSP_PARTITION${_bsp_sym}[ \t]")
        if(_hit)
            string(REGEX REPLACE ".*\\(([^)]+)\\).*" "\\1" _val "${_hit}")
            set(${_tfm_var} "${_val}" CACHE STRING "From the e2 solution partitioning")
        endif()
    endforeach()
else()
    message(WARNING
        "RA8M2: ${_ra8m2_linker_info} not found while reading the MCUboot header/trailer "
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
# 32: the RA8M2 MRAM programming size (BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES), not the 128
# of RA6M5's code flash. It must equal the program_unit Driver_Flash.c reports, which
# flash_map.c:422 returns as flash_area_align(). Images are signed --align
# ${MCUBOOT_ALIGN_VAL} --pad, so signing and runtime derive the trailer from the same number;
# if they disagree, BL2 looks for the magic where imgtool did not put it.
#
# MRAM has no erase granularity to reconcile this against: r_mram.c reports block_size 32 and
# documents its mram_erase_blocks() as simulated. So 32 is a write-size constraint only, and
# FLASH_AREA_IMAGE_SECTOR_SIZE is chosen for the layout instead: 0x8000, matching FSP's
# RM_MCUBOOT_MRAM_BLOCK_SIZE (flash_layout.h).
#
# At 32 the OVERWRITE_ONLY trailer is max_align*2 + align_up(16,32) = 0x60, against RA6M5's
# 0x180 at align 128. The __BL_*_T partitions are zero-size end markers here, so the trailer
# has no region of its own: it occupies the last 0x60 bytes of the slot, which on the secure
# primary is the top of the NSC window (FLASH_CPU0_C, 0x400 at offset 0xAFC00). The veneers
# therefore have 0x3A0 of that 0x400. ra8m2_layout_checks.c asserts that the NSC ends exactly
# at the secure/non-secure boundary, which keeps this arithmetic valid.
#
# Partition values are in RA8M2_SOLUTION.md. Secure code FLASH_CPU0_S is 0x47A00 at offset
# 0x68200, NSC 0x400 at 0xAFC00, non-secure from 0xB0000.
#
# WARNING: imgtool encodes max_align into the boot magic whenever it is not 8
# (image.py:189-202), so a BL2 built at one alignment rejects images signed at another.
# Reflash every slot rather than mixing. RA6M5 images are signed at 128, so the two parts'
# images are not interchangeable.
set(MCUBOOT_ALIGN_VAL                   32          CACHE STRING    "RA8M2 MRAM programming size")
set(MCUBOOT_HW_KEY                      OFF         CACHE BOOL      "")
# MCUBOOT_DATA_SHARING does the work: without it BL2 writes no boot record and
# MCUBOOT_MEASURED_BOOT has nothing to carry. With no boot record,
# attest_add_all_sw_components() (attest_core.c:120) sees component_cnt == 0, and the
# default ATTEST_TOKEN_PROFILE_PSA_IOT_1 (config_base.h:165) lets it emit
# IAT_NO_SW_COMPONENTS and succeed. Any other profile fails with
# PSA_ATTEST_ERR_CLAIM_UNAVAILABLE. See CONFIGURATION.md §3 and DECISIONS D080.
#
# The platform side agrees with the e2 solution:
#   BOOT_TFM_SHARED_DATA_BASE  0x22000000 (S_DATA_START)   region_defs.h
#   BOOT_TFM_SHARED_DATA_SIZE  0x400
#   SHARED_BOOT_MEASUREMENT_*  aliased onto the same region
#   FSP's MCUBOOT_SHARED_DATA_BASE/SIZE  0x22000000 / 0x380, inside it
#
# WARNING: the PSA Arch attestation suite passes with or without a boot record. Verify by
# reading the token for IAT_SW_COMPONENTS rather than IAT_NO_SW_COMPONENTS.
set(MCUBOOT_MEASURED_BOOT               ON          CACHE BOOL      "Boot record of each image")
set(MCUBOOT_DATA_SHARING                ON          CACHE BOOL      "BL2 -> secure boot record")

# RSIP-E50D acceleration (platform/ext/accelerator/renesas/rsip_e50d): FSP's rm_psa_crypto
# *_ALT sources (AES, AES-GCM, SHA-256, ECDSA/ECDH P-256, plaintext keys) are compiled into
# TF-M's Mbed TLS for the secure crypto partition, and SHA-256 serves BL2's image hash. FSP
# reaches E50D through the same r_sce driver (procedures under
# crypto_procedures/src/rsip_e50d/plainkey/), and 33 of the 35 rm_psa_crypto ALT sources are
# byte-identical to the SCE9 pack. The accelerator directory stays separate from sce9 until
# it consolidates at P7 (DECISIONS D050).
#
# CCM stays in software, as on SCE9. This diverges from FSP, whose E50D configuration enables
# MBEDTLS_CCM_ALT. Whether E50D shares the 110 B associated-data cap that broke Protected
# Storage on SCE9 is unmeasured on this engine (DECISIONS D043, D051).
#
# The 6.7.0-beta0+20260924.4dfe9b7f pack carries both rm_psa_crypto session-leak fixes
# (DECISIONS D053, which supersedes D052).
#
# Needs the Crypto stack (rm_psa_crypto) in the secure e2 project. -DCRYPTO_HW_ACCELERATOR=OFF
# selects all-software ciphers; the TRNG is hardware either way (sce_trng.c).
set(CRYPTO_HW_ACCELERATOR               ON          CACHE BOOL      "RSIP-E50D ciphers via FSP *_ALT")
set(CRYPTO_HW_ACCELERATOR_TYPE          "renesas/rsip_e50d" CACHE STRING "platform/ext/accelerator/<type>")

# Crypto library: FSP's Mbed TLS, not upstream, because FSP's *_ALT sources depend on FSP's
# PSA core. cmake/fsp_mbedtls.cmake gives the reasons and builds the overlay that
# MBEDCRYPTO_PATH points at. OFF falls back to TF-M's downloaded Mbed TLS, without multi-part
# GCM.
set(RA8M2_FSP_MBEDTLS                   ON          CACHE BOOL      "Crypto from FSP's Mbed TLS")
if(RA8M2_FSP_MBEDTLS AND CRYPTO_HW_ACCELERATOR)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/fsp_mbedtls.cmake)
endif()
set(TFM_CRYPTO_TEST_ALG_CFB             OFF         CACHE BOOL      "")

# Entropy comes from the RSIP-E50D TRNG through PSA's external-RNG hook (sce_trng.c), not from a
# stored NV seed. The NV-seed path seeds from PLAT_OTP_ID_ENTROPY_SEED, whose default
# provisioning value is a hard-coded constant shared by every device, so it is unusable for
# attestation or PS. config_tfm_target.h sets CRYPTO_EXT_RNG / CRYPTO_NV_SEED; these are the
# CMake half.
set(PLATFORM_DEFAULT_NV_SEED            OFF         CACHE BOOL      "RSIP-E50D TRNG instead")
set(TFM_MBEDCRYPTO_PLATFORM_EXTRA_CONFIG_PATH
    ${CMAKE_CURRENT_LIST_DIR}/mbedtls_extra_config.h CACHE PATH
    "Appended to TF-M's mbedcrypto config; enables MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG")

# Fault diagnosis. Both default OFF upstream. Without them every panic and every fault ends in
# the same tfm_hal_system_halt() spin with no detail. They are set here rather than in a build
# cache so that a clean reconfigure keeps them.
#
# TFM_EXCEPTION_INFO_DUMP costs ~3.4 KB of secure text; TFM_SPM_DEBUG_TRACE is a handful of log
# calls that print partition ids and status codes on the console. Turn both off for a
# production or size-constrained build (CONFIGURATION.md §6, §9).
set(TFM_EXCEPTION_INFO_DUMP             ON          CACHE BOOL      "Capture and print exception info on a fatal error")
set(TFM_SPM_DEBUG_TRACE                 ON          CACHE BOOL      "Log the tfm_core_panic() caller and each partition init")

set(TFM_ISOLATION_LEVEL                 1           CACHE STRING    "Isolation level")
set(CONFIG_TFM_SPM_BACKEND              "SFN"       CACHE STRING    "SFN - no IPC overhead")

# Levels 2 and 3 are selected with -DTFM_ISOLATION_LEVEL=, which overrides the default above.
# config/check_config.cmake rejects level 3 unless the platform declares support; level 2 has
# no such gate.
#
# Nothing platform-specific implements level 3: the shared
# platform/ext/common/tfm_hal_isolation_v8m.c carries the mechanism (per-partition MPU regions
# programmed on each context switch), and the RA8M2 CPU0, a Cortex-M85, has the 8-region
# ARMv8-M MPU it assumes. MPU->TYPE reads 8 on silicon, with 5 static regions at both levels 2
# and 3 and a sixth appended at level 2 when the FLIH binds AGT0. TFM_PXN_ENABLE is OFF but
# available here (D134). The flag states that the port has been built and run that way, so
# keep it true.
#
# The isolation level does not change the TrustZone boundaries. Those come from the
# solution's IDAU partitioning (the BSP_PARTITION_* values in the generated
# bsp_linker_info.h), are programmed with the Renesas Device Partition Manager, and separate
# secure from non-secure. Levels 2 and 3 subdivide the secure side with the MPU, which the
# IDAU/SAU does not see. The RDPM values in RA8M2_SOLUTION.md are unaffected.
set(PLATFORM_HAS_ISOLATION_L3_SUPPORT   ON          CACHE BOOL      "Shared v8m isolation HAL implements L3")

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

# Console. RTT needs no UART wiring and avoids S/NS peripheral contention; each image has its
# own control block. OFF routes stdout to the FSP SCI UART, which does not build as the
# projects ship (CONFIGURATION.md §9).
set(RA8M2_STDOUT_RTT                    ON          CACHE BOOL      "stdout over SEGGER RTT")

# In the default mode (SEGGER_RTT_MODE_NO_BLOCK_SKIP) RTT drops a whole write when the
# up-buffer is full, so fast output such as the PSA Arch suites loses lines from the log while
# the run itself is fine. ON makes the target wait for the host to drain the buffer.
#
# Test builds only: with no RTT viewer attached, nothing drains the buffer and the first write
# past 4 KB blocks forever. Default OFF for that reason.
set(RA8M2_RTT_BLOCKING          OFF         CACHE BOOL      "RTT blocks rather than dropping output")
if(RA8M2_STDOUT_RTT)
    set(PLATFORM_DEFAULT_UART_STDOUT    OFF         CACHE BOOL      "RTT backend instead")
else()
    set(PLATFORM_DEFAULT_UART_STDOUT    ON          CACHE BOOL      "")
endif()

# Debug aid for first flash on a new board: BL2 spins at the top of main() so FAWMON and FSPR
# can be read back before MCUboot runs. See DESIGN.md 8.4.
set(RA8M2_BL2_HALT_AT_MAIN              OFF         CACHE BOOL      "BL2 spins at main()")

set(PS_ENCRYPTION                       ON          CACHE BOOL      "AES-GCM")
set(PS_RAM_FS                           OFF         CACHE BOOL      "")
set(ITS_RAM_FS                          OFF         CACHE BOOL      "")

set(CONFIG_TFM_USE_TRUSTZONE            ON          CACHE BOOL      "")
set(TFM_MULTI_CORE_TOPOLOGY             OFF         CACHE BOOL      "")
set(CONFIG_TFM_ENABLE_CP10CP11          OFF         CACHE BOOL      "Soft float, matches TF-M")

# ------------------------------------------------------------------------------------------
# Supported toolchains: GNUARM and IAR only (D136 R116). Nothing else in the build rejects
# ARMClang or Clang, which would otherwise fail later, in the FSP module scaffolding or at
# link, with an unrelated message.
# ------------------------------------------------------------------------------------------
if(DEFINED TFM_TOOLCHAIN_FILE AND NOT TFM_TOOLCHAIN_FILE MATCHES "(GNUARM|IAR)")
    get_filename_component(_ra_tc "${TFM_TOOLCHAIN_FILE}" NAME)
    message(FATAL_ERROR
        "RA8M2 supports GNUARM and IAR only; TFM_TOOLCHAIN_FILE is '${_ra_tc}'. "
        "Use toolchain_GNUARM.cmake or toolchain_IAR.cmake.")
endif()
