#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
PROCESS_PLACEHOLDER_BLOCKS
- Parsuje Reconstruction.a51
- Najde DB-only bloky mimo TRACE
- Vytvoří INFO_PLACEHOLDER.txt
- Seřadí bloky od nejmenších po největší
"""

from pathlib import Path
import re

INPUT_FILE  = Path("Reconstruction.a51")
OUTPUT_FILE = Path("INFO_placeholder_FINAL.txt")

ORG_RE = re.compile(r'^\s*org\s+0x([0-9A-Fa-f]{4})', re.IGNORECASE)
DB_RE  = re.compile(r'\bDB\b', re.IGNORECASE)
HEX_RE = re.compile(r'0x([0-9A-Fa-f]{2})')

def main():
    if not INPUT_FILE.exists():
        print(f"[ERR] Nenalezen vstupní soubor: {INPUT_FILE}")
        return

    lines = INPUT_FILE.read_text(encoding="utf-8", errors="ignore").splitlines()

    blocks = []

    current_start = None
    current_size  = 0
    inside_block  = False

    for ln in lines:
        ln = ln.strip()

        # -------- org --------
        m = ORG_RE.match(ln)
        if m:
            # uzavřít předchozí blok
            if inside_block and current_start is not None and current_size > 0:
                end = current_start + current_size - 1
                blocks.append((current_start, end))

            current_start = int(m.group(1), 16)
            current_size  = 0
            inside_block  = True
            continue

        # -------- DB --------
        if inside_block and DB_RE.search(ln):
            bytes_found = HEX_RE.findall(ln)
            current_size += len(bytes_found)
            continue

        # -------- jiný řádek → ukončení bloku --------
        if inside_block:
            if current_start is not None and current_size > 0:
                end = current_start + current_size - 1
                blocks.append((current_start, end))
            inside_block = False
            current_start = None
            current_size  = 0

    # poslední blok
    if inside_block and current_start is not None and current_size > 0:
        end = current_start + current_size - 1
        blocks.append((current_start, end))

    # ---- bezpečnostní filtr ----
    blocks = [(s, e) for s, e in blocks if s is not None and e is not None and e >= s]

    # ---- seřazení podle velikosti (nejmenší první) ----
    blocks_sorted = sorted(blocks, key=lambda b: (b[1] - b[0] + 1, b[0]))

    # ---- zápis ----
    with OUTPUT_FILE.open("w", encoding="utf-8") as f:
        for s, e in blocks_sorted:
            size = e - s + 1
            f.write(f"0x{s:04X}-0x{e:04X}/{size}b\n")

    print("[OK] Placeholder bloky zpracovány")
    print(f"[OK] Počet bloků: {len(blocks_sorted)}")
    print(f"[OK] Výstup: {OUTPUT_FILE}")

if __name__ == "__main__":
    main()

