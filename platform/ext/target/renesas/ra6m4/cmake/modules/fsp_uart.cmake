# FSP UART Module Library
# This library provides SCI UART driver functionality
#
# When FSP_S_APP_DIR is set, sources are pulled from the external RASC project.
# Otherwise, sources are pulled from the embedded fsp/ directory.

# Determine FSP source directory (use same logic as fsp_bsp.cmake)
if(FSP_S_APP_DIR)
    set(FSP_S_DIR ${FSP_S_APP_DIR})
else()
    set(FSP_S_DIR ${CMAKE_CURRENT_LIST_DIR}/../../fsp)
endif()

if(NOT TARGET fsp_uart)
    add_library(fsp_uart STATIC)
endif()

# UART source files
target_sources(fsp_uart
    PRIVATE
        ${FSP_S_DIR}/ra/fsp/src/r_sci_uart/r_sci_uart.c
)

# UART depends on BSP
target_link_libraries(fsp_uart
    PUBLIC
        fsp_bsp
)
