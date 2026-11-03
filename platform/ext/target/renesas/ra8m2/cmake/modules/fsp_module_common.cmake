# Shared scaffolding for the FSP module libraries.
#
# Modules are per role. The secure image and the bootloader come from different e2 projects
# (ra8m2_secure, ra8m2_mcuboot) and CMake target names are global, so each module produces
# fsp_<name>_<role> rather than one shared fsp_<name>. This also keeps the two images' FSP
# configurations apart; the bootloader project enables modules the secure project does not.
#
# Context, set by fsp_add_modules() in the platform CMakeLists:
#   FSP_MODULE_ROLE      s | bl2
#   FSP_MODULE_BASE_DIR  generated project root
#   FSP_MODULE_INCLUDES  BSP include paths for that project
#
# Adding a module: ra_fsp_cmake_tfm/ADDING_FSP_MODULES.md.

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

    # The FP/ABI flag, per role. TF-M applies COMPILER_CP_FLAG per target (platform/ and
    # secure_fw/CMakeLists.txt add it to the targets they create), not through CMAKE_C_FLAGS.
    # Without this, the FSP module libraries compile with the compiler's default FPU for the
    # -mcpu/--cpu in use.
    #
    # On Cortex-M33, iccarm's default FPU is none, which matches the --fpu=none link at
    # CONFIG_TFM_FLOAT_ABI=soft. On Cortex-M85 the default is a present FPU, so the FSP objects
    # carry VFP while the link does not, and ILINK rejects the image:
    #
    #     Error[Lt006]: Incompatible object(s): system.o(libfsp_bsp_s.a) and 195 other
    #       objects ... use VFP instructions incompatible with No vfp (provided as FPU option)
    #
    # The flag comes from the variables the rest of the build uses rather than a hardcoded
    # --fpu=none, so it follows CONFIG_TFM_FLOAT_ABI / CONFIG_TFM_ENABLE_FP if hardware FP is
    # enabled. BL2 has its own variable because TF-M builds the bootloader soft-float
    # regardless of the secure image.
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

# Glob one FSP module directory. Globbing within a declared module is the granularity the
# module convention allows: the module is opt-in, but a list of each generated .c inside it
# would drift on every FSP update.
macro(fsp_module_glob _out _subdir)
    file(GLOB_RECURSE ${_out} CONFIGURE_DEPENDS "${FSP_MODULE_BASE_DIR}/${_subdir}/*.c")
endmacro()
