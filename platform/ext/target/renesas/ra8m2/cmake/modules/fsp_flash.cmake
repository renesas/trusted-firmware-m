# FSP Flash Module Library - MRAM (r_mram), not Flash HP.
#
# RA8M2 has MRAM and NO DATA FLASH, so this is not the r_flash_hp module RA6E1/RA6M5 build.
# TF-M's cmsis_drivers/Driver_Flash.c wraps the one driver as TWO CMSIS instances over the
# same array:
#
#   Driver_FLASH0 - the whole 1 MB MRAM, for BL2's four MCUboot slots
#   Driver_FLASH1 - windowed onto DF_EMULATION, for ITS, PS and the NV counters
#
# Both are needed in both roles: the secure image for storage, BL2 for slot access. See
# flash_layout.h for the partitioning and Driver_Flash.c for the offset-vs-address
# convention the two instances use.
#
# Adding a module: fsp_cmake/TFM_INTEGRATION_COMPLETE.md, "Adding New FSP Modules".

fsp_module_library(FSP_FLASH_TARGET flash)

fsp_module_glob(_src "ra/fsp/src/r_mram")
if(NOT _src)
    message(FATAL_ERROR
        "RA8M2: the MRAM module is not in ${FSP_MODULE_BASE_DIR}.\n"
        "Both the secure image (ITS/PS/NV counters in DF_EMULATION) and BL2 (MCUboot slot "
        "access) need it. Add the Flash (r_mram) module in e2 studio - it appears as "
        "'Flash' with the MRAM implementation on this part, NOT 'Flash (r_flash_hp)' - "
        "regenerate, and build the project once.")
endif()

target_sources(${FSP_FLASH_TARGET} PRIVATE ${_src})

target_link_libraries(${FSP_FLASH_TARGET} PUBLIC fsp_bsp_${FSP_MODULE_ROLE})
