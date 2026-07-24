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

# The SCI UART driver is optional: a flat MCUboot/BL2 RASC project has no r_sci_uart
# (console is SEGGER RTT here). If the source isn't present, skip this module cleanly
# rather than erroring with "No SOURCES given to target: fsp_uart".
if(NOT EXISTS ${FSP_S_DIR}/ra/fsp/src/r_sci_uart/r_sci_uart.c)
    message(STATUS "FSP UART: no r_sci_uart in ${FSP_S_DIR} - skipping (console is RTT)")
    if(NOT TARGET fsp_uart)
        add_library(fsp_uart INTERFACE)   # empty stub so link references resolve
    endif()
    return()
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

# UART compile options - must include -mcmse for TrustZone secure builds
target_compile_options(fsp_uart
    PRIVATE
        -mcmse             # Enable CMSE intrinsics for TrustZone
)
