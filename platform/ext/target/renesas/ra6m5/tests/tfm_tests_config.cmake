#-------------------------------------------------------------------------------
# Copyright (c) 2026 Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
#
# Test capabilities this port declares to tf-m-tests. Included by
# tests_reg/CMakeLists.txt from the INSTALLED SPE tree, so it must live under
# <platform>/tests/ and is copied there by the platform install step.
#
# Until this file existed, tf-m-tests found nothing for any RA part. That is why the
# FLIH/SLIH suites were silently absent rather than reported as unsupported, and why
# PSA_API_TEST_TARGET had to be passed by hand on every invocation. DECISIONS D062.

#-------------------------------------------------------------------------------
# Interrupt tests
#-------------------------------------------------------------------------------
#
# All four prerequisites are now in place (DECISIONS D061, D068):
#   1. AGT0 (g_timer0) is configured in the SECURE e2 project with a real interrupt
#      priority - which is what makes the interrupt secure on RA, since attribution is
#      configurator-driven and there is no runtime API for it.
#   2. plat_test.c drives AGT0 start / stop / underflow-ack directly.
#   3. TFM_PERIPHERAL_TIMER0 and TFM_TIMER0_IRQ are in tfm_peripherals_def.h, and
#      plat_test.c #errors if TFM_TIMER0_IRQ stops matching the generated
#      VECTOR_NUMBER_AGT0_INT.
#   4. __VECTOR_TABLE[16] in startup_ra6m5.c points at TFM_TIMER0_IRQ_Handler, and
#      tfm_timer0_irq.c supplies the handler and the tfm_timer0_irq_init() hook.
#
# These two flags are what tests_reg/test/config/config.cmake:78-86 reads to auto-enable
# TEST_NS_FLIH_IRQ / TEST_NS_SLIH_IRQ alongside TEST_NS, so they gate every TEST_NS
# build - which is why they stayed off until the platform side existed.
#
# REQUIRES THE SECURE PROJECT TO HAVE THE AGT INTERRUPT ENABLED. plat_test.c fails the
# build with an explanatory #error if VECTOR_NUMBER_AGT0_INT is absent, i.e. if the AGT
# ipl was left Disabled in RASC. As of 2026-09-30 that is true of ra6m5_iar_secure and
# NOT of ra6m5_gcc_secure.
#
# The manifests that consume these are tf-m-tests'
# suites/spm/irq/service/tfm_{flih,slih}_test_service.yaml, which require the
# TFM_PERIPHERAL_TIMER0 mmio_region and TFM_TIMER0_IRQ source verbatim.
set(PLATFORM_FLIH_IRQ_TEST_SUPPORT      ON   CACHE BOOL "Platform supports FLIH IRQ tests")
set(PLATFORM_SLIH_IRQ_TEST_SUPPORT      ON   CACHE BOOL "Platform supports SLIH IRQ tests")

#-------------------------------------------------------------------------------
# Firmware update - not supported
#-------------------------------------------------------------------------------
# PLATFORM_HAS_FIRMWARE_UPDATE_SUPPORT is not set: TFM_PARTITION_FIRMWARE_UPDATE is
# OFF on every RA part. The dual-slot flash layout groundwork exists, but the FWU
# bootloader shim and second-slot write path do not. TEST_S_FWU / TEST_NS_FWU are
# forced OFF by tests_reg config.cmake:51-54 as a result.
