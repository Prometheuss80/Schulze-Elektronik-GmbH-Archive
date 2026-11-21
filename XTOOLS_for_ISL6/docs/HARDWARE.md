# Hardware profiles

Source: `firmware/source/hw_profile.c`, `eeprom.c`, `lcd.c`, `main.c`.

## CPU and EEPROM

The firmware targets the **Siemens 80C535** family and a **93C46/93LC46B EEPROM arranged as 64 16-bit words**.

| Feature | MODE 1 – ISL6-330D | MODE 2 – ISL6-430/530/636 family |
|---|---|---|
| LCD | 1 × 16 character | 2 × 16 character |
| EEPROM CS | P5.7 | P5.4 |
| EEPROM CLK | P5.6 | P5.5 |
| EEPROM DI | P5.5 | P5.6 (shared) |
| EEPROM DO | P5.4 | P5.6 (shared) |
| Key electrically detected | P1.7 | P1.6 |

The physical left key is wired differently in the two families. The startup selection prompts `PRESS LEFT KEY` and chooses the appropriate mode **before calling any EEPROM access routine**.

The 330D 1×16 LCD uses two eight-character halves in `lcd.c`; the larger-family LCD uses two conventional 16-character rows. The firmware uses a write-only LCD driver and a watchdog service routine.

**Hardware note:** These are the mappings implemented in the supplied project source. Verify component revision and wiring on your specific charger before installing custom firmware.
