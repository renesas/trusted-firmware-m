#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#-------------------------------------------------------------------------------
#
# RA8M2 platform configuration.
#
# MCUboot settings. TF-M builds its OWN BL2 from its own vendored MCUboot (bl2/ext/mcuboot)
# and signs with its own keys - the e2 ra8m2_mcuboot project contributes only FSP driver
# modules (FSP_MODULES_BL2 = bsp flash) and never its MCUboot. So these values, not the e2
# ones, decide what TF-M's bootloader verifies. The two bootloaders are separate binaries
# with different keys; images signed for one will not boot under the other.
#
# What DOES have to agree is anything the shared flash layout implies: the image count and
# the upgrade mode, because the solution partitioned the slots for them (two images,
# overwrite-only). The solution reserves a 32 KB secure UNUSED_SCRATCH block after BL2 for
# a later move to swap mode; nothing here uses it yet - see flash_layout.h. The
# header/trailer sizes are read straight from the solution below.
#
# ra8m2_mcuboot's own settings, for the standalone bootloader (2026-08-26):
#     signature      ECDSA P-256    - was "None" until 2026-08-26, so the standalone
#                                     bootloader had only ever hash-checked images
#     upgrade_mode   overwrite-only  (hence no scratch area)
#     validate_primary  enabled      - re-verifies the secure image every boot
#     MCUBOOT_IMAGE_NUMBER 2         - dual image
#
# NOTE: RA8M2 has RSIP-E50D, NOT SCE9. bsp_feature.h is explicit -
# BSP_FEATURE_RSIP_RSIP_E50D_SUPPORTED == 1 and BSP_FEATURE_RSIP_SCE9_SUPPORTED == 0 - so the
# RA6M4/RA6M5 comment this was seeded from had it exactly backwards.
#
# The port still uses the engine for ciphers, hashes and ECC through FSP's rm_psa_crypto
# *_ALT sources, and BL2 hashes images there too. The ALT layer is very nearly the same code:
# FSP reaches E50D through the same r_sce driver and 33 of the 35 ALT sources are
# byte-identical to the SCE9 pack. What differs is the accelerator directory
# (renesas/rsip_e50d, DECISIONS D050) and the curve set - E50D adds P-521 and Curve25519 in
# hardware (D051). See the crypto block below.
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
# MUST be 32 - the RA8M2 MRAM programming size
# (BSP_FEATURE_MRAM_PROGRAMMING_SIZE_BYTES), NOT the 128 that RA6M5's code flash needed. It
# must equal the program_unit Driver_Flash.c reports, which flash_map.c:422 returns as
# flash_area_align(). Images are signed --align ${MCUBOOT_ALIGN_VAL} --pad, so signing and
# runtime derive the trailer from the same number; disagreement means BL2 hunts for the magic
# where imgtool did not put it.
#
# MRAM has no erase granularity to reconcile this against: r_mram.c reports block_size 32 and
# its mram_erase_blocks() is documented as simulated. So 32 is a write-size constraint only,
# and FLASH_AREA_IMAGE_SECTOR_SIZE is free to be chosen for the layout instead (see
# flash_layout.h - it is 0x1000 here, set by the slot geometry rather than by the device).
#
# At 32 the OVERWRITE_ONLY trailer is max_align*2 + align_up(16,32) = 0x60, far smaller than
# RA6M5's 0x180 at align 128. The secure slot reserves 0x100 for it (__BL_0_P_T) and the NSC
# is 0x300 at 0x60C00, so the trailer is clear with room to spare - the trailer pressure that
# forced RA6E1's NSC up to 0x800 does not arise at this alignment.
#
# Corresponding partition values: see the raPartitions block in RA8M2_SOLUTION.md. The secure
# image region FLASH_CPU0_S is 0x4EA00 at 0x12200, NSC 0x300 at 0x60C00, NS at 0xB0000.
#
# FLAG DAY: imgtool encodes max_align into the boot magic whenever it is not 8
# (image.py:189-202), so images signed at one alignment are NOT accepted by a BL2 built at
# another. Reflash every slot rather than mixing - and note RA6M5 images are signed at 128,
# so the two parts' images are not interchangeable even setting the addresses aside.
set(MCUBOOT_ALIGN_VAL                   32          CACHE STRING    "RA8M2 MRAM programming size")
set(MCUBOOT_HW_KEY                      OFF         CACHE BOOL      "")
set(MCUBOOT_MEASURED_BOOT               OFF         CACHE BOOL      "Disabled in the solution")
set(MCUBOOT_DATA_SHARING                OFF         CACHE BOOL      "Disabled in the solution")

