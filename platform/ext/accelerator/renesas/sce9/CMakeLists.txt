#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Renesas SCE9 crypto accelerator - CRYPTO_HW_ACCELERATOR_TYPE=renesas/sce9.
#
# Per-engine on purpose: RA8's RSIP is a different driver API and lands beside this as
# renesas/rsip, not as a variant of it. SCE9 parts: RA6M4, RA6M5, RA6E1, RA4M2/3 ...
#
# Model: the legacy *_ALT interface (TF-M's LEGACY_DRIVER_API_ENABLED, as CC312 uses it).
# FSP's rm_psa_crypto ships SCE9 implementations of Mbed TLS modules as forks of the
# library sources; the core set is compiled here, INTO TF-M's own Mbed TLS, from the
# secure e2 project. Mbed TLS 3.6-only - see mbedtls_accelerator_config.h.
#
# The secure crypto partition gets the full set; BL2 gets SHA-256 (its image hash).
#-------------------------------------------------------------------------------

if(NOT FSP_S_APP_DIR)
    message(FATAL_ERROR
        "renesas/sce9: FSP_S_APP_DIR is not set. The SCE9 ALT sources are taken from the "
        "secure e2 project's rm_psa_crypto module.")
endif()

set(SCE9_RM_PSA_CRYPTO_DIR "${FSP_S_APP_DIR}/ra/fsp/src/rm_psa_crypto")
if(NOT EXISTS "${SCE9_RM_PSA_CRYPTO_DIR}")
    message(FATAL_ERROR
        "renesas/sce9: ${SCE9_RM_PSA_CRYPTO_DIR} not found.\n"
        "The accelerator compiles FSP's SCE9 *_ALT sources from the secure project. Add the "
        "Crypto stack (rm_psa_crypto) to it in e2 studio, regenerate, and build once - or "
        "configure with -DCRYPTO_HW_ACCELERATOR=OFF to keep ciphers in software.")
endif()

############################ BL2 - SHA-256 #####################################

if(BL2)
    # SHA-256 only - see bl2_mbedtls_accelerator_config.h for why the P-256 verify stays on
    # p256-m. The SCE is brought up by the platform's boot_platform_post_init(), which MCUboot
    # calls before the first image is hashed; boot_hal_bl2.c's crypto_hw_accelerator_init()
    # resolves to bl2_main.c's stub and does nothing.
    # CRYPTO_HW_ACCELERATOR must be on bl2_crypto_config itself, not only inherited from
    # tfm_config: mcuboot-mbedtls-cfg.h includes the accelerator config only under it, and
    # bl2_crypto does not link tfm_config. Without this the Mbed TLS library builds software
    # sha256.c with the native context while bootutil - which does see the define - uses
    # FSP's mbedtls_sha256_context: a struct-layout mismatch across translation units, not
    # merely a missed speed-up. Found that way: the first build linked upstream sha256.o.
    target_compile_definitions(bl2_crypto_config
        INTERFACE
            CRYPTO_HW_ACCELERATOR
            MBEDTLS_ACCELERATOR_CONFIG_FILE="${CMAKE_CURRENT_SOURCE_DIR}/bl2_mbedtls_accelerator_config.h"
            MBEDTLS_ACCELERATOR_PSA_CRYPTO_CONFIG_FILE="${CMAKE_CURRENT_SOURCE_DIR}/bl2_crypto_accelerator_config.h"
    )

    add_library(bl2_crypto_hw STATIC)

    target_sources(bl2_crypto_hw
        PRIVATE
            ${SCE9_RM_PSA_CRYPTO_DIR}/sha256_alt.c
            ${SCE9_RM_PSA_CRYPTO_DIR}/sha256_alt_process.c
    )

    # bl2_crypto brings the BL2 Mbed TLS config and library/ (PUBLIC there); fsp_sce_bl2
    # brings HW_SCE_* and the FSP include paths for the bootloader role (flat, no TZ defines).
    target_link_libraries(bl2_crypto_hw
        PRIVATE
            bl2_crypto
            fsp_sce_bl2
    )

    target_compile_options(bl2_crypto_hw
        PRIVATE
            ${BL2_COMPILER_CP_FLAG}
    )

    target_link_libraries(bl2_crypto
        PUBLIC
            bl2_crypto_hw
    )

    # Everything compiling against the BL2 Mbed TLS config now includes sce9_fsp_cfg.h
    # (bsp_api.h) and, via mbedtls/sha256.h, FSP's sha256_alt.h (which includes the internal
    # "common.h") - so it needs these paths and the FSP defines.
    # ${MBEDCRYPTO_PATH}/include LAST: FSP's psa_crypto_driver_wrappers.h includes
    # "psa/crypto_accel_driver.h", which only exists in the crypto library's own include
    # tree, while BL2 otherwise takes its psa/*.h from TF-M's interface/include - which is
    # added first and so still wins.
    target_include_directories(bl2_crypto_config
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${SCE9_RM_PSA_CRYPTO_DIR}/inc
            ${MBEDCRYPTO_PATH}/library
            ${MBEDCRYPTO_PATH}/include
            $<TARGET_PROPERTY:fsp_sce_bl2,INTERFACE_INCLUDE_DIRECTORIES>
    )
    target_compile_definitions(bl2_crypto_config
        INTERFACE
            $<TARGET_PROPERTY:fsp_sce_bl2,INTERFACE_COMPILE_DEFINITIONS>
    )
