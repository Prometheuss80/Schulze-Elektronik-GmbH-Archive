from machine import Pin
from machine import I2C
from time import sleep

from ads1115 import ADS1115


i2c = I2C(
    0,
    scl=Pin(17),
    sda=Pin(16),
    freq=400000
)

ads = ADS1115(i2c)


print("ADS1115 TEST")

if ads.self_test():

    print("OK")

else:

    print("NOT FOUND")

    while True:
        pass


while True:

    raw = ads.read_average(0,64)

    volts = raw * 4.096 / 32768.0

    print(raw, "{:.5f} V".format(volts))

    sleep(0.5)