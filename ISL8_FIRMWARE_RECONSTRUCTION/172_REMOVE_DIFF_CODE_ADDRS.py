def load_diff(filename):
    diff = set()
    with open(filename, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            addr = line.split("/")[0].strip()
            diff.add(int(addr, 16))
    return diff


def load_user_ranges(filename):
    addrs = set()
    with open(filename, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue

            try:
                part = line.split("/")[0].strip()
                start, end = part.split("-")

                start = int(start, 16)
                end = int(end, 16)

                for addr in range(start, end + 1):
                    addrs.add(addr)
            except:
                pass

    return addrs


def compress_ranges(addresses):
    ranges = []
    sorted_addrs = sorted(addresses)

    if not sorted_addrs:
        return ranges

    start = prev = sorted_addrs[0]

    for addr in sorted_addrs[1:]:
        if addr == prev + 1:
            prev = addr
        else:
            ranges.append((start, prev))
            start = prev = addr

    ranges.append((start, prev))
    return ranges


def main():
    user_file = "FINAL_USER_CODE.txt"
    diff_file = "DIFF_CODE.txt"
    out_file = "FINAL_USER_CODE_CLEAN.txt"

    user_addrs = load_user_ranges(user_file)
    diff_addrs = load_diff(diff_file)

    cleaned = user_addrs - diff_addrs

    ranges = compress_ranges(cleaned)

    with open(out_file, "w", encoding="utf-8") as f:
        for start, end in ranges:
            size = end - start + 1
            f.write(f"0x{start:04X}-0x{end:04X} / {size}b\n")

    print(f"Hotovo.")
    print(f"Původní: {len(user_addrs)} adres")
    print(f"Odebráno: {len(diff_addrs)} adres")
    print(f"Zůstalo: {len(cleaned)} adres")


if __name__ == "__main__":
    main()