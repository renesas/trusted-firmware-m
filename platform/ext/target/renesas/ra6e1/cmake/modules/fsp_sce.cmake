# FSP SCE Module Library
# SCE9 crypto engine. TF-M uses this module for ONE thing: the TRNG, which backs PSA's
# external RNG (see sce_trng.c). The *_ALT hardware-accelerated cipher path is a separate,
# later project - it needs FSP's mbedTLS and rm_psa_crypto, whose generated config pulls
# bsp_api.h and wires PSA ITS to littlefs, which collides with TF-M's ITS. See DESIGN.md 6.
#
# Secure image only. BL2 verifies signatures but never needs randomness - it builds with
# MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG and a weak no-op provider - so the bootloader role does
# not declare this module even though its e2 project contains r_sce.
#
# Adding a module: fsp_cmake/TFM_INTEGRATION_COMPLETE.md, "Adding New FSP Modules".

fsp_module_library(FSP_SCE_TARGET sce)

set(_sce_dir "${FSP_MODULE_BASE_DIR}/ra/fsp/src/r_sce")
if(NOT EXISTS "${_sce_dir}")
    message(FATAL_ERROR
        "RA6E1: the SCE module is not in ${FSP_MODULE_BASE_DIR}.\n"
        "TF-M takes its entropy from the SCE9 TRNG, so this module is required. Add the "
        "Crypto (rm_psa_crypto) stack in e2 studio - it pulls r_sce in - regenerate, and "
        "build the project once.")
endif()

# ~180 generated primitive files. Only the TRNG path is reachable from
# mbedtls_psa_external_get_random(); -ffunction-sections + --gc-sections drop the rest at
# link, which measured at about 8 KB of secure image for the whole module.
fsp_module_glob(_src "ra/fsp/src/r_sce")
target_sources(${FSP_SCE_TARGET} PRIVATE ${_src})

# SCE keeps its private headers beside the sources rather than in ra/fsp/inc.
# fsp_bsp.cmake mirrors this list for the generated ra_gen files - keep them in step.
target_include_directories(${FSP_SCE_TARGET}
    PUBLIC
        "${_sce_dir}"
        "${_sce_dir}/common"
        "${_sce_dir}/crypto_procedures/src/sce9/plainkey/private/inc"
        "${_sce_dir}/crypto_procedures/src/sce9/plainkey/public/inc"
        "${_sce_dir}/crypto_procedures/src/sce9/plainkey/primitive"
)

target_link_libraries(${FSP_SCE_TARGET} PUBLIC fsp_bsp_${FSP_MODULE_ROLE})
