#!/usr/bin/env python3
"""
check_ofs.py - BRICK GUARD for RA6M4/RA6E1 option-setting (OFS) memory.

Root cause of the two bricked RA6M4 boards (DESIGN.md 8.4): the OFS words were
placed so GNU ld coalesced them into ONE PT_LOAD segment spanning
0x0100A100-0x0100A284 with the gaps ZERO-FILLED. A debugger flashes by program
header, so it wrote 0x00000000 into the gap words - including PBPS (0x0100A1E0),
the one-time Permanent Block Protect - permanently locking the flash.

Having OFS in an image is FINE and expected. What is lethal is a LOAD segment
that SPANS more than one option word (i.e. covers gaps that get zero-filled and
programmed). FSP's fsp_gen.ld avoids this by giving each word its own MEMORY
region -> a separate, tiny PT_LOAD segment. This guard enforces the same:

  FAIL if any PT_LOAD segment overlapping 0x0100A100-0x0100A2CF has FileSiz > 12
  bytes (0xC = the largest single option word, BPS/PBPS). A spanning/gap-filled
  segment (e.g. the 0x184 one that bricked the boards) is > 12 and fails.

Usage:
    python check_ofs.py [IMAGE_ELF ...]   (defaults to the build_ra6m4_boot images)
Exit: 0 = safe, 1 = a spanning OFS segment found (DO NOT FLASH), 2 = tool error.
"""
import os, re, shutil, subprocess, sys

OFS_START, OFS_END = 0x0100A100, 0x0100A2D0     # config region (exclusive end)
MAX_WORD          = 0x0C                          # largest single option word (BPS/PBPS = 12B)
READELF           = os.environ.get("READELF", "arm-none-eabi-readelf")

def default_images():
    here = os.path.dirname(os.path.abspath(__file__))
    binp = os.path.normpath(os.path.join(here, "..", "..", "trusted-firmware-m",
                                          "build_ra6m4_boot", "bin"))
    return [os.path.join(binp, n) for n in ("bl2.elf", "tfm_s.axf", "tfm_ns.axf")
            if os.path.isfile(os.path.join(binp, n))]

def load_segments(elf):
    """Return list of (vaddr, filesz) for PT_LOAD segments overlapping the config region."""
    try:
        out = subprocess.run([READELF, "-l", elf], capture_output=True, text=True,
                             check=True).stdout
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        sys.exit(f"ERROR: readelf failed on {elf}: {e}")
    segs = []
    for m in re.finditer(r"LOAD\s+0x[0-9a-fA-F]+\s+0x([0-9a-fA-F]+)\s+0x[0-9a-fA-F]+\s+0x([0-9a-fA-F]+)", out):
        vaddr, filesz = int(m.group(1), 16), int(m.group(2), 16)
        if filesz and vaddr < OFS_END and (vaddr + filesz) > OFS_START:
            segs.append((vaddr, filesz))
    return segs

def main():
    if not shutil.which(READELF):
        sys.exit(f"ERROR: readelf '{READELF}' not on PATH (set $READELF)")
    images = sys.argv[1:] or default_images()
    if not images:
        sys.exit("ERROR: no images given and no default build images found")

    print(f"OFS brick guard: no PT_LOAD segment across {hex(OFS_START)}-{hex(OFS_END-1)} may exceed "
          f"{MAX_WORD} bytes (a span => zero-filled gaps => PBPS=0 => brick, DESIGN.md 8.4)\n")
    bad = 0
    for elf in images:
        if not os.path.isfile(elf):
            print(f"  {os.path.basename(elf):<14} MISSING"); continue
        segs = load_segments(elf)
        spanning = [(v, s) for (v, s) in segs if s > MAX_WORD]
        if not segs:
            print(f"  {os.path.basename(elf):<14} CLEAN (no OFS segments)")
        elif spanning:
            bad += 1
            det = ", ".join(f"0x{v:08X}+{s}B" for v, s in spanning)
            print(f"  {os.path.basename(elf):<14} *** SPANNING OFS SEGMENT - DO NOT FLASH *** ({det})")
        else:
            det = ", ".join(f"0x{v:08X}({s}B)" for v, s in segs)
            print(f"  {os.path.basename(elf):<14} OK - {len(segs)} discrete OFS segment(s): {det}")

    print()
    if bad:
        print(f"FAIL: {bad} image(s) have a spanning OFS segment. GNU ld coalesced option words into one")
        print("gap-filled PT_LOAD; flashing it programs PBPS=0 -> permanent brick. Give each OFS word its")
        print("OWN MEMORY region (like FSP's fsp_gen.ld / ra6m4_bl2.ld). DESIGN.md 8.4.")
        return 1
    print("PASS: OFS (if any) is in discrete per-word segments - safe to flash.")
    return 0

if __name__ == "__main__":
    sys.exit(main())
