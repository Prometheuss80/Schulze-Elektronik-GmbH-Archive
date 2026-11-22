# LCD wiring – Schulze ISL8-936G → SAP1024B

The mapping below follows the **confirmed user wiring on the tested replacement board**, not a generic T6963C module's pinout. Different commercial 240×64 modules may expose different connector numbering (especially contrast/negative voltage). Verify the silkscreen, the actual LCD datasheet, and continuity before connecting power.

## Functional signal map

| Signal on ISL8 | MCU pin | Original LM6061SYS function | Replacement SAP1024B function | Notes |
|---|---|---|---|---|
| D0 | P5.0 | DB0 | DB0 | direct |
| D1 | P5.1 | DB1 | DB1 | direct |
| D2 | P5.2 | DB2 | DB2 | direct |
| D3 | P5.3 | DB3 | DB3 | direct |
| D4 | P5.4 | DB4 | DB4 | direct |
| D5 | P5.5 | DB5 | DB5 | direct |
| D6 | P5.6 | DB6 | DB6 | direct |
| D7 | P5.7 | DB7 | DB7 | direct |
| E | P6.0 | E | E / write strobe | handled by patched driver |
| R/W | P6.1 | R/W | R/W / read control | handled by patched driver |
| RS | P6.2 | RS | C/D (command/data) | **different interpretation** |
| CS | P6.3 | CS | CS | handled by patched driver |
| RESET | P6.4 | RESET | RESET | handled by patched driver |

`P6.5` and `P6.6` are main-board latch controls; `P6.7` is a fuse input. **Do not repurpose these pins for the LCD.**

## 20-pin display connection – tested board's numbering

| Pin | Original LM6061SYS | Replacement PG24064E-PD / tested board | Action |
|---|---|---|---|
| 1 | GND | GND | Common ground |
| 2 | +5 V | VCC | Check supply polarity |
| 3 | Contrast | Contrast | **Verify contrast voltage requirements; don't assume direct compatibility** |
| 4 | RS | RS / C-D | P6.2 |
| 5 | R/W | R/W | P6.1 |
| 6 | E | E | P6.0 |
| 7 | DB0 | DB0 | P5.0 |
| 8 | DB1 | DB1 | P5.1 |
| 9 | DB2 | DB2 | P5.2 |
| 10 | DB3 | DB3 | P5.3 |
| 11 | DB4 | DB4 | P5.4 |
| 12 | DB5 | DB5 | P5.5 |
| 13 | DB6 | DB6 | P5.6 |
| 14 | DB7 | DB7 | P5.7 |
| 15 | CS | CS | P6.3 |
| 16 | RESET | RESET | P6.4 |
| 17 | VLCD | VLCD | **Check bias/contrast circuit, module-specific** |
| 18 | NC | NC | Leave open |
| 19 | NC on original board | Not connected on tested connector | **FS is a separate controller/module configuration; see below** |
| 20 | NC | NC | Leave open |

### FS width selection

Set **FS = LOW (GND)** on the **SAP1024B controller/module's actual FS connection**. For the tested board, pin 19 of the 20-pin LCD connector is not connected, so **do not assume FS is available on connector pin 19**. Locate the FS pad or trace using the particular board design. Always turn the charger off before altering wiring.

- FS LOW: 8 horizontal graphics pixels per memory byte, graphics area 30 bytes/line, 240×64 = **1,920 bytes**.
- FS HIGH: 6 pixels/byte; this specific driver **does not support** that mode.

### Important note about other modules

A generic 20-pin `T6963C`-compatible LCD **often uses a different standard pinout**, sometimes with `VEE` on pin 9 and `FS` on pin 19. Such a module **must not** be wired pin-for-pin according to the tested board's table. The connector pin numbers above describe the particular board used in this project, not every product sold under a similar name.
