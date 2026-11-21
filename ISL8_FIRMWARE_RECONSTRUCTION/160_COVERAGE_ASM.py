import re

FILE = "FINAL_COVERAGE_ASM.txt"
ROM_SIZE = 65536

line_count = 0
total_bytes = 0

with open(FILE, "r", encoding="utf-8") as f:
    for line in f:
        line = line.strip()

        # přeskočit prázdné řádky
        if not line:
            continue

        # přeskočit komentáře
        if line.startswith("//"):
            continue

        # přeskočit neznámé bloky
        if "???" in line:
            continue

        m = re.match(r'([0-9A-Fa-f]{4})-([0-9A-Fa-f]{4})', line)
        if not m:
            continue

        start = int(m.group(1), 16)
        end = int(m.group(2), 16)

        size = end - start + 1

        line_count += 1
        total_bytes += size

percent = (total_bytes / ROM_SIZE) * 100

print("ROM size:", ROM_SIZE)
print("Blocks:", line_count)
print("Bytes covered:", total_bytes)
print("Covered: %.2f kB" % (total_bytes/1024))
print("Coverage: %.2f %% " % percent)