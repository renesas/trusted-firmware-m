#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# BSP clock options
# =======================
# This file is the single list of configurable clock settings, with their type
# and default value.
#
# Each option KEY becomes:
#   - a cache variable   RA_BSP_CLK_<KEY>         (can be overridden with -D)
#   - a generated define #define BSP_CFG_<KEY>    (in bsp_clock_cfg.h.in)
#
#   Declaration                              Accepted value   Emitted as
#   ---------------------------------------  ---------------  -----------------------------------
#   ra_clock_int  KEY DEFAULT                non-neg integer  (<n>)
#   ra_clock_enum KEY DEFAULT ALLOWED...     one of ALLOWED   (<n>)
#   ra_clock_div  KEY SET(*) PREFIX DEFAULT  <n> in SET       (BSP_CLOCKS_<PREFIX>_DIV_<n>)
#   ra_clock_src  KEY DEFAULT                source name      (BSP_CLOCKS_SOURCE_CLOCK_<name>)
#                                            DISABLED         (BSP_CLOCKS_CLOCK_DISABLED)
#   ra_clock_mul  KEY DEFAULT                <int>:<frac>     BSP_CLOCKS_PLL_MUL(<int>, <frac>)
#
# (*) SET is the divider table to check against: SYS, PLL or PERIPH.
#
# The configuration stops with an error if:
#   - a value is invalid for its option, or
#   - an unknown RA_BSP_CLK_* variable is passed.

#===================== Allowed values ==========================================#

set(_RA_CLK_DIVS_SYS    1 2 3 4 6 8 12 16 24 32 64 128)
set(_RA_CLK_DIVS_PLL    1 2 3 4 5 6 8 9 16)
set(_RA_CLK_DIVS_PERIPH 1 2 3 4 5 6 8 10 16 32)
set(_RA_CLK_SRCS        DISABLED HOCO MOCO LOCO MAIN_OSC SUBCLOCK PLL PLL2
                        PLL1P PLL2P PLL1Q PLL1R PLL2Q PLL2R)

#===================== Shared state ============================================#

set_property(GLOBAL PROPERTY RA_BSP_CLK_KEYS "")
set_property(GLOBAL PROPERTY RA_BSP_CLK_DEFINES "")

#===================== Helpers =================================================#

# Create cache variable RA_BSP_CLK_<key> with <default>, and register <key>.
function(_ra_clock_get key default out_var)
  set(RA_BSP_CLK_${key} "${default}" CACHE STRING "BSP_CFG_${key}")
  set_property(GLOBAL APPEND PROPERTY RA_BSP_CLK_KEYS ${key})
  set(${out_var} "${RA_BSP_CLK_${key}}" PARENT_SCOPE)
endfunction()

# Stop the configuration with a clear message.
function(_ra_clock_fail key val expected)
  message(FATAL_ERROR "RA_BSP_CLK_${key}='${val}' is not valid, expected ${expected}")
endfunction()

# Stop unless <val> is one of the remaining arguments.
function(_ra_clock_require_in key val)
  if(NOT val IN_LIST ARGN)
    _ra_clock_fail(${key} "${val}" "one of: ${ARGN}")
  endif()
endfunction()

# Append "#define BSP_CFG_<key> <expr>" to the generated header content.
function(_ra_clock_define key expr)
  set_property(GLOBAL APPEND_STRING PROPERTY RA_BSP_CLK_DEFINES
    "#define BSP_CFG_${key} ${expr}\n")
endfunction()

#===================== Option types ============================================#

# Any non-negative integer.
function(ra_clock_int key default)
  _ra_clock_get(${key} "${default}" val)
  if(NOT val MATCHES "^[0-9]+$")
    _ra_clock_fail(${key} "${val}" "a non-negative integer")
  endif()
  _ra_clock_define(${key} "(${val})")
endfunction()

# Integer that must be one of the values listed after the default.
function(ra_clock_enum key default)
  _ra_clock_get(${key} "${default}" val)
  _ra_clock_require_in(${key} "${val}" ${ARGN})
  _ra_clock_define(${key} "(${val})")
endfunction()

