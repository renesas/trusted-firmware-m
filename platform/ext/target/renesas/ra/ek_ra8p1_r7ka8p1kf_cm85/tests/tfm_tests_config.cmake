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

if(NOT RA_NS_INTERFACE_MAILBOX)
    set(TEST_NS_MULTI_CORE OFF CACHE BOOL "Whether to build NS regression multi-core tests" FORCE)
endif()
