# FSP SCE Module Library
# RSIP-E50D crypto engine - the HW_SCE_* procedures. The module directory is still r_sce:
# FSP routes E50D through the same driver, with the engine-specific procedures under
# crypto_procedures/src/rsip_e50d (DECISIONS D050). This module builds r_sce only. The
# rm_psa_crypto *_ALT sources that drive it are compiled by the accelerator
# (platform/ext/accelerator/renesas/rsip_e50d) into TF-M's own Mbed TLS, NOT through FSP's
# mbedTLS/rm_psa_crypto config - that config wires PSA ITS to littlefs, which collides with
# TF-M's ITS (DESIGN.md 6).
#
# Secure image: the TRNG always; the cipher ALT path when CRYPTO_HW_ACCELERATOR is on
# (platform/ext/accelerator/renesas/rsip_e50d).
#
# BL2: only with CRYPTO_HW_ACCELERATOR, for the SHA-256 image hash. BL2 never needs
# randomness - it builds with MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG and a weak no-op provider - so
# without the accelerator the bootloader role does not declare this module, even though its
# e2 project contains r_sce.
#
# Adding a module: fsp_cmake/TFM_INTEGRATION_COMPLETE.md, "Adding New FSP Modules".

fsp_module_library(FSP_SCE_TARGET sce)

set(_sce_dir "${FSP_MODULE_BASE_DIR}/ra/fsp/src/r_sce")
if(NOT EXISTS "${_sce_dir}")
    message(FATAL_ERROR
        "RA8M2: the SCE module is not in ${FSP_MODULE_BASE_DIR}.\n"
        "TF-M takes its entropy from the RSIP-E50D TRNG, so this module is required. Add the "
        "Crypto (rm_psa_crypto) stack in e2 studio - it pulls r_sce in - regenerate, and "
        "build the project once.")
endif()

# The generated primitive tree. Only the TRNG path is reachable from
# mbedtls_psa_external_get_random(); -ffunction-sections + --gc-sections drop the rest at
# link. The 8.9 KB figure measured on RA6M5 is NOT carried over - E50D has a different and
# larger procedure set (it adds SHA-512, SHA-3, ChaCha, ML-KEM, ML-DSA), so re-measure on
# this part rather than assuming.
#
# Beware of sizing this module from a linker map without skipping everything above
# "Linker script and memory map" - the discarded-section list at the top of the map makes
# the same objects look like ~185 KB.
fsp_module_glob(_src "ra/fsp/src/r_sce")
target_sources(${FSP_SCE_TARGET} PRIVATE ${_src})

# The engine keeps its private headers beside the sources rather than in ra/fsp/inc.
#
# The engine subdirectory is DISCOVERED, not hardcoded - "rsip_e50d" here, "sce9" on the RA6
# parts. fsp_bsp.cmake needs the same three paths and discovers them the same way, so the two
# cannot drift; hardcoding the name in both is exactly how this port broke when it was seeded
# from RA6M5.
file(GLOB _sce_engines LIST_DIRECTORIES true "${_sce_dir}/crypto_procedures/src/*")
set(_sce_engine_dirs "")
foreach(_e IN LISTS _sce_engines)
    if(IS_DIRECTORY "${_e}")
        list(APPEND _sce_engine_dirs "${_e}")
    endif()
endforeach()
list(LENGTH _sce_engine_dirs _n_engines)
if(NOT _n_engines EQUAL 1)
    message(FATAL_ERROR
        "RA8M2: expected exactly one engine directory under "
        "${_sce_dir}/crypto_procedures/src, found ${_n_engines}: ${_sce_engine_dirs}")
endif()
list(GET _sce_engine_dirs 0 _sce_engine)

target_include_directories(${FSP_SCE_TARGET}
    PUBLIC
        "${_sce_dir}"
        "${_sce_dir}/common"
        "${_sce_engine}/plainkey/private/inc"
        "${_sce_engine}/plainkey/public/inc"
        "${_sce_engine}/plainkey/primitive"
)

target_link_libraries(${FSP_SCE_TARGET} PUBLIC fsp_bsp_${FSP_MODULE_ROLE})
