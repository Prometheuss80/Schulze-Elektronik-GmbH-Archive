def load_time_code(filename):
    addrs = set()
    with open(filename, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue

            # formát: 0000
            addrs.add(f"0x{line.upper()}")

    return addrs


def load_user_code(filename):
    addrs = set()
    with open(filename, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue

            # formát: 0x000A-0x000A / 1b
            try:
                part = line.split("/")[0].strip()
                start, end = part.split("-")

                start = int(start, 16)
                end = int(end, 16)

                for addr in range(start, end + 1):
                    addrs.add(f"0x{addr:04X}")

            except:
                pass

    return addrs


def main():
    file1 = "TIME_CODE_ADDRS.txt"
    file2 = "FINAL_USER_CODE.txt"
    out_file = "DIFF_CODE.txt"

    set1 = load_time_code(file1)
    set2 = load_user_code(file2)

    common = sorted(set1 & set2)

    with open(out_file, "w", encoding="utf-8") as f:
        for addr in common:
            f.write(f"{addr} /\n")

    print(f"Hotovo. Překryv: {len(common)} adres.")


if __name__ == "__main__":
    main()