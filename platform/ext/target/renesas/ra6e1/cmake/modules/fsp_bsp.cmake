# FSP BSP Module Library
# Core BSP, clocks, IRQ, IOPORT, the generated ra_gen instance data, and (for now) the
# flash driver. Every other FSP module links this one.
#
# Adding a module: fsp_cmake/TFM_INTEGRATION_COMPLETE.md, "Adding New FSP Modules".
#
# WHY THIS IS A LIBRARY AND NOT target_sources(platform_s ...):
# the generated ra_gen/common_data.h includes "psa/crypto.h", "psa/crypto_extra.h" and
# "mbedtls/platform.h" unconditionally once the Crypto stack is present in the project -
# even though common_data.c instantiates nothing but IOPORT. Satisfying those means
# putting FSP's own mbedTLS include directory on the path, and FSP ships its own
# ra/arm/mbedtls/include/psa/crypto.h. On platform_s that would shadow TF-M's psa/crypto.h
# for sce_trng.c, attest_hal.c and everything else compiled into the secure image.
# Keeping the FSP sources in their own target confines those headers to the files that
# actually need them (see FSP_GENERATED_CRYPTO_INCLUDES below, PRIVATE).
#
# Inputs: FSP_MODULE_BASE_DIR, FSP_MODULE_INCLUDES, FSP_MODULE_SOURCES.

if(NOT DEFINED FSP_MODULE_BASE_DIR)
    message(FATAL_ERROR "fsp_bsp: FSP_MODULE_BASE_DIR not defined.")
endif()
if(NOT DEFINED FSP_MODULE_SOURCES)
    message(FATAL_ERROR "fsp_bsp: FSP_MODULE_SOURCES not defined.")
endif()

if(NOT TARGET fsp_bsp)
    add_library(fsp_bsp STATIC)
endif()

# bsp_init_stub.c belongs in this module, not in platform_s: FSP's system.c (here)
# references g_init_info and g_main_stack, and the stub defines them weakly. platform_s is
# scanned before fsp_bsp on the link line, so a definition sitting there would already have
# been passed over by the time system.o introduces the undefined reference. ra6m4's
# fsp_bsp.cmake places it the same way.
target_sources(fsp_bsp
    PRIVATE
        ${FSP_MODULE_SOURCES}
        "${CMAKE_CURRENT_LIST_DIR}/../../bsp_init_stub.c"
)

# PUBLIC: platform_s compiles Driver_Flash.c and target_cfg.c, which include hal_data.h
# and bsp_api.h.
target_include_directories(fsp_bsp PUBLIC ${FSP_MODULE_INCLUDES})

# PRIVATE, deliberately - see the note above. rm_psa_crypto/inc gives rm_psa_crypto.h
# (which itself needs only bsp_api.h and the SCE private headers); ra/arm/mbedtls/include
# gives the psa/ and mbedtls/ headers ra_gen asks for. Neither propagates.
#   ra_cfg/arm      - FSP's mbedtls/build_info.h does #include "mbedtls/config.h", which
#                     is the generated ra_cfg/arm/mbedtls/config.h. Only reached by the
#                     ra_gen files; nothing here uses the crypto it configures.
#   r_sce/...       - rm_psa_crypto.h in turn includes hw_sce_private.h and
#                     hw_sce_trng_private.h. fsp_sce exports these too, but fsp_bsp does
#                     not link it (fsp_sce is a consumer of the BSP, not the reverse), so
#                     name the paths here rather than create a dependency cycle.
set(FSP_GENERATED_CRYPTO_INCLUDES
    "${FSP_MODULE_BASE_DIR}/ra/fsp/src/rm_psa_crypto/inc"
    "${FSP_MODULE_BASE_DIR}/ra/arm/mbedtls/include"
    "${FSP_MODULE_BASE_DIR}/ra_cfg/arm"
#                     This mirrors the include set in fsp_sce.cmake - keep them in step.
    "${FSP_MODULE_BASE_DIR}/ra/fsp/src/r_sce"
    "${FSP_MODULE_BASE_DIR}/ra/fsp/src/r_sce/common"
    "${FSP_MODULE_BASE_DIR}/ra/fsp/src/r_sce/crypto_procedures/src/sce9/plainkey/private/inc"
    "${FSP_MODULE_BASE_DIR}/ra/fsp/src/r_sce/crypto_procedures/src/sce9/plainkey/public/inc"
    "${FSP_MODULE_BASE_DIR}/ra/fsp/src/r_sce/crypto_procedures/src/sce9/plainkey/primitive"
)
foreach(_d ${FSP_GENERATED_CRYPTO_INCLUDES})
    if(EXISTS "${_d}")
        target_include_directories(fsp_bsp PRIVATE "${_d}")
    endif()
endforeach()

target_compile_definitions(fsp_bsp PUBLIC ${FSP_COMPILE_DEFS} _RA_TZ_SECURE=1)

target_compile_options(fsp_bsp
    PRIVATE
        ${COMPILER_CMSE_FLAG}
        ${FSP_COMPILE_OPTIONS}
)
