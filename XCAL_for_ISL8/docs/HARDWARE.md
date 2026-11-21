# Hardware and firmware connections

## Microcontroller and display

| Function | Pico pin in `main.py` | Implementation |
|---|---|---|
| UART0 TX/RX | GP0 / GP1 | 9600 baud, **3.3 V UART**. Requires proper level translation to RS-232. |
| I²C0 SDA/SCL | GP16 / GP17 | 400 kHz; both SSD1306 and ADS1115 share the bus. |
| ADS1115 address | 0x48 | AIN0 for monitored reference voltage. |
| OLED | I²C | 128 × 64 SSD1306. |
| 3.000 V selection | GP15 | `led1` output; external circuit determines selected voltage. |
| 4.200 V selection | GP14 | `led2` output. |
| 8.400 V selection | GP13 | `led3` output. |
| 12.600 V selection | GP12 | `led4` output. |
| ADC ground-calibration switch | GP6 | `relay_gnd`. |
| ADC reference-calibration switch | GP7 | `relay_ref`. |
| PLUS / RESET / MINUS buttons | GP9 / GP10 / GP11 | Pull-down inputs. |
| Built-in status LED | GP25 | Startup, command and error indication. |

### Reference-voltage selection versus relay hardware

The software labels GP15–GP12 outputs `led1..led4`, but these are also the **four reference selection signals** in `update_leds()`. The code alone does not identify the driver circuit, relay ratings, signal polarity beyond active-high selection or analog reference topology. Verify these against the prototype wiring before powering a charger. **Never connect a 12.600 V reference directly to a Pico pin.**

## ADC signal path

The firmware defines `R_TOP = 20000` Ω and `R_BOTTOM = 2200` Ω, giving a nominal divider ratio of 10.0909. The ADS1115 input must remain within its allowed input/common-mode range. For startup self-calibration, `main.py` asserts GP6 for a ground measurement, then GP7 for a reference measurement and assumes a **4.205 V** source (`gain = (4.205 / DIVIDER_RATIO) / delta`). The 4.205 V figure is an internal code assumption, **not** the same thing as proving the switched 4.200 V output to 1 mV.

ADS1115 config uses `0x00E0` for 860 SPS; an adjacent driver comment says 128 SPS but the register value corresponds to 860 SPS. The original driver is retained unchanged for archive fidelity.

## Manual/automatic modes

- At startup: automatic mode, no reference selected.
- In AUTO mode: serial lines with `SetUref:` select a reference; a line containing both `(CHK=` and `fertig` returns to idle.
- PLUS/MINUS: switches to MANUAL and advances/reverses the selection in the order 3.000, 4.200, 8.400, 12.600 V.
- RESET button: returns to AUTO and turns off all reference-selection signals.

Do not assume that turning off the selection signals necessarily disconnects all analog outputs — this depends on the external circuit.

## Files needed on Pico

Copy `firmware/main.py`, `firmware/ads1115.py`, `assets/xcal.pbm` (as `xcal.pbm`) plus a compatible MicroPython `ssd1306.py` driver into the Pico filesystem root. The startup call to `show_schulze()` also tries to read optional `schulze.pbm`; if missing, the program displays an error logo screen and then continues. The third-party logo is not included in the public package.

The provided `test_i2c.py` and `test_ads1115.py` can be run separately. Do not run both diagnostic scripts and `main.py` concurrently.
