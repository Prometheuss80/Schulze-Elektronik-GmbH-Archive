#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
CHECK_FINAL.py

Najde všechny skokové instrukce v Reconstruction.a51
a vypíše ty, které míří do oblasti DATA
(TIME DATA + USER DATA).
"""

from pathlib import Path
import re

# ============================================================
# KONFIGURACE
# ============================================================

ASM_FILE = Path("Reconstruction.a51")

TIME_CODE_FILE = Path("TIME_CODE_ADDRS.txt")
TIME_DATA_FILE = Path("TIME_DATA_ADDRS.txt")

FINAL_USER_CODE_FILE = Path("FINAL_USER_CODE.txt")
FINAL_USER_DATA_FILE = Path("FINAL_USER_DATA.txt")

OUT_FILE = Path("CHECK_FINAL.txt")

# ============================================================
# REGEXY
# ============================================================

# Skokové instrukce + cílový label
JUMP_RE = re.compile(
    r'\b(?:ACALL|LCALL|AJMP|LJMP|SJMP|JZ|JNZ|JC|JNC)\s+A_([0-9A-Fa-f]{4})'
)

ADDR_RE = re.compile(r'^A_([0-9A-Fa-f]{4}):')

# ============================================================
# HELPERS
# ============================================================

def load_addr_file(path: Path) -> set[int]:
    out = set()
    if not path.exists():
        return out

    for ln in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        s = ln.strip()
        if not s or s.startswith(";"):
            continue

        if "-" in s:
            try:
                left, right = s.split("-", 1)
                right = right.split("/")[0]
                a1 = int(left.replace("0x", ""), 16)
                a2 = int(right.replace("0x", ""), 16)
                for a in range(min(a1, a2), max(a1, a2) + 1):
                    out.add(a & 0xFFFF)
                continue
            except:
                pass

        try:
            out.add(int(s.replace("0x", "").split()[0], 16) & 0xFFFF)
        except:
            pass

    return out


# ============================================================
# MAIN
# ============================================================

def main():
    print("[+] CHECK: hledám skoky do DATA")

    if not ASM_FILE.exists():
        raise FileNotFoundError("Reconstruction.a51 nenalezen")

    # --- načtení adres ---
    time_code = load_addr_file(TIME_CODE_FILE)
    time_data = load_addr_file(TIME_DATA_FILE)
    user_code = load_addr_file(FINAL_USER_CODE_FILE)
    user_data = load_addr_file(FINAL_USER_DATA_FILE)

    data_addrs = time_data | user_data
    code_addrs = time_code | user_code

    print(
        f"[i] TIME code: {len(time_code)} | "
        f"TIME data: {len(time_data)} | "
        f"USER code: {len(user_code)} | "
        f"USER data: {len(user_data)}"
    )

    lines = ASM_FILE.read_text(encoding="utf-8", errors="ignore").splitlines()

    current_addr = None
    findings = []

    for ln_no, line in enumerate(lines, 1):
        # zjisti aktuální adresu (návěstí)
        m_addr = ADDR_RE.match(line.strip())
        if m_addr:
            current_addr = int(m_addr.group(1), 16)

        # hledej skok
        m_jump = JUMP_RE.search(line)
        if not m_jump or current_addr is None:
            continue

        target = int(m_jump.group(1), 16)

        # zajímá nás pouze skok do DATA
        if target in data_addrs:
            findings.append(
                f"0x{current_addr:04X} -> 0x{target:04X}   | {line.strip()}"
            )

    # --- výstup ---
    with OUT_FILE.open("w", encoding="utf-8") as f:
        f.write("=== SKOKY DO DATA ===\n\n")
        if not findings:
            f.write("Nenalezen žádný skok do DATA.\n")
        else:
            for ln in findings:
                f.write(ln + "\n")

    print(f"[✓] Hotovo — nalezeno {len(findings)} problémových skoků")
    print(f"[✓] Výstup: {OUT_FILE}")


if __name__ == "__main__":
    main()
