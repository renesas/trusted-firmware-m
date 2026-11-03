# FSP SCE Module Library
# SCE9 crypto engine: the HW_SCE_* procedures. This module builds r_sce only. The accelerator
# (platform/ext/accelerator/renesas/sce9) compiles the rm_psa_crypto *_ALT sources that drive
# it into TF-M's own Mbed TLS, not through FSP's mbedTLS/rm_psa_crypto config, which wires PSA
# ITS to littlefs and collides with TF-M's ITS (DESIGN.md 6).
#
# Secure image: the TRNG always; the cipher ALT path when CRYPTO_HW_ACCELERATOR is on.#
# BL2: only with CRYPTO_HW_ACCELERATOR, for the SHA-256 image hash. BL2 never needs
# randomness (it builds with MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG and a weak no-op provider), so
# without the accelerator the bootloader role does not declare this module, even though its
# e2 project contains r_sce.
#
# Adding a module: ra_fsp_cmake_tfm/ADDING_FSP_MODULES.md.

fsp_module_library(FSP_SCE_TARGET sce)

set(_sce_dir "${FSP_MODULE_BASE_DIR}/ra/fsp/src/r_sce")
if(NOT EXISTS "${_sce_dir}")
    message(FATAL_ERROR
        "RA6M5: the SCE module is not in ${FSP_MODULE_BASE_DIR}.\n"
        "TF-M takes its entropy from the SCE9 TRNG, so this module is required. Add the "
        "Crypto (rm_psa_crypto) stack in e2 studio - it pulls r_sce in - regenerate, and "
        "build the project once.")
endif()

# 181 generated primitive files, the same tree as RA6E1 (both SCE9). Most of the module links.
# Only with CRYPTO_HW_ACCELERATOR OFF is the TRNG path the only reachable one; with it on, the
# default, the ALT sources call HW_SCE_* directly and --gc-sections keeps every primitive that
# AES, GCM, CMAC, SHA-256, ECC and RSA reach. Measured 2026-11-03, Debug: tfm_s links 115 of
# these objects for 146.1 KB (128,330 text + 21,284 rodata) and 6,148 bytes of .bss; bl2, which
# uses SHA-256 only, links 20 for 9.5 KB. Size this from below "Linker script and memory map":
# the discarded-section list above it makes the same objects look like ~185 KB.
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
