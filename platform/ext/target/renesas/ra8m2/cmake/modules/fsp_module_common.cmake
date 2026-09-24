# Shared scaffolding for the FSP module libraries.
#
# Modules are per ROLE. The secure image and the bootloader are generated from DIFFERENT
# e2 projects (ra8m2_secure, ra8m2_mcuboot) and CMake target names are global, so each
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
    # FSP_MODULE_TZ_DEFS is per role: _RA_TZ_SECURE=1 for the secure image, empty for BL2,
    # which is flat like the e2 bootloader. See the note in the platform CMakeLists.
    target_compile_definitions(${${_out}} PUBLIC ${FSP_COMPILE_DEFS} ${FSP_MODULE_TZ_DEFS})

    # The FP/ABI flag, PER ROLE. Not optional, and the reason is worth stating.
    #
    # TF-M applies COMPILER_CP_FLAG per TARGET - platform/CMakeLists.txt and
    # secure_fw/CMakeLists.txt each add it to the targets they create - not globally through
    # CMAKE_C_FLAGS. The FSP module libraries are created here, so nothing was giving it to
    # them and they compiled with the compiler's DEFAULT FPU for the -mcpu/--cpu in use.
    #
    # On Cortex-M33 that was invisible: iccarm's default FPU there is none, which happens to
    # match the --fpu=none the link uses at CONFIG_TFM_FLOAT_ABI=soft. Cortex-M85 defaults to
    # a present FPU, so the FSP objects carried VFP while the link did not, and ILINK refused
    # the image outright:
    #
    #     Error[Lt006]: Incompatible object(s): system.o(libfsp_bsp_s.a) and 195 other
    #       objects ... use VFP instructions incompatible with No vfp (provided as FPU option)
    #
    # Taken from the same variables the rest of the build uses rather than hardcoding
    # --fpu=none, so this follows CONFIG_TFM_FLOAT_ABI / CONFIG_TFM_ENABLE_FP if the port ever
    # enables hardware FP. BL2 has its own variable because TF-M builds the bootloader
    # soft-float regardless of what the secure image does.
    if(FSP_MODULE_ROLE STREQUAL "bl2")
        set(_fsp_cp_flag ${BL2_COMPILER_CP_FLAG})
    else()
        set(_fsp_cp_flag ${COMPILER_CP_FLAG})
    endif()

    # -mcmse: every module here is linked into a secure-side image (S or BL2).
    target_compile_options(${${_out}}
        PRIVATE
            ${COMPILER_CMSE_FLAG}
            ${_fsp_cp_flag}
            ${FSP_COMPILE_OPTIONS}
    )
endmacro()

# Glob one FSP module directory. Globbing WITHIN a declared module is the granularity the
# module convention allows: the module is opt-in, but naming each generated .c inside it
# would drift on every FSP bump.
macro(fsp_module_glob _out _subdir)
    file(GLOB_RECURSE ${_out} "${FSP_MODULE_BASE_DIR}/${_subdir}/*.c")
endmacro()
