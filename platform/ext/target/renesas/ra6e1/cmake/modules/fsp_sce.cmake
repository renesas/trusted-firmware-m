# FSP SCE Module Library
# SCE9 crypto engine. TF-M uses this module for ONE thing: the TRNG, which backs PSA's
# external RNG (see sce_trng.c). The *_ALT hardware-accelerated cipher path is a separate,
# later project - it needs FSP's mbedTLS and rm_psa_crypto, whose generated config pulls
# bsp_api.h and wires PSA ITS to littlefs, which collides with TF-M's ITS. See DESIGN.md 6.
#
# Adding a module: fsp_cmake/TFM_INTEGRATION_COMPLETE.md, "Adding New FSP Modules".

if(NOT DEFINED FSP_MODULE_BASE_DIR)
    message(FATAL_ERROR "fsp_sce: FSP_MODULE_BASE_DIR not defined. Set it to the generated "
                        "FSP project root before including this module.")
endif()
if(NOT DEFINED FSP_MODULE_INCLUDES)
    message(FATAL_ERROR "fsp_sce: FSP_MODULE_INCLUDES not defined. It carries the BSP include "
                        "paths every FSP module needs.")
endif()

set(FSP_SCE_DIR "${FSP_MODULE_BASE_DIR}/ra/fsp/src/r_sce")
if(NOT EXISTS "${FSP_SCE_DIR}")
    message(FATAL_ERROR
        "RA6E1: the SCE module is not in ${FSP_MODULE_BASE_DIR}.\n"
        "TF-M takes its entropy from the SCE9 TRNG, so this module is required. Add the "
        "Crypto (rm_psa_crypto) stack in e2 studio - it pulls r_sce in - regenerate, and "
        "rebuild the project once.")
endif()

# Guard: the S and BL2 sides may both include this file in one configure.
if(NOT TARGET fsp_sce)
    add_library(fsp_sce STATIC)
endif()

# Globbed WITHIN the module, which is the granularity the module convention allows. r_sce
# is ~180 generated primitive files whose individual names are an FSP implementation
# detail; naming them here would drift on every FSP bump. Only the TRNG path is reachable,
# and -ffunction-sections + --gc-sections drops the rest at link.
file(GLOB_RECURSE FSP_SCE_SOURCES "${FSP_SCE_DIR}/*.c")
if(NOT FSP_SCE_SOURCES)
    message(FATAL_ERROR "RA6E1: no sources under ${FSP_SCE_DIR} - is the project generated?")
endif()
target_sources(fsp_sce PRIVATE ${FSP_SCE_SOURCES})

# SCE keeps its private headers beside the sources rather than in ra/fsp/inc, so these are
# not covered by the BSP include paths.
target_include_directories(fsp_sce
    PUBLIC
        ${FSP_MODULE_INCLUDES}
        "${FSP_SCE_DIR}"
        "${FSP_SCE_DIR}/common"
        "${FSP_SCE_DIR}/crypto_procedures/src/sce9/plainkey/private/inc"
        "${FSP_SCE_DIR}/crypto_procedures/src/sce9/plainkey/public/inc"
        "${FSP_SCE_DIR}/crypto_procedures/src/sce9/plainkey/primitive"
)

target_compile_definitions(fsp_sce PUBLIC ${FSP_COMPILE_DEFS} _RA_TZ_SECURE=1)

# -mcmse: this library is linked into the secure image.
target_compile_options(fsp_sce
    PRIVATE
        ${COMPILER_CMSE_FLAG}
        ${FSP_COMPILE_OPTIONS}
)
