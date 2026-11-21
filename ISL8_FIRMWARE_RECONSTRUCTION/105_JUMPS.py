#!/usr/bin/env python3
from pathlib import Path
import re

ASM_FILE = Path("Reconstruction.a51")
OUT = Path("labels_jumps.txt")

# A_XXXX:
LABEL_PREFIX_RE = re.compile(r'^\s*A_[0-9A-Fa-f]{4}:\s*')

# A_XXXX kdekoliv v instrukci
A_REF_RE = re.compile(r'\bA_([0-9A-Fa-f]{4})\b')


def main():
    found = set()

    with ASM_FILE.open("r", encoding="ascii", errors="ignore") as f:
        for line in f:
            # odstraníme komentář
            code = line.split("//", 1)[0]

            # odstraníme případný LABEL na začátku řádku
            code = LABEL_PREFIX_RE.sub("", code)

            # teď teprve hledáme A_xxxx v instrukci
            for m in A_REF_RE.findall(code):
                found.add(int(m, 16))

    with OUT.open("w", encoding="ascii") as f:
        for a in sorted(found):
            f.write(f"0x{a:04X}\n")

    print("[✓] Jump/Call labels collected")
    print(f"    count: {len(found)}")
    print(f"[+] Output: {OUT.name}")


if __name__ == "__main__":
    main()
