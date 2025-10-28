# FSP Flash Module Library
# This library provides Flash HP driver functionality

# FSP base directory is relative to this platform directory
set(FSP_BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)

add_library(fsp_flash STATIC)

# Flash source files
target_sources(fsp_flash
    PRIVATE
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/r_flash_hp/r_flash_hp.c
        ${FSP_BASE_DIR}/fsp/ra_gen/hal_data.c
)

# Flash depends on BSP
target_link_libraries(fsp_flash
    PUBLIC
        fsp_bsp
)
