#!/usr/bin/env python3
from pathlib import Path

TEMP_BLOCKS      = Path("TEMP_BLOCKS.txt")
TIME_CODE_FILE   = Path("TIME_CODE_ADDRS.txt")
TIME_DATA_FILE   = Path("TIME_DATA_ADDRS.txt")
USER_CODE_FILE   = Path("FINAL_USER_CODE.txt")
USER_DATA_FILE   = Path("FINAL_USER_DATA.txt")

OUT_ALL      = Path("labels_all.txt")
OUT_CODE     = Path("labels_code.txt")
OUT_DATA     = Path("labels_data.txt")
OUT_UNKNOWN  = Path("labels_unknown.txt")


def load_addr_set(path):
    s = set()
    with path.open("r", encoding="ascii", errors="ignore") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                s.add(int(line.split("/", 1)[0], 16))
            except ValueError:
                pass
    return s


def load_user_code_ranges(path):
    ranges = []
    with path.open("r", encoding="ascii", errors="ignore") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            part = line.split("/", 1)[0]
            start, end = part.split("-", 1)
            ranges.append((int(start, 16), int(end, 16)))
    return ranges


def in_ranges(addr, ranges):
    for s, e in ranges:
        if s <= addr <= e:
            return True
    return False


def main():
    print("[*] Loading priority sources")

    time_code = load_addr_set(TIME_CODE_FILE)
    time_data = load_addr_set(TIME_DATA_FILE)
    user_code = load_user_code_ranges(USER_CODE_FILE)
    user_data = load_addr_set(USER_DATA_FILE)

    labels_all = set()
    labels_code = set()
    labels_data = set()
    labels_unknown = set()

    print("[*] Processing TEMP_BLOCKS")

    with TEMP_BLOCKS.open("r", encoding="ascii", errors="ignore") as f:
        for line in f:
            line = line.strip()
            if not line or "-" not in line:
                continue

            start_str = line.split("-", 1)[0]
            addr = int(start_str, 16)
            labels_all.add(addr)

            # --- PRIORITNÍ ROZHODOVÁNÍ ---
            if addr in time_code:
                labels_code.add(addr)
            elif addr in time_data:
                labels_data.add(addr)
            elif in_ranges(addr, user_code):
                labels_code.add(addr)
            elif addr in user_data:
                labels_data.add(addr)
            else:
                labels_unknown.add(addr)

    def save_hex(path, values):
        with path.open("w", encoding="ascii") as f:
            for v in sorted(values):
                f.write(f"0x{v:04X}\n")

    save_hex(OUT_ALL, labels_all)
    save_hex(OUT_CODE, labels_code)
    save_hex(OUT_DATA, labels_data)
    save_hex(OUT_UNKNOWN, labels_unknown)

    print("[✓] Done")
    print(f"    ALL:     {len(labels_all)}")
    print(f"    CODE:    {len(labels_code)}")
    print(f"    DATA:    {len(labels_data)}")
    print(f"    UNKNOWN: {len(labels_unknown)}")


if __name__ == "__main__":
    main()
