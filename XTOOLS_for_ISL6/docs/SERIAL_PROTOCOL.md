# PC serial protocol

Source: `firmware/source/serial_cmd.c`, `uart.c`, `gui/xtools_isl6_gui.py`.

**UART:** 9600 baud, 8 data bits, no parity, 1 stop bit (8N1). Commands are ASCII, case-insensitive in the firmware, terminated with carriage return.

| Command | Function |
|---|---|
| `PING` | Test serial link |
| `INFO` | Report XTOOLS version, selected hardware mode, serial number and commands |
| `READ aa` | Read one 16-bit EEPROM word, hexadecimal address `00–3F` |
| `DUMP` | Dump the complete EEPROM (`00–3F`) as address/value pairs |
| `WRITE aa xxxx` | Write one 16-bit EEPROM word and verify by readback |
| `RESET` | Request a watchdog reset of the charger |

Examples of message format:

```text
READ 0A
READ 0x0A = 0x1234 / 4660
```

The numeric values above are illustrative, not values from a real EEPROM.

The firmware opens a **30-second command window**, refreshed after each recognized command. When it times out, the normal service LCD overview begins. The GUI sends `PING` and `INFO` after connecting and `RESET` on disconnect. WRITE and DUMP can produce substantial serial output; allow requests to finish before disconnecting.

**Write behavior:** the current implementation accepts the full `00–3F` address range. An old comment in `serial_cmd.c` still refers to an earlier safety-test restriction to `3F` only; this comment is outdated and the active `ParseWrite()` code allows `00–3F`.
