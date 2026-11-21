#!/usr/bin/env python3
from pathlib import Path
import re

ASM_FILE = Path("Reconstruction.a51")

OUT_MOVC = Path("dptr_movc.txt")
OUT_MOVX = Path("dptr_movx.txt")

LABEL_RE = re.compile(r'^A_([0-9A-Fa-f]{4}):')
DPTR_SET_RE = re.compile(r'MOV\s+DPTR,#0x([0-9A-Fa-f]{1,4})')
MOVC_RE = re.compile(r'\bMOVC\b')
MOVX_RE = re.compile(r'\bMOVX\b')


def sort_key(entry):
    dptr = entry["dptr"]
    if dptr is None:
        return 0xFFFF + 1
    return dptr


def fmt_dptr(dptr):
    if dptr is None:
        return "????"
    return f"{dptr:04X}"


def main():
    current_code_addr = "????"
    current_dptr = None

    movc_hits = []
    movx_hits = []

    with ASM_FILE.open("r", encoding="ascii", errors="ignore") as f:
        for line in f:
            line_strip = line.strip()

            # label adresa
            m = LABEL_RE.match(line_strip)
            if m:
                current_code_addr = m.group(1)

            # DPTR load
            m = DPTR_SET_RE.search(line)
            if m:
                current_dptr = int(m.group(1), 16)

            # MOVC
            if MOVC_RE.search(line):
                movc_hits.append({
                    "dptr": current_dptr,
                    "text": f"{current_code_addr}  DPTR={fmt_dptr(current_dptr)}  MOVC   | {line_strip}"
                })

            # MOVX
            if MOVX_RE.search(line):
                movx_hits.append({
                    "dptr": current_dptr,
                    "text": f"{current_code_addr}  DPTR={fmt_dptr(current_dptr)}  MOVX   | {line_strip}"
                })

    # sort
    movc_hits.sort(key=sort_key)
    movx_hits.sort(key=sort_key)

    with OUT_MOVC.open("w", encoding="ascii") as f:
        f.write("=== MOVC (ROM reads) sorted by DPTR ===\n\n")
        for e in movc_hits:
            f.write(e["text"] + "\n")

    with OUT_MOVX.open("w", encoding="ascii") as f:
        f.write("=== MOVX (XRAM/IO) sorted by DPTR ===\n\n")
        for e in movx_hits:
            f.write(e["text"] + "\n")

    print("[✓] Hotovo — seřazeno podle DPTR")
    print(f"    MOVC: {len(movc_hits)}")
    print(f"    MOVX: {len(movx_hits)}")
    print(f"[+] Výstup: {OUT_MOVC.name}, {OUT_MOVX.name}")


if __name__ == "__main__":
    main()
