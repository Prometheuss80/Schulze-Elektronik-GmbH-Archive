#!/usr/bin/env python3
import time
from pathlib import Path

INPUT_LOG = Path("LOG_ISL8_ALL.txt")
OUTPUT_LOG = Path("LOG_ISL8_ALL_DEDUP.txt")

PROGRESS_INTERVAL = 500_000   # každého půl milionu řádků vypíše progress


def count_lines(path):
    print("[i] Počítám řádky (FAST MODE)...")
    t0 = time.time()

    n = 0
    with open(path, "rb") as f:
        for _ in f:
            n += 1

    print(f"[i] Celkem řádků: {n:,} (počítání trvalo {time.time()-t0:.1f}s)")
    return n


def main():
    total_lines = count_lines(INPUT_LOG)

    print("[i] Spouštím DEDUP...")
    t0 = time.time()
    last_print = t0

    last_line = None
    out_count = 0

    with open(INPUT_LOG, "r", encoding="ascii", errors="replace") as fin, \
         open(OUTPUT_LOG, "w", encoding="ascii") as fout:

        for i, line in enumerate(fin, 1):

            # ---- progress ----
            now = time.time()
            if now - last_print >= 1:
                elapsed = now - t0
                speed = i / elapsed
                eta = int((total_lines - i) / speed) if speed > 0 else 0
                pct = (i / total_lines) * 100

                print(
                    f"\r[DEDUP] {pct:6.2f}% | {i:,}/{total_lines:,} | "
                    f"{int(speed):8d} ř/s | ETA {eta//60:02d}:{eta%60:02d} | "
                    f"výstupních řádků {out_count:,}",
                    end=""
                )
                last_print = now

            # ---- dedup hlavní logika ----
            if line != last_line:
                fout.write(line)
                out_count += 1

            last_line = line

    print()
    print("[OK] DEDUP hotovo")
    print(f"[OK] Výsledný počet řádků: {out_count:,}")
    print(f"[OK] Výstup: {OUTPUT_LOG}")
    print(f"[OK] Celkový čas: {time.time()-t0:.1f}s")


if __name__ == "__main__":
    main()
