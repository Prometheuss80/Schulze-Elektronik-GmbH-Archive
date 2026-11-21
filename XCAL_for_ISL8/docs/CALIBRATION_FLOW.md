# Factory-calibration handshake and reference selection

In the captured trace, `Bereit` starts the original firmware's calibration process; **Schulze controls the sequence**, Pico reacts to serial commands. The baud rate configured in `main.py` is **9600**.

```text
Schulze ISL8 factory calibration     Raspberry Pi Pico XCAL       External reference selector
             |                                 |                             |
             |  (5) Start CAL                  |                             |
             |  (5) SetUref: 0                 |                             |
             |-------------------------------->|  GPIO12 selects 12.600 V   |
             |                                 |---------------------------->|
             |  (8) SetUref:10                 |  GPIO15 selects 3.000 V    |
             |-------------------------------->|---------------------------->|
             |  (18) SetUref: 8                |  GPIO14 selects 4.200 V    |
             |-------------------------------->|---------------------------->|
             |  (23) SetUref: 4                |  GPIO13 selects 8.400 V    |
             |-------------------------------->|---------------------------->|
             |  (29) SetUref: 0                |  GPIO12 selects 12.600 V   |
             |-------------------------------->|---------------------------->|
             |  (35) Rtol/Offs Berechnung       |                             |
             |  (CAL=  0) fertig               |                             |
             |  (35) SetUref:10                |  GPIO15 selects 3.000 V    |
             |  (41) SetUref: 8                |  GPIO14 selects 4.200 V    |
             |  (47) SetUref: 4                |  GPIO13 selects 8.400 V    |
             |  (53) SetUref: 0                |  GPIO12 selects 12.600 V   |
             |  (59) SetUref: 0                |                             |
             |  (CHK=  0) fertig               |  set idle                   |
             |-------------------------------->|---------------------------->|
```

The log shows intermediate `setze_offs` / `setze_rtol` messages and final coefficients. `main.py` does **not** compute those coefficients or transmit them to the charger. It receives `SetUref` and switches hardware output selects; the original Schulze firmware performs the calculations.

> The serial log demonstrates one captured successful completion. It does not establish general electrical accuracy or guaranteed results on every ISL8 hardware revision.