# SCE9 cipher acceleration: platform/ext/accelerator/renesas/sce9. FSP's rm_psa_crypto
# *_ALT sources (AES, AES-GCM, SHA-256, ECDSA/ECDH P-256, plaintext keys) compiled into
# TF-M's Mbed TLS for the secure crypto partition, and SHA-256 for BL2's image hash. The
# engine here is RSIP-E50D, not SCE9 - FSP reaches it through the same r_sce driver
# (procedures under crypto_procedures/src/rsip_e50d/plainkey/) and 33 of the 35 rm_psa_crypto
# ALT sources are byte-identical to the SCE9 pack, but it is a separate accelerator directory
# for now and consolidates with sce9 at P7 (DECISIONS D050).
#
# CCM stays in software, as on SCE9 - and here that DIVERGES from FSP, whose E50D
# configuration enables MBEDTLS_CCM_ALT. Whether E50D shares the 110 B associated-data cap
# that broke Protected Storage on SCE9 is unmeasured on this engine (DECISIONS D043, D051).
#
# KNOWN: the E50D pack is missing both rm_psa_crypto session-leak fixes that the SCE9 pack
# has, so the PSA Arch crypto suite is expected to cascade until an updated pack lands
# (DECISIONS D052).
#
# Needs the Crypto stack (rm_psa_crypto) in the secure e2 project.
# -DCRYPTO_HW_ACCELERATOR=OFF returns to all-software ciphers. The TRNG is hardware either
# way (sce_trng.c).
set(CRYPTO_HW_ACCELERATOR               ON          CACHE BOOL      "RSIP-E50D ciphers via FSP *_ALT")
set(CRYPTO_HW_ACCELERATOR_TYPE          "renesas/rsip_e50d" CACHE STRING "platform/ext/accelerator/<type>")

# Crypto library: FSP's Mbed TLS, not upstream.
#
# FSP's *_ALT sources are written against FSP's PSA core, which differs from upstream in ways
# they depend on - GCM finish reporting its ciphertext length, GCM verify routed to
# sce_gcm_verify with the expected tag - and FSP's cipher_alt.c supplies the block chunking
# and session close that its aes_alt.c assumes. cmake/fsp_mbedtls.cmake builds an overlay of
# FSP's include/library on upstream scaffolding of the same version and points MBEDCRYPTO_PATH
# at it. OFF falls back to TF-M's downloaded Mbed TLS, with multi-part GCM unsupported.
set(RA8M2_FSP_MBEDTLS                   ON          CACHE BOOL      "Crypto from FSP's Mbed TLS")
if(RA8M2_FSP_MBEDTLS AND CRYPTO_HW_ACCELERATOR)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/fsp_mbedtls.cmake)
endif()
set(TFM_CRYPTO_TEST_ALG_CFB             OFF         CACHE BOOL      "")

# Entropy from the RSIP-E50D TRNG via PSA's external-RNG hook (sce_trng.c), NOT from a stored
# NV seed. The NV-seed path seeds from PLAT_OTP_ID_ENTROPY_SEED, whose default provisioning
# value is a hard-coded constant shared by every device - unusable for attestation or PS.
# CRYPTO_EXT_RNG / CRYPTO_NV_SEED are set in config_tfm_target.h; these are the CMake half.
set(PLATFORM_DEFAULT_NV_SEED            OFF         CACHE BOOL      "RSIP-E50D TRNG instead")
set(TFM_MBEDCRYPTO_PLATFORM_EXTRA_CONFIG_PATH
    ${CMAKE_CURRENT_LIST_DIR}/mbedtls_extra_config.h CACHE PATH
    "Appended to TF-M's mbedcrypto config; enables MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG")

