# FSP MCUboot Port - the flash_map_backend implementation BL2 links instead of TF-M's.
#
# BL2 ONLY. The secure and non-secure images have no MCUboot in them, and the module is not
# in their e2 projects.
#
# ==================== WHY FSP'S BACKEND AND NOT TF-M's ====================
#
# bl2/src/flash_map.c and this file implement the same seven flash_area_* entry points over
# different conventions. FSP's is used because it carries three things the port otherwise has
# to re-derive, and each one cost a hardware-only failure before it was understood:
#
#   1. THE ADDRESSES ARE IN THE TABLE. FSP's flash_map[] holds absolute addresses in fa_off.
#      On RA6M5 that is a formality - code flash is based at 0x00000000 and
#      BSP_FEATURE_TZ_NS_OFFSET is 0, so an absolute address and a flat offset are the same
#      number. It matters anyway for consistency with RA8M2, where they are 32 MB apart and
#      getting it wrong was a HardFault only visible on a partitioned part (D073, D074).
#
#   2. NOT the SAU - that half does not apply here. __SAUREGION_PRESENT is 0 on RA6M5, so
#      FSP's RM_MCUBOOT_PORT_CONFIGURE_SAU block compiles out entirely. It is the reason the
#      RA8M2 port needs this module; on RA6M5 the case rests on 1 and 3 alone.
#
#   3. ITS ERASE/WRITE/SECTOR LOGIC IS THE VENDOR'S. An FSP bug fix reaches the image on a
#      pack update and regenerate, instead of having to be re-found on hardware here.
#
# BSP_BOOTLOADER_PROJECT, which gates FSP's SAU block, is irrelevant here for the same
# reason - the block is already excluded by __SAUREGION_PRESENT == 0. The generated
# bsp_linker_info.h defines it anyway, because RASC knows the project is a bootloader.
#
# ==================== WHAT HAD TO CHANGE TO ADOPT IT ====================
#
#   DEFAULT_MCUBOOT_FLASH_MAP=OFF (config.cmake). This is the upstream-supported switch -
#   corstone1000 and rse both use it. It empties TF-M's sysflash.h and drops
#   bl2/src/default_flash_map.c, letting the generated bsp_linker_info.h supply flash_map[]
#   and FLASH_AREA_IMAGE_PRIMARY/SECONDARY instead. The two cannot coexist: TF-M defines
#   those as function-like macros and FSP as static inline functions of the same name, so
#   leaving the option ON fails to compile rather than silently half-adopting.
#
#   FSP's area IDs differ from TF-M's - 0P=1, 0S=2, 1P=3, 1S=4 against TF-M's 0=1, 1=2,
#   2=3, 3=4 - and that does not matter. Nothing in bootutil uses a raw numeric ID; it is a
#   platform parameter, and corstone1000's image-0 primary is FLASH_AREA_2_ID.
#
#   bl2/src/flash_map.c has to be excluded, or the seven entry points double-define. Upstream
#   makes the MAP overridable but not the BACKEND, so that needs a one-line guard in
#   bl2/CMakeLists.txt - see UPSTREAM_CHANGES.md item 13.
#
#   bl2/src/security_cnt.c stays TF-M's. rm_mcuboot_port does not implement NV rollback
#   counters (DESIGN.md 5).
#
# Adding a module: fsp_cmake/TFM_INTEGRATION_COMPLETE.md, "Adding New FSP Modules".

fsp_module_library(FSP_MCUBOOT_PORT_TARGET mcuboot_port)

# ONE FILE, named rather than globbed. fsp_module_glob() is GLOB_RECURSE, and this module
# directory also holds rm_mcuboot_port.c (the standalone bootloader's application entry -
# image validation and the jump, which is bl2_main.c's job here), custom_crypto_stacks/
# (FSP's SCE9/RSIP image-verification stacks, built against FSP's mbedTLS) and os/. None of
# those belong in TF-M's BL2, and none of them compile against it.
set(_src "${FSP_MODULE_BASE_DIR}/ra/fsp/src/rm_mcuboot_port/flash_map.c")
if(NOT EXISTS "${_src}")
    set(_src "")
endif()

