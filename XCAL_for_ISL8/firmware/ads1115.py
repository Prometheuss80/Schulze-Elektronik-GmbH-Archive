from machine import I2C
from time import sleep_ms


class ADS1115:

    # Registry
    REG_CONVERSION = 0x00
    REG_CONFIG     = 0x01

    # PGA ±4.096V
    PGA_4096 = 0x0200

    # 128 SPS
    RATE_860 = 0x00E0

    # Single-shot
    MODE_SINGLE = 0x0100

    # Disable comparator
    COMP_DISABLE = 0x0003


    def __init__(self, i2c, address=0x48):

        self.i2c = i2c
        self.address = address


    def write_register(self, reg, value):

        self.i2c.writeto(
            self.address,
            bytes([
                reg,
                (value >> 8) & 0xFF,
                value & 0xFF
            ])
        )


    def read_register(self, reg):

        self.i2c.writeto(self.address, bytes([reg]))
        data = self.i2c.readfrom(self.address,2)

        return (data[0]<<8) | data[1]


    def read_raw(self, channel=0):

        mux = {
            0:0x4000,
            1:0x5000,
            2:0x6000,
            3:0x7000
        }

        config = (

            0x8000 |                 # Start conversion
            mux[channel] |
            self.PGA_4096 |
            self.MODE_SINGLE |
            self.RATE_860 |
            self.COMP_DISABLE

        )

        self.write_register(self.REG_CONFIG,config)

        #
        # čekání na převod
        #

        while True:

            cfg = self.read_register(self.REG_CONFIG)

            if cfg & 0x8000:
                break

            sleep_ms(1)

        value = self.read_register(self.REG_CONVERSION)

        #
        # signed integer
        #

        if value >= 32768:
            value -= 65536

        return value


    def read_voltage(self, channel=0):

        raw = self.read_raw(channel)

        return raw * 4.096 / 32768.0


    def read_average(self, channel=0, samples=32):

        s = 0

        for _ in range(samples):

            s += self.read_raw(channel)

        return s / samples


    def read_average_voltage(self, channel=0, samples=32):

        raw = self.read_average(channel,samples)

        return raw * 4.096 / 32768.0


    def self_test(self):

        try:

            cfg = self.read_register(self.REG_CONFIG)

            return True

        except:

            return False