# Fault diagnosis. Both default OFF upstream; both are on here because a fault on this
# port is otherwise undiagnosable - every panic and every fault funnels into the same
# tfm_hal_system_halt() spin with no detail, which is what made a HardFault look like a
# deliberate SPM panic during bring-up and cost a long detour into the ITS filesystem.
#
# Set HERE rather than left in a build directory's cache: a cache value survives rebuilds
# but not a clean reconfigure, so the next person to configure would silently get a
# quieter build than this one and no way to know why.
#
# TFM_EXCEPTION_INFO_DUMP costs ~3.4 KB of secure text; TFM_SPM_DEBUG_TRACE is a handful of
# log calls. Turn both off for a production or size-constrained build - they are debug
# aids, and TFM_SPM_DEBUG_TRACE in particular reports partition ids and status codes on
# the console.
set(TFM_EXCEPTION_INFO_DUMP             ON          CACHE BOOL      "Capture and print exception info on a fatal error")
set(TFM_SPM_DEBUG_TRACE                 ON          CACHE BOOL      "Log the tfm_core_panic() caller and each partition init")

set(TFM_ISOLATION_LEVEL                 1           CACHE STRING    "Isolation level")
set(CONFIG_TFM_SPM_BACKEND              "SFN"       CACHE STRING    "SFN - no IPC overhead")

# Levels 2 and 3 are reachable with -DTFM_ISOLATION_LEVEL=, which overrides the default
# above. Level 3 additionally needs this declaration, because config/check_config.cmake
# rejects TFM_ISOLATION_LEVEL 3 unless the platform claims support - there is no such gate
# on level 2.
#
# Nothing platform-specific implements it: this port uses the shared
# platform/ext/common/tfm_hal_isolation_v8m.c, which carries the whole level-3 mechanism
# (per-partition MPU regions programmed on each context switch), and the RA8M2 Cortex-M33
# has the 8-region ARMv8-M MPU it assumes. The flag is a statement that the port has been
# built and run that way, so keep it honest.
#
# The isolation level does NOT change the TrustZone boundaries programmed with the Renesas
# Device Partition Manager. Those come from the solution's IDAU partitioning - the
# BSP_PARTITION_* values in the generated bsp_linker_info.h - and separate secure from
# non-secure. Levels 2 and 3 subdivide the SECURE side with the MPU, which is invisible to
# the IDAU/SAU. RA8M2_SOLUTION.md carries the RDPM values; they are unaffected by this.
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

# Console. RTT avoids UART wiring and S/NS peripheral contention; each image gets its
# own control block. OFF routes stdout to the FSP SCI UART instead.
set(RA8M2_STDOUT_RTT                    ON          CACHE BOOL      "stdout over SEGGER RTT")

# RTT drops a whole write when the up-buffer is full (SEGGER_RTT_MODE_NO_BLOCK_SKIP), which
# is why a fast talker - the PSA Arch suites - loses whole lines and tests from the log while
# the run itself is fine. ON makes the target wait for the host to drain instead, so the
# transcript is complete.
#
# Test builds only: with no RTT viewer attached, nothing drains the buffer and the first
# write past 4 KB blocks forever. Default OFF for that reason.
set(RA8M2_RTT_BLOCKING          OFF         CACHE BOOL      "RTT blocks rather than dropping output")
if(RA8M2_STDOUT_RTT)
    set(PLATFORM_DEFAULT_UART_STDOUT    OFF         CACHE BOOL      "RTT backend instead")
else()
    set(PLATFORM_DEFAULT_UART_STDOUT    ON          CACHE BOOL      "")
endif()

# Debug aid for first flash on a new board: spin at the top of BL2 main() so FAWMON /
# FSPR can be read back before MCUboot runs. See DESIGN.md 8.4.
set(RA8M2_BL2_HALT_AT_MAIN              OFF         CACHE BOOL      "BL2 spins at main()")

set(PS_ENCRYPTION                       ON          CACHE BOOL      "AES-GCM")
set(PS_RAM_FS                           OFF         CACHE BOOL      "")
set(ITS_RAM_FS                          OFF         CACHE BOOL      "")

set(CONFIG_TFM_USE_TRUSTZONE            ON          CACHE BOOL      "")
set(TFM_MULTI_CORE_TOPOLOGY             OFF         CACHE BOOL      "")
set(CONFIG_TFM_ENABLE_CP10CP11          OFF         CACHE BOOL      "Soft float, matches TF-M")