# Clock divider. divs: SYS, PLL or PERIPH. prefix: FSP enum family (e.g. SCI_CLOCK).
function(ra_clock_div key divs prefix default)
  _ra_clock_get(${key} "${default}" val)
  _ra_clock_require_in(${key} "${val}" ${_RA_CLK_DIVS_${divs}})
  _ra_clock_define(${key} "(BSP_CLOCKS_${prefix}_DIV_${val})")
endfunction()

# Clock source name, or DISABLED.
function(ra_clock_src key default)
  _ra_clock_get(${key} "${default}" val)
  _ra_clock_require_in(${key} "${val}" ${_RA_CLK_SRCS})
  if(val STREQUAL "DISABLED")
    _ra_clock_define(${key} "(BSP_CLOCKS_CLOCK_DISABLED)")
  else()
    _ra_clock_define(${key} "(BSP_CLOCKS_SOURCE_CLOCK_${val})")
  endif()
endfunction()

# PLL multiplier written as <integer>:<fraction>.
function(ra_clock_mul key default)
  _ra_clock_get(${key} "${default}" val)
  if(NOT val MATCHES "^([0-9]+):([0-9]+)$")
    _ra_clock_fail(${key} "${val}" "<integer>:<fraction>")
  endif()
  _ra_clock_define(${key} "BSP_CLOCKS_PLL_MUL(${CMAKE_MATCH_1}, ${CMAKE_MATCH_2})")
endfunction()

#===================== Constants and oscillators ===============================#

ra_clock_enum(CLOCKS_SECURE    1  0 1)
ra_clock_enum(CLOCKS_OVERRIDE  0  0 1)
ra_clock_int (XTAL_HZ          24000000)
# HOCO frequency register code: 16/18/20/32/48 MHz.
ra_clock_enum(HOCO_FREQUENCY   7  0 1 2 4 7)

#===================== PLL1 ====================================================#

ra_clock_src (PLL_SOURCE        MAIN_OSC)
ra_clock_div (PLL_DIV           PLL PLL 3)
ra_clock_mul (PLL_MUL           250:0)
ra_clock_int (PLL_FREQUENCY_HZ  2000000000)

ra_clock_div (PLODIVP           PLL PLL 2)
ra_clock_int (PLL1P_FREQUENCY_HZ 1000000000)
ra_clock_div (PLODIVQ           PLL PLL 6)
ra_clock_int (PLL1Q_FREQUENCY_HZ 333333333)
ra_clock_div (PLODIVR           PLL PLL 5)
ra_clock_int (PLL1R_FREQUENCY_HZ 400000000)

#===================== PLL2 ====================================================#

ra_clock_src (PLL2_SOURCE       MAIN_OSC)
ra_clock_div (PLL2_DIV          PLL PLL 3)
ra_clock_mul (PLL2_MUL          300:0)
ra_clock_int (PLL2_FREQUENCY_HZ 2400000000)

ra_clock_div (PL2ODIVP          PLL PLL 4)
ra_clock_int (PLL2P_FREQUENCY_HZ 600000000)
ra_clock_div (PL2ODIVQ          PLL PLL 3)
ra_clock_int (PLL2Q_FREQUENCY_HZ 800000000)
ra_clock_div (PL2ODIVR          PLL PLL 5)
ra_clock_int (PLL2R_FREQUENCY_HZ 480000000)

#===================== System clocks ===========================================#

ra_clock_src (CLOCK_SOURCE      PLL1P)

ra_clock_div (CPUCLK_DIV        SYS SYS_CLOCK 1)
ra_clock_div (CPUCLK1_DIV       SYS SYS_CLOCK 4)
ra_clock_div (MRICLK_DIV        SYS SYS_CLOCK 4)
ra_clock_div (FCLK_DIV          SYS SYS_CLOCK 8)
ra_clock_div (ICLK_DIV          SYS SYS_CLOCK 4)
ra_clock_div (PCLKA_DIV         SYS SYS_CLOCK 8)
ra_clock_div (PCLKB_DIV         SYS SYS_CLOCK 16)
ra_clock_div (PCLKC_DIV         SYS SYS_CLOCK 8)
ra_clock_div (PCLKD_DIV         SYS SYS_CLOCK 4)
ra_clock_div (PCLKE_DIV         SYS SYS_CLOCK 4)

