# Shared scaffolding for the FSP module libraries.
#
# Modules are per ROLE. The secure image and the bootloader are generated from DIFFERENT
# e2 projects (ra6e1_secure, ra6e1_mcuboot) and CMake target names are global, so each
# module produces fsp_<name>_<role> rather than one shared fsp_<name>. That also keeps the
# two images' FSP configurations from leaking into each other, which matters: the
# bootloader project enables modules the secure project does not.
#
# Context, set by fsp_add_modules() in the platform CMakeLists:
#   FSP_MODULE_ROLE      s | bl2
#   FSP_MODULE_BASE_DIR  generated project root
#   FSP_MODULE_INCLUDES  BSP include paths for that project
#
# Adding a module: fsp_cmake/TFM_INTEGRATION_COMPLETE.md, "Adding New FSP Modules".

macro(fsp_module_require)
    foreach(_fsp_v FSP_MODULE_ROLE FSP_MODULE_BASE_DIR FSP_MODULE_INCLUDES)
        if(NOT DEFINED ${_fsp_v})
            message(FATAL_ERROR
                "FSP modules: ${_fsp_v} is not set. Module files are included through "
                "fsp_add_modules(), not directly.")
        endif()
    endforeach()
endmacro()

# Create this role's library for <base> and apply everything common to every FSP module.
# Sets <out> to the target name.
macro(fsp_module_library _out _base)
    fsp_module_require()
    set(${_out} "fsp_${_base}_${FSP_MODULE_ROLE}")

    if(NOT TARGET ${${_out}})
        add_library(${${_out}} STATIC)
    endif()

    # PUBLIC: consumers compile FSP headers too (Driver_Flash.c, target_cfg.c).
    target_include_directories(${${_out}} PUBLIC ${FSP_MODULE_INCLUDES})
    target_compile_definitions(${${_out}} PUBLIC ${FSP_COMPILE_DEFS} _RA_TZ_SECURE=1)

    # -mcmse: every module here is linked into a secure-side image (S or BL2).
    target_compile_options(${${_out}}
        PRIVATE
            ${COMPILER_CMSE_FLAG}
            ${FSP_COMPILE_OPTIONS}
    )
endmacro()

# Glob one FSP module directory. Globbing WITHIN a declared module is the granularity the
# module convention allows: the module is opt-in, but naming each generated .c inside it
# would drift on every FSP bump.
macro(fsp_module_glob _out _subdir)
    file(GLOB_RECURSE ${_out} "${FSP_MODULE_BASE_DIR}/${_subdir}/*.c")
endmacro()