endif()

############################ Secure crypto partition ###########################

if(TFM_PARTITION_CRYPTO)
    # FSP's SCE9 set: the generic cipher layer, AES, GCM, CCM, CMAC, SHA-256, ECC and RSA.
    # Each *_alt.c replaces the matching Mbed TLS library file and each *_process.c holds its
    # SCE9 calls. Adding a module means adding its pair here AND its MBEDTLS_*_ALT in
    # mbedtls_accelerator_config.h.
    #
    # ccm_alt.c/ccm_alt_process.c are absent on purpose - FSP's SCE9 CCM caps associated data
    # at 110 B and Protected Storage exceeds it (D043, mbedtls_accelerator_config.h).
    #
    # cipher_alt.c is not optional with the rest: it supplies the block chunking the AES ALT
    # assumes (whole blocks only, partial blocks cached in the cipher context) and issues the
    # SCE session FinalSub at finish. Without it, upstream cipher.c hands partial blocks
    # straight to the engine and never closes the session.
    set(SCE9_ALT_SOURCES
        cipher_alt.c
        aes_alt.c           aes_alt_process.c
        gcm_alt.c           gcm_alt_process.c
        cmac_alt.c
        sha256_alt.c        sha256_alt_process.c
        ecp_alt.c           ecp_alt_process.c
        ecp_curves_alt.c
        ecdsa_alt.c         ecdsa_alt_process.c
        rsa_alt.c           rsa_alt_process.c
    )
    list(TRANSFORM SCE9_ALT_SOURCES PREPEND "${SCE9_RM_PSA_CRYPTO_DIR}/")

    target_sources(crypto_service_crypto_hw
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/crypto_hw.c
            ${SCE9_ALT_SOURCES}
    )

    target_include_directories(crypto_service_crypto_hw
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${SCE9_RM_PSA_CRYPTO_DIR}/inc
        PRIVATE
            # The ALT sources are library files: they include Mbed TLS internals by bare
            # name ("common.h", "bn_mul.h", "bignum_internal.h", ...).
            ${MBEDCRYPTO_PATH}/library
    )

    # r_sce provides the HW_SCE_* procedures; platform_s provides ra_sce_init().
    target_link_libraries(crypto_service_crypto_hw
        PRIVATE
            crypto_service_mbedcrypto
            platform_s
            fsp_sce_s
    )

    target_link_libraries(crypto_service_mbedcrypto
        PUBLIC
            crypto_service_crypto_hw
    )

    # LEGACY_DRIVER_API_ENABLED is what makes crypto_init.c call crypto_hw_accelerator_init()
    # before psa_crypto_init().
    target_compile_definitions(crypto_service_mbedcrypto
        PUBLIC
            LEGACY_DRIVER_API_ENABLED
    )

    # Everything that compiles against the Mbed TLS config now sees
    # mbedtls_accelerator_config.h, which includes bsp_api.h and makes mbedtls/*.h pull the
    # FSP *_alt.h context definitions - so it needs this directory, rm_psa_crypto/inc and
    # the FSP/SCE include paths and defines. Taken from fsp_sce_s rather than linking it, so
    # the FSP library is not dragged into every consumer of the config.
    #
    # BUILD_INTERFACE: both config targets are exported to the installed api_ns tree, and
    # CMake refuses source-tree paths in an exported interface.
    target_include_directories(psa_crypto_library_config
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<BUILD_INTERFACE:${SCE9_RM_PSA_CRYPTO_DIR}/inc>
            $<BUILD_INTERFACE:$<TARGET_PROPERTY:fsp_sce_s,INTERFACE_INCLUDE_DIRECTORIES>>
            # FSP's *_alt.h headers include Mbed TLS's internal "common.h" (sha256_alt.h
            # does), and mbedtls/<module>.h includes the *_alt.h once the ALT is defined -
            # so every consumer of the public headers needs the library directory too.
            $<BUILD_INTERFACE:${MBEDCRYPTO_PATH}/library>
    )
    target_compile_definitions(psa_crypto_library_config
        INTERFACE
            $<BUILD_INTERFACE:$<TARGET_PROPERTY:fsp_sce_s,INTERFACE_COMPILE_DEFINITIONS>>
    )

    target_include_directories(psa_crypto_config
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )
endif()
