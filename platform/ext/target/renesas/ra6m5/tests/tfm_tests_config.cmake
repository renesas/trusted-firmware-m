#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
#
# Test capabilities this port declares to tf-m-tests. tests_reg/CMakeLists.txt includes this
# file from the installed SPE tree, so it must live under <platform>/tests/; the platform
# install step copies it there.
#
# Without this file tf-m-tests finds nothing for an RA part: the FLIH/SLIH suites are absent
# rather than reported as unsupported. DECISIONS D062.

#-------------------------------------------------------------------------------
# Interrupt tests
#-------------------------------------------------------------------------------
#
# All prerequisites are in place (DECISIONS D061, D068, D069):
#   1. AGT0 (g_timer0) is configured in the secure e2 project with a real interrupt priority.
#      On RA that is what makes the interrupt secure: attribution is configurator-driven, and
#      there is no runtime API for it.
#   2. tfm_timer0_irq_init() in tfm_timer0_irq.c opens AGT0 with R_AGT_Open(); plat_test.c
#      starts, stops and acknowledges it with direct AGTCR writes.
#   3. TFM_PERIPHERAL_TIMER0 and TFM_TIMER0_IRQ are in tfm_peripherals_def.h, and plat_test.c
#      fails the build (_Static_assert) if TFM_TIMER0_IRQ stops matching the generated
#      VECTOR_NUMBER_AGT0_INT.
#   4. __VECTOR_TABLE[16] in startup_ra6m5.c points at TFM_TIMER0_IRQ_Handler, and
#      tfm_timer0_irq.c supplies the handler, which clears the ICU latch, and the
#      tfm_timer0_irq_init() hook.
#   5. target_cfg.c carries the anchor that forces the generated ICU event-link table to be
#      linked. Without it IELSR is never programmed and the interrupt never arrives (see
#      fsp_bsp.cmake).
#
# tests_reg/test/config/config.cmake:78-86 reads these two flags to enable TEST_NS_FLIH_IRQ /
# TEST_NS_SLIH_IRQ alongside TEST_NS, so they gate every TEST_NS build.
#
# FLIH and SLIH are mutually exclusive: both claim TIMER0, so tf-m-tests enables one.
# Select with -DTEST_NS_FLIH_IRQ=OFF -DTEST_NS_SLIH_IRQ=ON, or the reverse.
#
# Requires the secure project to have the AGT interrupt enabled. plat_test.c fails the build
# with an explanatory #error if VECTOR_NUMBER_AGT0_INT is absent, that is, if the AGT ipl was
# left Disabled in RASC. As of 2026-10-26 all four secure projects generate it,
# ra6m5_gcc_secure included.
#
# The manifests that consume these are tf-m-tests'
# suites/spm/irq/service/tfm_{flih,slih}_test_service.yaml, which require the
# TFM_PERIPHERAL_TIMER0 mmio_region and TFM_TIMER0_IRQ source verbatim.
set(PLATFORM_FLIH_IRQ_TEST_SUPPORT      ON   CACHE BOOL "Platform supports FLIH IRQ tests")
set(PLATFORM_SLIH_IRQ_TEST_SUPPORT      ON   CACHE BOOL "Platform supports SLIH IRQ tests")

#-------------------------------------------------------------------------------
# Firmware update - not supported
#-------------------------------------------------------------------------------
# PLATFORM_HAS_FIRMWARE_UPDATE_SUPPORT is not set: TFM_PARTITION_FIRMWARE_UPDATE is OFF on
# every RA part. The dual-slot flash layout exists, but the FWU bootloader shim and the
# second-slot write path do not. tests_reg config.cmake:47-50 therefore forces TEST_S_FWU /
# TEST_NS_FWU OFF.
