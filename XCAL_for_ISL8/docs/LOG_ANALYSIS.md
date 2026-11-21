# Captured factory-calibration log

The distributed log is derived from the project archive's `CALIB_LOG.log`, with the unique device number masked. Original measurements and sequence are unchanged.

## Selected events

| Serial output | Interpretation |
|---|---|
| `(5) Start CAL` | Original ISL8 calibration run begins. |
| `(5) SetUref: 0` | Select 12.600 V. |
| `(8) SetUref:10` | Select 3.000 V. |
| `setze_offs(   0,   0,   0)` | Charger resets or initializes offset values in this observed run. |
| `setze_rtol(   0,   0,   0,   0)` | Charger resets/initializes relative tolerance values in this run. |
| `(18) SetUref: 8` | Select 4.200 V. |
| `(23) SetUref: 4` | Select 8.400 V. |
| `(29) SetUref: 0` | Select 12.600 V. |
| `(35) Rtol/Offs Berechnung` | Charger calculates the calibration correction. |
| `setze_rtol(  -2,  -2,   2,   0)` | Calculated TOL values in the captured run. |
| `setze_offs(  72,  99,  13)` | Calculated OFS values in the captured run. |
| `(CAL=  0) fertig` | Calculation step is completed. |
| `SetUref` measurements repeated | Charger re-checks its calibrated measurements. |
| `(CHK=  0) fertig` | Completion marker received; XCAL returns to idle. |

The status percentages are transient device output, not direct metrology proof. Some lines carry `999%` or other large values while inputs settle or a channel is out of its measurement range. The trace documents observed behavior, not a universal limit specification.

See [the redacted original capture](../logs/CALIB_LOG_REDACTED.log).
