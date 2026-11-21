# EEPROM layout and calibration redundancy

Source: `firmware/source/service.c`, `eeprom.c`, and `gui/xtools_isl6_gui.py`.

The serial EEPROM uses **64 words of 16 bits** (`00` through `3F`). The source recognizes the following fields inside the first 16 words:

| Meaning | Primary address | Inverted backup 1 | Inverted backup 2 |
|---|---|---|---|
| CAL1 | `01` | `09` | `0E` |
| CAL2 | `03` | `06` | `0B` |
| CAL3 | `04` | `07` | `0C` |
| CAL4 | `05` | `08` | `0D` |

Each inverted backup is expected to equal `primary XOR FFFF`. The GUI displays `OK` only when both backups match. Its calibration delta editor updates all three words for a chosen calibration value.

| Address | Observed label |
|---|---|
| `0A` | Serial number (16-bit value) |
| `00`, `02`, `0F` | Unknown / not decoded in supplied code |
| `10`–`3F` | Raw words; purpose not decoded in supplied code |

The GUI may save a text dump with lines such as `01 1234`. Use `DUMP EEPROM` to read all 64 words, save a complete reference, and check the charger identity before restoring. A dump may include a personal device serial number; do not upload private backups to a public repository.

**WARNING:** `RESTORE 00–0F`, `RESTORE 00–3F`, `WRITE` and `ERASE EEPROM` modify real EEPROM words. Full erase writes `FFFF` to every address and should be performed only with a verified backup. The original charger's interpretation of unknown words is not fully documented.
