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
# Interrupt tests - NOT YET SUPPORTED
#-------------------------------------------------------------------------------
#
# PLATFORM_FLIH_IRQ_TEST_SUPPORT / PLATFORM_SLIH_IRQ_TEST_SUPPORT are left
# NOT set. tests_reg/test/config/config.cmake:78-86 auto-enables TEST_NS_FLIH_IRQ /
# TEST_NS_SLIH_IRQ whenever the corresponding flag is ON and TEST_NS is requested, so
# turning either on before the platform side exists would break every TEST_NS build.
#
# To enable, all four of these are needed (see target_cfg.c for the interrupt model
# and DECISIONS D061):
#   1. A secure timer instance (GPT or AGT) in the SECURE e2 project - that is what
#      makes its interrupt secure on RA; attribution is configurator-driven.
#   2. plat_test.c implementing tfm_plat_test_secure_timer_start() / _stop() /
#      _clear_intr() from platform/include/tfm_plat_test.h. Model on
#      platform/ext/target/arm/mps2/an521/plat_test.c.
#   3. TFM_PERIPHERAL_TIMER0 and TFM_TIMER0_IRQ in tfm_peripherals_def.h, the IRQ
#      number taken from FSP's generated vector_data.h - never hardcoded, because
#      RASC assigns ICU slots densely in the order interrupts were added.
#   4. The vector-table override and the ISR/init pair, per DECISIONS D061.
#
# The manifests that consume these are tf-m-tests'
# suites/spm/irq/service/tfm_{flih,slih}_test_service.yaml, which require the
# TFM_PERIPHERAL_TIMER0 mmio_region and TFM_TIMER0_IRQ source verbatim.

#-------------------------------------------------------------------------------
# Firmware update - not supported
#-------------------------------------------------------------------------------
# PLATFORM_HAS_FIRMWARE_UPDATE_SUPPORT is not set: TFM_PARTITION_FIRMWARE_UPDATE is
# OFF on every RA part. The dual-slot flash layout groundwork exists, but the FWU
# bootloader shim and second-slot write path do not. TEST_S_FWU / TEST_NS_FWU are
# forced OFF by tests_reg config.cmake:51-54 as a result.
