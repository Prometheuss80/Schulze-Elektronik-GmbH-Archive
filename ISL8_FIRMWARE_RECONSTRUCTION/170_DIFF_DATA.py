def load_addresses(filename):
    addrs = set()
    with open(filename, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue

            # očekává formát: 0x1234 / ...
            try:
                addr = line.split("/")[0].strip()
                addrs.add(addr.upper())
            except:
                pass

    return addrs


def main():
    file1 = "TIME_DATA_ADDRS.txt"
    file2 = "FINAL_USER_DATA.txt"
    out_file = "DIFF_DATA.txt"

    set1 = load_addresses(file1)
    set2 = load_addresses(file2)

    common = sorted(set1 & set2)

    with open(out_file, "w", encoding="utf-8") as f:
        for addr in common:
            f.write(f"{addr} /\n")

    print(f"Hotovo. Nalezeno {len(common)} shodných adres.")


if __name__ == "__main__":
    main()