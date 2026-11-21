#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from pathlib import Path

# ============================
# NASTAVENÍ
# ============================

RANGE = "B264-B276"          # <-- ZDE ZMĚŇ ROZSAH
OUTPUT_FILE = Path("data_fill.txt")

# ============================
# MAIN
# ============================

def main():
    start_s, end_s = RANGE.split("-", 1)
    start = int(start_s, 16)
    end   = int(end_s, 16)

    if start > end:
        start, end = end, start

    lines = []
    for addr in range(start, end + 1):
        lines.append(f"0x{addr:04X} /")

    OUTPUT_FILE.write_text("\n".join(lines), encoding="utf-8")

    print("[✓] Hotovo")
    print(f"    Rozsah: 0x{start:04X} - 0x{end:04X}")
    print(f"    Zapsáno: {len(lines)} adres")
    print(f"    Soubor:  {OUTPUT_FILE.name}")


if __name__ == "__main__":
    main()
