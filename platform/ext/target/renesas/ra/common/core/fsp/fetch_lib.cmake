#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

fetch_remote_library(
    LIB_NAME                renesas-ra-fsp
    LIB_SOURCE_PATH_VAR     RENESAS_RA_FSP_LIB_PATH
    LIB_PATCH_DIR           ${CMAKE_CURRENT_LIST_DIR}
    LIB_FORCE_PATCH         RENESAS_RA_FSP_FORCE_PATCH
    FETCH_CONTENT_ARGS
        GIT_REPOSITORY      ${RENESAS_RA_FSP_LIB_GIT_REMOTE}
        GIT_TAG             ${RENESAS_RA_FSP_LIB_VERSION}
        GIT_SHALLOW         TRUE
        GIT_PROGRESS        TRUE
        GIT_SUBMODULES      ""
)
