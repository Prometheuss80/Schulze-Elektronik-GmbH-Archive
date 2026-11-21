# Schulze ISL8-936G — Factory Calibration F2 Patcher

**Release 1.2 — hardware tested on all four supported original firmware images.**

**Author / maintainer:** Martin Stránský ([Prometheuss80](https://github.com/Prometheuss80)).

This Python utility creates a locally patched firmware image from a **user-supplied original 64 KiB Schulze ISL8-936G ROM**. It makes the existing factory calibration entry accessible with **F2**; it does **not** implement or replace Schulze's calibration algorithms.

The modified firmware is identified by its display label (`8.19` or `8.59`) and by edited date/time **display strings** (`22.04.26`, `18:30`). These are **custom patch identifiers, not official Schulze versions or release dates**. The patch does not set the RTC.

## Supported originals (all tested on physical ISL8-936G hardware)

| Original | Modified LCD label | Output file | Calibration entry | Original `LCALL` target | Hardware |
|---|---|---|---|---|---|
| 8.10 DE | 8.19 | `fw819de.bin` | `B9B5h` | `3BE1h` | PASS |
| 8.10 EN | 8.19 | `fw819en.bin` | `B9A2h` | `3BDEh` | PASS |
| 8.50 DE | 8.59 | `fw859de.bin` | `BABAh` | `3BD8h` | PASS |
| 8.50 EN | 8.59 | `fw859en.bin` | `BAA7h` | `3BD5h` | PASS |

Only these four **exact original SHA-256 hashes** are accepted. Even if a different version contains a similar instruction sequence, this conservative release will **refuse** to patch it. This is intentional.

## Quick start

Requires **Python 3.9+**, no third-party dependencies.

1. Obtain and back up your own **unmodified** 65,536-byte original firmware image.
2. Place it beside `patch_factory_calibration.py` as `original.bin`.
3. Preview the patch:

   ```sh
   python patch_factory_calibration.py original.bin --dry-run
   ```

4. Generate the output:

   ```sh
   python patch_factory_calibration.py original.bin
   ```

The output name is selected automatically from the detected original (`fw819de.bin`, `fw819en.bin`, `fw859de.bin`, or `fw859en.bin`). The input is **never overwritten**, and an existing output filename is **never replaced**.

Optional commands:

```sh
python patch_factory_calibration.py path/to/another_original.bin
python patch_factory_calibration.py original.bin -o custom_output.bin
python -m unittest discover -v
```

## What the patch changes

1. **Calibration entry, 35 bytes:** Changes the triggering button check from the original bit `22h` to **F2 (bit `14h`)**, establishes the required internal RAM state, preserves the per-version absolute `LCALL` target, and retains the original routine following the patched region.
2. **LCD version text, one byte:** `8.10` → `8.19` or `8.50` → `8.59` (`30h` to `39h` in the text field).
3. **Date text, two occurrences:** `15.06.04` → `22.04.26`.
4. **Time text, two occurrences:** `we 12:30` → `we 18:30` in English ROMs; `Mi 12:30` → `Mi 18:30` in German ROMs. The weekday text and zero terminators are preserved.

Exactly **41 bytes change** in each of the four tested images (28 calibration bytes, 1 version byte, 10 date bytes, 2 time bytes). ROM size and all other bytes are identical to the original.

**More detail:** [Technical explanation](docs/PATCH_TECHNICAL.md) · [Four version-specific HEX change tables](docs/HEX_CHANGES.md) · [Testing, hashes and verification](docs/TESTING.md)

## Safety and scope

**Factory calibration is a service function.** Using it incorrectly can alter calibration values and make the charger unsafe or inaccurate. Preserve the original ROM and any available calibration/Timekeeper backup; test on the intended charger before relying on it. This program performs no flashing or physical device operations.

The tool verifies image size, original SHA-256, original instruction pattern and its unique offset, branch/`LCALL` targets, both date/time occurrences, the LCD version text, and that changes remain within the explicitly authorized byte ranges. Unexpected images are rejected.

The original and modified Schulze firmware ROM images **are not distributed** here. You must provide your own legally obtained firmware image. This repository contains only the patching tool, tests and documentation.

## Documentation origin

The technical documentation incorporates original 8051/A51 investigation notes for the **8.50 EN** and **8.50 DE** versions, including original and modified assembly listings. Their address labels differ between versions; see the [technical explanation](docs/PATCH_TECHNICAL.md). All four variants have been independently verified against the provided reference binary images and tested by the project maintainer on the actual charger.

## Status

**V1.2 FINAL — four versions hardware tested.** Unknown original versions are unsupported by this release.

## Author and license

**Martin Stránský (Prometheuss80)** — development, reverse engineering, firmware analysis and hardware verification.

Copyright © 2026 Martin Stránský (Prometheuss80).

The original Python patching utility, tests, and project-authored documentation are released under the **[MIT License](LICENSE)**. You may use, modify, and redistribute this project under its terms while retaining the copyright and license notice.

This is an independent community preservation/service project and is **not affiliated with or endorsed by Schulze Elektronik**. The license **does not cover** original Schulze firmware, ROM images or other third-party material. No original or patched firmware image is included here.
