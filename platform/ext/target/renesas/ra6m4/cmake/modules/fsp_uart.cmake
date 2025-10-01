# FSP UART Module Library
# This library provides SCI UART driver functionality

# FSP base directory is relative to this platform directory
set(FSP_BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)

add_library(fsp_uart STATIC)

# UART source files
target_sources(fsp_uart
    PRIVATE
        ${FSP_BASE_DIR}/fsp/ra/fsp/src/r_sci_uart/r_sci_uart.c
)

# UART depends on BSP
target_link_libraries(fsp_uart
    PUBLIC
        fsp_bsp
)
