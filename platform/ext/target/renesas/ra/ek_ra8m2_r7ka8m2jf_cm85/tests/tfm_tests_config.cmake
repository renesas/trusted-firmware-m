#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: BSD-3-Clause
#-------------------------------------------------------------------------------

# Override the default test configurations at:
# - tf-m-tests/tests_reg/test/config/default_s_test_config.cmake
# - tf-m-tests/tests_reg/test/config/default_ns_test_config.cmake

set(TEST_NS_ATTESTATION      OFF CACHE BOOL "Whether to build NS regression Attestation tests")
# TODO: - Fix build issue with TEST_NS_ATTESTATION enabled
#       - Support and enable other default OFF tests suite
#         (default_s_test_config.cmake, default_ns_test_config.cmake).

# Flash slot sizes for the regression tests: give the test SPE a larger S slot,
# taken from the CPU1 NS slot (platform defaults in config.cmake: S=0x50000,
# CPU1 NS=0x40000). config.cmake has already cached the platform defaults, hence
# FORCE; the NS image offsets are derived afterwards in
# common/core/post_config.cmake.
set(FLASH_S_PARTITION_SIZE           "0x70000" CACHE STRING "Secure image slot size" FORCE)
if(RA_CM33_NS_PRESENT)
    set(FLASH_CPU1_NS_PARTITION_SIZE "0x20000" CACHE STRING "CPU1 NS image slot size" FORCE)
endif()

if(NOT RA_NS_INTERFACE_MAILBOX)
    set(TEST_NS_MULTI_CORE OFF CACHE BOOL "Whether to build NS regression multi-core tests" FORCE)
endif()
