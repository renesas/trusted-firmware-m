#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
#
# Parameters for the PSA Arch tests.

# psa-arch-tests has one target for the whole RA family,
# api-tests/platform/targets/tgt_dev_apis_tfm_renesas_ra, not one per part. Without this,
# tests_psa_arch/CMakeLists.txt:52-54 derives the target from the leaf of TFM_PLATFORM
# ("ra6m5", "ra8m2", ...), none of which exist, and every PSA Arch build needs
# -DPSA_API_TEST_TARGET=renesas_ra by hand. DECISIONS D062.
set(PSA_API_TEST_TARGET              renesas_ra   CACHE STRING  "PSA_API_TARGET name")

# NOTE: the IPC (PSA Firmware Framework) suite cannot be built for RA. It resolves
# tgt_ff_tfm_${PSA_API_TEST_TARGET} (tests_psa_arch/CMakeLists.txt:59-63), and only
# tgt_dev_apis_tfm_renesas_ra exists. An FF target also needs a secure-side pal_driver_intf.c
# backed by a real secure UART and watchdog, which the RA secure project does not instantiate.
# That suite is the only one that provokes intentional cross-partition boundary violations,
# so it is the main gap in isolation coverage.