if(NOT _src)
    message(FATAL_ERROR
        "RA6M5: the MCUboot port module is not in ${FSP_MODULE_BASE_DIR}.\n"
        "BL2 takes its flash_map_backend from FSP rather than from TF-M - see the note at "
        "the top of this file. Add the MCUboot module to the BOOTLOADER project in e2 "
        "studio, regenerate, and build the project once. Building BL2 without it needs "
        "DEFAULT_MCUBOOT_FLASH_MAP back ON and TF-M's bl2/src/flash_map.c back in the "
        "link, which also gives up the SAU programming BL2 needs on this part (D076).")
endif()

target_sources(${FSP_MCUBOOT_PORT_TARGET} PRIVATE ${_src})

# MCUboot's own headers - bootutil/, flash_map_backend/, sysflash/ - come from the BL2 build,
# not from FSP's copies under rm_mcuboot_port/. mcuboot_port_includes is set by the platform
# CMakeLists from MCUBOOT_PATH for exactly this.
target_include_directories(${FSP_MCUBOOT_PORT_TARGET} PRIVATE ${FSP_MCUBOOT_PORT_INCLUDES})

# TWO flash_map_backend.h EXIST AND BOTH ARE REACHABLE. FSP ships its own under
# rm_mcuboot_port/flash_map_backend/ (mbed-derived, guard H_UTIL_FLASH_MAP_, 209 lines);
# TF-M has one under bl2/ext/mcuboot/include/ (Arm's trim, guard __FLASH_MAP_BACKEND_H__,
# 103 lines). They declare the same struct and the same static inline accessors, so pulling
# in both is a redefinition error.
#
# flash_map.c gets FSP's whether we like it or not: it includes
# "flash_map_backend/flash_map_backend.h" in quotes, and the C rule is that the INCLUDING
# FILE'S OWN DIRECTORY is searched first - no -I order defeats that. So TF-M's is the one to
# silence, by pre-defining its guard. TF-M's include directory still has to be on the path,
# because sysflash/sysflash.h lives beside it.
#
# Safe because the two struct flash_area definitions are field-for-field identical -
# uint8_t fa_id, uint8_t fa_device_id, uint16_t pad16, uint32_t fa_off, uint32_t fa_size -
# so this object and bootutil agree on the layout. RE-CHECK THAT IF EITHER SIDE IS UPREVVED:
# it is the one thing here that would break silently rather than loudly.
target_compile_definitions(${FSP_MCUBOOT_PORT_TARGET} PRIVATE
    MCUBOOT_${MCUBOOT_UPGRADE_STRATEGY}
    __FLASH_MAP_BACKEND_H__)

# bootutil/boot_hooks.h HAS NO INCLUDES OF ITS OWN. It uses size_t and fih_ret and expects
# whoever includes it to have pulled <stddef.h> and bootutil/fault_injection_hardening.h
# first - which every MCUboot translation unit happens to do, and flash_map.c does not: it
# includes boot_hooks.h third, before bootutil_public.h. Force them in rather than editing
# FSP's source, which would make this a fork instead of a consumed file.
# SHELL: keeps each flag glued to its argument - without it CMake de-duplicates the repeated
# -include and the bare filenames arrive as input files ("cannot specify '-o' with '-c'").
if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    target_compile_options(${FSP_MCUBOOT_PORT_TARGET} PRIVATE
        "SHELL:--preinclude stddef.h"
        "SHELL:--preinclude bootutil/fault_injection_hardening.h"
        "SHELL:--preinclude ${CMAKE_CURRENT_SOURCE_DIR}/mcuboot_hook_shim.h")
else()
    target_compile_options(${FSP_MCUBOOT_PORT_TARGET} PRIVATE
        "SHELL:-include stddef.h"
        "SHELL:-include bootutil/fault_injection_hardening.h"
        "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/mcuboot_hook_shim.h")
endif()

target_link_libraries(${FSP_MCUBOOT_PORT_TARGET}
    PUBLIC  fsp_bsp_${FSP_MODULE_ROLE}
    PRIVATE fsp_flash_${FSP_MODULE_ROLE}    # R_FLASH_HP_Open/Write/Erase/Close
)
