# FSP Flash Module Library
# Flash HP driver. TF-M's cmsis_drivers/Driver_Flash.c wraps it as Driver_FLASH0 (code
# flash) and Driver_FLASH1 (data flash, where ITS/PS/NV counters live), with the RA 32 KB
# region-1 erase geometry (DESIGN.md 4).
#
# Adding a module: fsp_cmake/TFM_INTEGRATION_COMPLETE.md, "Adding New FSP Modules".

fsp_module_library(FSP_FLASH_TARGET flash)

fsp_module_glob(_src "ra/fsp/src/r_flash_hp")
if(NOT _src)
    message(FATAL_ERROR
        "RA6E1: the Flash HP module is not in ${FSP_MODULE_BASE_DIR}.\n"
        "Both the secure image (ITS/PS/NV counters on data flash) and BL2 (MCUboot slot "
        "access) need it. Add the Flash (r_flash_hp) module in e2 studio, regenerate, and "
        "build the project once.")
endif()

target_sources(${FSP_FLASH_TARGET} PRIVATE ${_src})

target_link_libraries(${FSP_FLASH_TARGET} PUBLIC fsp_bsp_${FSP_MODULE_ROLE})