ra_clock_div (BCLK_DIV          SYS SYS_CLOCK 8)
ra_clock_src (BCLKA_SOURCE      DISABLED)
ra_clock_div (BCLKA_DIV         PERIPH BCLKA_CLOCK 6)
ra_clock_enum(SDCLK_OUTPUT      1  0 1)
ra_clock_enum(EBCLKA_SEL        0  0 1)
# BCLK pin output: 0 off, 1 BCLK, 2 BCLK/2.
ra_clock_enum(BCLK_OUTPUT       0  0 1 2)

#===================== Clock output and peripheral clocks ======================#

ra_clock_src (CLKOUT_SOURCE     DISABLED)
ra_clock_div (CLKOUT_DIV        SYS SYS_CLOCK 1)

ra_clock_src (SCICLK_SOURCE     PLL2R)
ra_clock_div (SCICLK_DIV        PERIPH SCI_CLOCK 4)

ra_clock_src (SPICLK_SOURCE     DISABLED)
ra_clock_div (SPICLK_DIV        PERIPH SPI_CLOCK 1)

ra_clock_src (CANFDCLK_SOURCE   DISABLED)
ra_clock_div (CANFDCLK_DIV      PERIPH CANFD_CLOCK 6)

ra_clock_src (GPTCLK_SOURCE     DISABLED)
ra_clock_div (GPTCLK_DIV        PERIPH GPT_CLOCK 2)
# GPT count clock selector (0/1), not a clock source enum.
ra_clock_enum(GPT_COUNT_CLOCK_SOURCE 1  0 1)

ra_clock_src (I3CCLK_SOURCE     DISABLED)
ra_clock_div (I3CCLK_DIV        PERIPH I3C_CLOCK 4)

ra_clock_src (UCLK_SOURCE       DISABLED)
ra_clock_div (UCLK_DIV          PERIPH USB_CLOCK 10)

ra_clock_src (USB60CLK_SOURCE   DISABLED)
ra_clock_div (USB60CLK_DIV      PERIPH USB60_CLOCK 8)

ra_clock_src (OCTACLK_SOURCE    DISABLED)
ra_clock_div (OCTACLK_DIV       PERIPH OCTA_CLOCK 1)

ra_clock_src (ADCCLK_SOURCE     DISABLED)
ra_clock_div (ADCCLK_DIV        PERIPH ADC_CLOCK 4)

ra_clock_src (ESWCLK_SOURCE     DISABLED)
ra_clock_div (ESWCLK_DIV        PERIPH ESW_CLOCK 4)

ra_clock_src (ESWPHYCLK_SOURCE  DISABLED)
ra_clock_div (ESWPHYCLK_DIV     PERIPH ESWPHY_CLOCK 2)

ra_clock_src (ETHPHYCLK_SOURCE  DISABLED)
ra_clock_div (ETHPHYCLK_DIV     PERIPH ETHPHY_CLOCK 32)

#===================== Reject unknown options ==================================#

# Any cache variable named RA_BSP_CLK_* that was not declared above is a typo.
get_property(_known_keys GLOBAL PROPERTY RA_BSP_CLK_KEYS)
get_cmake_property(_cache_vars CACHE_VARIABLES)

foreach(_var IN LISTS _cache_vars)
  if(_var MATCHES "^RA_BSP_CLK_(.+)$" AND NOT CMAKE_MATCH_1 IN_LIST _known_keys)
    message(FATAL_ERROR "Unknown BSP clock option ${_var}, see ${CMAKE_CURRENT_LIST_FILE}")
  endif()
endforeach()

#===================== Output ==================================================#

# Content for bsp_clock_cfg.h: all the "#define BSP_CFG_..." lines.
get_property(RA_BSP_CLK_DEFINES GLOBAL PROPERTY RA_BSP_CLK_DEFINES)

unset(_known_keys)
unset(_cache_vars)
unset(_var)
