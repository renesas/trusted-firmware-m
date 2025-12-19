# FSP Flash Module Library
# This library provides Flash HP driver functionality
#
# When FSP_S_APP_DIR is set, sources are pulled from the external RASC project.
# Otherwise, sources are pulled from the embedded fsp/ directory.

# Determine FSP source directory (use same logic as fsp_bsp.cmake)
if(FSP_S_APP_DIR)
    set(FSP_S_DIR ${FSP_S_APP_DIR})
else()
    set(FSP_S_DIR ${CMAKE_CURRENT_LIST_DIR}/../../fsp)
endif()

if(NOT TARGET fsp_flash)
    add_library(fsp_flash STATIC)
endif()

# Flash source files
target_sources(fsp_flash
    PRIVATE
        ${FSP_S_DIR}/ra/fsp/src/r_flash_hp/r_flash_hp.c
        ${FSP_S_DIR}/ra_gen/hal_data.c
)

# Flash depends on BSP
target_link_libraries(fsp_flash
    PUBLIC
        fsp_bsp
)
