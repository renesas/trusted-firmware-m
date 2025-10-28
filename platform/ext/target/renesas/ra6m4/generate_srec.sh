#!/bin/bash
#-------------------------------------------------------------------------------
# Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Post-build script to generate SREC files for RA6M4 platform
# Usage: ./generate_srec.sh [build_dir]

set -e

BUILD_DIR="${1:-build_ra6m4}"
BIN_DIR="${BUILD_DIR}/bin"

echo "=================================="
echo "Generating SREC files for RA6M4"
echo "=================================="

# Check if objcopy is available
if ! command -v arm-none-eabi-objcopy &> /dev/null; then
    echo "Error: arm-none-eabi-objcopy not found in PATH"
    exit 1
fi

# Generate SREC for BL2 bootloader
if [ -f "${BIN_DIR}/bl2.axf" ]; then
    echo "Generating bl2.srec..."
    arm-none-eabi-objcopy -O srec "${BIN_DIR}/bl2.axf" "${BIN_DIR}/bl2.srec"
    echo "  ✓ ${BIN_DIR}/bl2.srec"
else
    echo "  ⚠ BL2 binary not found, skipping"
fi

# Generate SREC for TF-M secure firmware
if [ -f "${BIN_DIR}/tfm_s.axf" ]; then
    echo "Generating tfm_s.srec..."
    arm-none-eabi-objcopy -O srec "${BIN_DIR}/tfm_s.axf" "${BIN_DIR}/tfm_s.srec"
    echo "  ✓ ${BIN_DIR}/tfm_s.srec"
else
    echo "  ⚠ TF-M secure binary not found, skipping"
fi

# Generate SREC for signed secure firmware (with correct base address)
if [ -f "${BIN_DIR}/tfm_s_signed.bin" ]; then
    echo "Generating tfm_s_signed.srec (base=0x00020000)..."
    arm-none-eabi-objcopy -I binary -O srec \
        --change-addresses=0x00020000 \
        "${BIN_DIR}/tfm_s_signed.bin" \
        "${BIN_DIR}/tfm_s_signed.srec"
    echo "  ✓ ${BIN_DIR}/tfm_s_signed.srec"
else
    echo "  ⚠ Signed secure binary not found, skipping"
fi

# Generate combined SREC (BL2 + signed secure image)
if [ -f "${BIN_DIR}/bl2.srec" ] && [ -f "${BIN_DIR}/tfm_s_signed.srec" ]; then
    echo "Generating combined SREC (BL2 + signed image)..."

    # Create header
    echo "S00F000074666D5F636F6D62696E656421" > "${BIN_DIR}/tfm_combined.srec"

    # Append BL2 data records (skip header and termination)
    grep "^S[123]" "${BIN_DIR}/bl2.srec" >> "${BIN_DIR}/tfm_combined.srec" || true

    # Append signed image data records
    grep "^S[123]" "${BIN_DIR}/tfm_s_signed.srec" >> "${BIN_DIR}/tfm_combined.srec" || true

    # Add termination record
    echo "S9030000FC" >> "${BIN_DIR}/tfm_combined.srec"

    echo "  ✓ ${BIN_DIR}/tfm_combined.srec"
    echo ""
    echo "Combined SREC contains:"
    echo "  - BL2 bootloader at 0x00000000"
    echo "  - Signed secure image at 0x00020000"
fi

echo ""
echo "SREC generation complete!"
echo "Files are in: ${BIN_DIR}/"
ls -lh "${BIN_DIR}"/*.srec 2>/dev/null || echo "No SREC files found"
