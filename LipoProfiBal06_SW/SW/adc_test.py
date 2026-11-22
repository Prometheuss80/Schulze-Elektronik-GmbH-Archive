import adc

adc.Init()

raw = adc.adc_raw[1]

# zatím podle současné reference používané v adc.py
v_adc = raw * adc.ADC_REFERENCE / 8388608.0

# 68k / 3k
v_cell = v_adc * (71.0 / 3.0)

print("RAW       =", raw)
print("ADC diff  =", v_adc * 1000, "mV")
print("CELL      =", v_cell * 1000, "mV")