#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from pathlib import Path

# ============================================================
# KONFIGURACE
# ============================================================

A51_FILE = Path("Reconstruction.a51")     # vstupní ASM (bude přepsán!)
BIN_FILE = Path("fw850en.bin")             # originální firmware

TIME_CODE_FILE       = Path("TIME_CODE_ADDRS.txt")    # 100% CODE (time-trace)
TIME_DATA_FILE       = Path("TIME_DATA_ADDRS.txt")    # 100% DATA (time-trace)
FINAL_USER_CODE_FILE = Path("FINAL_USER_CODE.txt")    # ručně potvrzený CODE
FINAL_USER_DATA_FILE = Path("FINAL_USER_DATA.txt")    # ručně potvrzená DATA

ROM_SIZE = 64 * 1024  # 64 kB


# ============================================================
# HELPERS
# ============================================================

def load_simple_addr_file(path: Path) -> set[int]:
    out = set()
    if not path.exists():
        return out

    for ln in path.read_text(encoding="utf-8").splitlines():
        s = ln.strip()
        if not s or s.startswith(";"):
            continue

        # --- ODSTRANIT KOMENTÁŘE ZA / ---
        if "/" in s:
            s = s.split("/", 1)[0].strip()

        # --- ROZSAH ---
        if "-" in s:
            try:
                left, right = s.split("-", 1)
                right = right.strip()
                a1 = int(left.replace("0x", ""), 16)
                a2 = int(right.replace("0x", ""), 16)
                for a in range(min(a1, a2), max(a1, a2) + 1):
                    out.add(a & 0xFFFF)
                continue
            except:
                pass

        # --- JEDNOTLIVÁ ADRESA ---
        try:
            out.add(int(s.replace("0x", ""), 16) & 0xFFFF)
        except:
            pass

    return out



def write_block(out_lines: list[str], start_addr: int, data: bytes) -> None:
    """Vloží blok ORG + DB dat."""
    out_lines.append("")
    out_lines.append(f"org  0x{start_addr:04X}")
    for i in range(0, len(data), 16):
        chunk = data[i:i + 16]
        hexes = ", ".join(f"0x{b:02X}" for b in chunk)
        out_lines.append(f"    DB {hexes}")


# ============================================================
# MAIN
# ============================================================

def main():
    print("[+] FILL: doplňuji nepoužitou paměť jako DB")

    for f in (
        A51_FILE, BIN_FILE,
        TIME_CODE_FILE, TIME_DATA_FILE,
        FINAL_USER_CODE_FILE, FINAL_USER_DATA_FILE
    ):
        if not f.exists():
            print(f"[!] Chybí soubor: {f}")

    # --- načtení firmware ---
    fw = bytearray(BIN_FILE.read_bytes())
    if len(fw) < ROM_SIZE:
        fw.extend(b"\x00" * (ROM_SIZE - len(fw)))

    # --- načtení adres ---
    time_code = load_simple_addr_file(TIME_CODE_FILE)
    time_data = load_simple_addr_file(TIME_DATA_FILE)
    user_code = load_simple_addr_file(FINAL_USER_CODE_FILE)
    user_data = load_simple_addr_file(FINAL_USER_DATA_FILE)

    confirmed_code = time_code | user_code
    confirmed_data = time_data | user_data

    used = confirmed_code | confirmed_data

    print(
        f"[i] TIME code: {len(time_code)} | "
        f"TIME data: {len(time_data)} | "
        f"USER code: {len(user_code)} | "
        f"USER data: {len(user_data)}"
    )
    print(f"[i] Celkem použitých adres: {len(used)}")

    # --- načtení ASM ---
    lines = A51_FILE.read_text(encoding="utf-8").splitlines()

    # najdi END
    end_idx = None
    for i, ln in enumerate(lines):
        if ln.strip().lower() == "end":
            end_idx = i
            break
    if end_idx is None:
        end_idx = len(lines)
        lines.append("end")

    # --- chybějící adresy ---
    all_addrs = set(range(ROM_SIZE))
    missing = sorted(all_addrs - used)

    print(f"[i] Nepoužitých adres (DB fill): {len(missing)}")

    blocks = []
    if missing:
        blocks += [
            "",
            "; ========================================================",
            ";  DOPLNĚNÁ NEPOUŽITÁ PAMĚŤ (AUTO DB FILL)",
            "; ========================================================",
        ]

        start = prev = missing[0]
        for a in missing[1:]:
            if a == prev + 1:
                prev = a
                continue
            write_block(blocks, start, fw[start:prev + 1])
            start = prev = a
        write_block(blocks, start, fw[start:prev + 1])

    final = lines[:end_idx] + blocks + ["", "end"]
    A51_FILE.write_text("\n".join(final), encoding="utf-8")

    print("[✓] FILL hotový")
    print(f"    Doplněno DB adres: {len(missing)}")


if __name__ == "__main__":
    main()
