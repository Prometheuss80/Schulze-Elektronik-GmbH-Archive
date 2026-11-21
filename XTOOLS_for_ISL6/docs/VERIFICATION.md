# Source and release verification

This package was prepared from `XTOOLS_ISL6.rar`, supplied by the project author. All **25 original files** were copied without modification, preserving their original basenames.

- Firmware source files: Keil C and A51, with the original `main.c`, `service.c`, `eeprom.c`, `serial_cmd.c` etc.
- Firmware HEX: `xtools_isl6.hex` (Intel HEX validation: checksums pass, EOF record present; 6,430 programmed byte addresses, `0000–191D`)
- GUI: `xtools_isl6_gui.py` (Tkinter + `pyserial`; GUI source label **V0.9 UNIVERSAL**)
- Firmware string literal: **V0.8 UNIVERSAL**

**Verification limits:** This packaging step checks contents and HEX record integrity; it does **not** rebuild the HEX in Keil, execute hardware tests, or exercise the GUI against a live charger. The project author reports a working ISL6 toolchain; release-specific hardware tests should be recorded separately for each supported charger family.

No proprietary original Schulze firmware images are included. The supplied `xtools_isl6.hex` is the service firmware artifact; compare with your own hardware before programming.
