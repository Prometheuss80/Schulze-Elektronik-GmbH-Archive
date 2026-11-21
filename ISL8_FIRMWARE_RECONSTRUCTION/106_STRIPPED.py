#!/usr/bin/env python3
from pathlib import Path
import re
import os

ASM_FILE = Path("Reconstruction.a51")
TMP_FILE = Path("Reconstruction.tmp")

LABELS_CODE  = Path("labels_code.txt")
LABELS_DATA  = Path("labels_data.txt")
LABELS_JUMPS = Path("labels_jumps.txt")
LABELS_CMD   = Path("FINAL_USER_LABELS_CMD.txt")   # <<< NOVÉ


def load_allowed_labels():
    allowed = set()

    for path in (LABELS_CODE, LABELS_DATA, LABELS_JUMPS, LABELS_CMD):
        if not path.exists():
            continue
        with path.open("r", encoding="ascii") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                addr = int(line, 16)
                allowed.add(f"A_{addr:04X}")

    return allowed


# zachytí celý "LABEL:    "
LABEL_RE = re.compile(r'^([A-Za-z_][A-Za-z0-9_]*:)(\s*)')


def main():
    allowed = load_allowed_labels()

    before_org0 = True
    removed = kept = 0

    with ASM_FILE.open("r", encoding="ascii", errors="ignore") as fin, \
         TMP_FILE.open("w", encoding="ascii") as fout:

        for line in fin:

            # --- dokud nenarazíme na org 0x0000, nic neměníme ---
            if before_org0:
                fout.write(line)
                if re.match(r'\s*org\s+0x0000\b', line, re.IGNORECASE):
                    before_org0 = False
                continue

            m = LABEL_RE.match(line)
            if not m:
                fout.write(line)
                continue

            label_with_colon = m.group(1)   # "A_0003:"
            label = label_with_colon[:-1]   # bez dvojtečky

            if label in allowed:
                kept += 1
                fout.write(line)
            else:
                removed += 1
                span_len = len(m.group(0))
                fout.write(" " * span_len + line[m.end():])

    # 🔁 atomické přepsání původního souboru
    os.replace(TMP_FILE, ASM_FILE)

    print("[✓] Safe label stripping done (in-place)")
    print(f"    kept:    {kept}")
    print(f"    removed: {removed}")
    print("[+] File overwritten:", ASM_FILE.name)


if __name__ == "__main__":
    main()