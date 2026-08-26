# FSP BSP Module Library
# Core BSP, clocks, IRQ, IOPORT, board files and the generated ra_gen instance data.
# Every other FSP module links this one.
#
# Adding a module: fsp_cmake/TFM_INTEGRATION_COMPLETE.md, "Adding New FSP Modules".

fsp_module_library(FSP_BSP_TARGET bsp)

fsp_module_glob(_bsp_src      "ra/fsp/src/bsp")
fsp_module_glob(_ioport_src   "ra/fsp/src/r_ioport")
fsp_module_glob(_ra_gen_src   "ra_gen")
fsp_module_glob(_board_src    "ra/board")
set(_src ${_bsp_src} ${_ioport_src} ${_ra_gen_src} ${_board_src})

# Files TF-M supplies itself for the S and BL2 images; taking FSP's would collide at link
# time. (The NS image keeps all three - it is a whole FSP application and is not built
# from these modules.)
#   ra_gen/main.c        - TF-M has its own main
#   .../Source/startup.c - TF-M has its own Reset_Handler and vector table
#   bsp_linker.c         - emits the OFS option-setting sections. TF-M emits those from
#                          bl2_option_setting.c, BL2 only, with discrete MEMORY regions;
#                          FSP's version in the secure image would put option memory in a
#                          signed image (DESIGN.md 8.4). It also defines g_init_info and
#                          g_main_stack, which bsp_init_stub.c replaces below.
list(REMOVE_ITEM _src "${FSP_MODULE_BASE_DIR}/ra_gen/main.c")
list(REMOVE_ITEM _src "${FSP_MODULE_BASE_DIR}/ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.c")
list(FILTER _src EXCLUDE REGEX "/ra/fsp/src/bsp/mcu/[^/]+/bsp_linker\\.c$")

list(REMOVE_DUPLICATES _src)
if(NOT _src)
    message(FATAL_ERROR "RA6E1: no BSP sources under ${FSP_MODULE_BASE_DIR} - is it generated?")
endif()

# bsp_init_stub.c belongs in this module rather than platform_s/platform_bl2: FSP's
# system.c is here and references g_init_info and g_main_stack, which the stub defines
# weakly. platform_* is scanned before this library on the link line, so a definition
# sitting there would already have been passed over when system.o introduces the
# undefined reference.
target_sources(${FSP_BSP_TARGET}
    PRIVATE
        ${_src}
        "${CMAKE_CURRENT_LIST_DIR}/../../bsp_init_stub.c"
)

#-------------------------------------------------------------------------------
# Include paths for modules TF-M does NOT build
#
# The generated ra_gen headers include every enabled module's header unconditionally, even
# when nothing is instantiated from it. With the Crypto stack present, common_data.h pulls
# "psa/crypto.h", "psa/crypto_extra.h", "mbedtls/platform.h" and "rm_psa_crypto.h" though
# common_data.c instantiates nothing but IOPORT; in the bootloader project hal_data.h
# pulls "rm_mcuboot_port.h", which pulls "bootutil/image.h". Those headers must be
# resolvable to parse the generated files.
#
# They are PRIVATE, deliberately. FSP ships its own ra/arm/mbedtls/include/psa/crypto.h;
# exporting it would shadow TF-M's psa/crypto.h for sce_trng.c, attest_hal.c and the rest
# of the secure image. Confining the FSP sources to this library is what makes that safe -
# it is the main reason this is a library and not target_sources(platform_s ...).
#
#   ra_cfg/arm  - FSP's mbedtls/build_info.h does #include "mbedtls/config.h", which is
#                 the generated ra_cfg/arm/mbedtls/config.h.
#   r_sce/...   - rm_psa_crypto.h in turn includes hw_sce_private.h and
#                 hw_sce_trng_private.h. fsp_sce exports these too, but the BSP cannot
#                 link it (fsp_sce depends on the BSP, not the reverse), so name the paths
#                 here. Mirrors the include set in fsp_sce.cmake - keep them in step.
#-------------------------------------------------------------------------------
#   mcu-tools   - FSP's MCUboot headers, for rm_mcuboot_port.h -> bootutil/image.h ->
#                 flash_map_backend.h. Bootloader project only. TF-M links its own
#                 bootutil; these are here only so hal_data.h parses.
#
# This mirrors the include set e2 studio itself compiles ra_gen with. If a regenerated
# project needs another path, read it off rather than guessing:
#   <project>/Debug/compile_commands.json, the entry for ra_gen/hal_data.c
# Every entry is EXISTS-guarded, so the same list serves the secure and bootloader
# projects even though they enable different modules.
set(_generated_module_includes
    "${FSP_MODULE_BASE_DIR}/ra/fsp/src/rm_psa_crypto/inc"
    "${FSP_MODULE_BASE_DIR}/ra/arm/mbedtls/include"
    "${FSP_MODULE_BASE_DIR}/ra/arm/mbedtls/library"
    "${FSP_MODULE_BASE_DIR}/ra_cfg/arm"
    "${FSP_MODULE_BASE_DIR}/ra_cfg/arm/mbedtls"
    "${FSP_MODULE_BASE_DIR}/ra/fsp/src/r_sce"
    "${FSP_MODULE_BASE_DIR}/ra/fsp/src/r_sce/common"
    "${FSP_MODULE_BASE_DIR}/ra/fsp/src/r_sce/crypto_procedures/src/sce9/plainkey/private/inc"
    "${FSP_MODULE_BASE_DIR}/ra/fsp/src/r_sce/crypto_procedures/src/sce9/plainkey/public/inc"
    "${FSP_MODULE_BASE_DIR}/ra/fsp/src/r_sce/crypto_procedures/src/sce9/plainkey/primitive"
    "${FSP_MODULE_BASE_DIR}/ra/fsp/src/rm_mcuboot_port"
    "${FSP_MODULE_BASE_DIR}/ra/mcu-tools/MCUboot/boot/bootutil/include"
    "${FSP_MODULE_BASE_DIR}/ra/mcu-tools/MCUboot/boot/bootutil/src"
    "${FSP_MODULE_BASE_DIR}/ra_cfg/mcu-tools/include"
    "${FSP_MODULE_BASE_DIR}/ra_cfg/mcu-tools/include/mcuboot_config"
    "${FSP_MODULE_BASE_DIR}/ra_cfg/mcu-tools/include/sysflash"
)
foreach(_d ${_generated_module_includes})
    if(EXISTS "${_d}")
        target_include_directories(${FSP_BSP_TARGET} PRIVATE "${_d}")
    endif()
endforeach()
