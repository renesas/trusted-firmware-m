#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#-------------------------------------------------------------------------------
#
# Post-link check: report allocatable ORPHAN sections.
#
# Why this exists
# ---------------
# FSP's linker script is generated per project. `<project>/script/fsp.ld` is a stub that
# INCLUDEs `memory_regions.ld` and `fsp_gen.ld`, and fsp_gen.ld carries the device's memory
# section contract - `.ram_from_flash`, `.ram_code_from_flash`, `.fsp_dtc_vector_table`,
# `.ram_nocache` / `.bss.*_fsp_nocache` at 32-byte alignment, `.ram_noinit`, `.qspi_flash*`,
# `.data_flash*`, the option_setting_* windows.
#
# tfm_s and bl2 do NOT use fsp.ld. tfm_s links TF-M's generated tfm_isolation_s.ld and bl2
# links ra6e1_bl2.ld. So an FSP section that neither script names is not diagnosed - GNU ld
# silently ORPHANS it, inventing an output section and placing it next to whatever looks
# similar. No warning, no error.
#
# That is not hypothetical. It is exactly how the r_flash_hp code-flash program/erase
# routines came to execute from code flash: FSP marks them PLACE_IN_RAM_SECTION
# (".ram_from_flash") because the FCU makes the whole code flash unreadable during a
# code-flash P/E, ld orphaned the section into .text, and the failure was a hardware hang
# with no diagnostic. See RA6E1_SOLUTION.md.
#
# Why not diff fsp_gen.ld against our scripts
# -------------------------------------------
# Because fsp_gen.ld is device boilerplate, not a per-module subset. The secure, bootloader
# and non-secure projects place the same 53 sections despite different module sets; they
# differ only in the TrustZone entries (.flash_nsc, .gnu.sgstubs*, .ram_nsc and the
# _sec/_sel option-setting variants). Diffing it would flag ~30 sections that nothing emits
# into. This check instead asks the only question that matters: is there a section in the
# LINKED IMAGE that the script never named?
#
# Why not --orphan-handling=warn
# ------------------------------
# ld has that flag, and it is the same idea, but it reports every non-allocatable orphan
# too - .debug_*, .comment and .ARM.attributes from every object. On tfm_s that is hundreds
# of lines, and TF-M links with -Wl,-fatal-warnings, so it cannot simply be switched on.
#
# How it decides
# --------------
# An orphan output section's name appears nowhere in the linker script - that is what made
# it an orphan. So: list allocatable, non-empty sections in the ELF, and report any whose
# name is not a token in the (preprocessed, comment-stripped) script. Substring matching,
# not grammar parsing: ld's output-section syntax is `.NAME <addr-expr> <(ATTRS)> :` where
# the address expression is arbitrary, and parsing it is not worth the fragility.
#
# Inputs: IMAGE, AXF, SCATTER (the PREPROCESSED script - pass
# $<TARGET_OBJECTS:<target>_scatter>), OBJDUMP, ALLOW (;-list), STRICT.
#-------------------------------------------------------------------------------

foreach(_var IMAGE AXF SCATTER OBJDUMP)
    if(NOT DEFINED ${_var})
        message(FATAL_ERROR "ra6e1_check_orphans: ${_var} not set")
    endif()
endforeach()

if(NOT EXISTS "${AXF}" OR NOT EXISTS "${SCATTER}")
    # Nothing to check rather than a spurious failure: a build that got this far without
    # both files has already failed for a better reason.
    return()
endif()

# Every section-shaped token the script mentions, comments removed so a name that only
# appears in a comment cannot mask a real orphan.
file(READ "${SCATTER}" _script)
string(REGEX REPLACE [[/\*([^*]|\*[^/])*\*/]] " " _script "${_script}")
string(REGEX MATCHALL [[\.[A-Za-z_][A-Za-z0-9_.$]*]] _named "${_script}")

execute_process(COMMAND "${OBJDUMP}" -h "${AXF}"
                OUTPUT_VARIABLE _dump RESULT_VARIABLE _rc ERROR_QUIET)
if(NOT _rc EQUAL 0)
    message(WARNING "RA6E1 [${IMAGE}]: objdump failed, orphan check skipped")
    return()
endif()

string(REPLACE ";" "," _dump "${_dump}")
string(REPLACE "\n" ";" _lines "${_dump}")

set(_orphans "")
set(_pending "")
foreach(_line ${_lines})
    if(_pending)
        # The flags line follows the header line. Only allocatable sections can be
        # mis-placed in a way that matters; .debug_*, .comment and .ARM.attributes cannot.
        if(_line MATCHES "ALLOC")
            list(FIND _named "${_pending_name}" _found)
            if(_found EQUAL -1)
                list(FIND ALLOW "${_pending_name}" _allowed)
                if(_allowed EQUAL -1)
                    list(APPEND _orphans "${_pending_name} (${_pending_size} bytes at ${_pending_vma})")
                endif()
            endif()
        endif()
        set(_pending "")
    endif()
    if(_line MATCHES "^[ \t]*[0-9]+[ \t]+([^ \t]+)[ \t]+([0-9a-f]+)[ \t]+([0-9a-f]+)[ \t]")
        if(NOT CMAKE_MATCH_2 STREQUAL "00000000")
            set(_pending TRUE)
            set(_pending_name "${CMAKE_MATCH_1}")
            set(_pending_vma "0x${CMAKE_MATCH_3}")
            math(EXPR _pending_size "0x${CMAKE_MATCH_2}")
        endif()
    endif()
endforeach()

if(_orphans)
    string(REPLACE ";" "\n    " _list "${_orphans}")
    set(_msg
        "RA6E1 [${IMAGE}]: allocatable ORPHAN section(s) - present in the image, named "
        "nowhere in the linker script, so ld chose the address:\n    ${_list}\n"
        "An FSP module placed here by fsp_gen.ld but not by this image's script will end up "
        "in the wrong memory. For .ram_from_flash that is a hardware hang, not a fault. "
        "Either handle the section in the script, or add it to RA6E1_ORPHAN_ALLOW in the "
        "platform CMakeLists with a comment saying why it is harmless.")
    if(STRICT)
        message(FATAL_ERROR ${_msg})
    else()
        message(WARNING ${_msg})
    endif()
endif()
