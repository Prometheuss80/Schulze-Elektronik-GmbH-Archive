from machine import Pin, I2C, UART
from ads1115 import ADS1115
from ssd1306 import SSD1306_I2C
from time import sleep
import framebuf

# ==================================================
# KONSTANTY
# ==================================================

MODE_AUTO = 0
MODE_MANUAL = 1

STAV_IDLE = 0

STAV_UREF_3V = 1
STAV_UREF_4V = 2
STAV_UREF_8V = 3
STAV_UREF_12V = 4

UREF_TABLE = {
    STAV_UREF_3V: 3.000,
    STAV_UREF_4V: 4.200,
    STAV_UREF_8V: 8.400,
    STAV_UREF_12V: 12.600
}

R_TOP = 20000
R_BOTTOM = 2200

DIVIDER_RATIO = (R_TOP + R_BOTTOM) / R_BOTTOM

ADC_MAX_VOLTAGE = 20.0

ADC_SAMPLES = 128
ADC_DISCARD = 16

# ==========================================
# ADC Calibration (Schulze style)
# ==========================================

last_cmd = ""

serial_buffer = ""

# ==================================================
# FUNKCE
# ==================================================

def ReadADC(channel=0):

    values = []

    for _ in range(ADC_SAMPLES):
        values.append(ads.read_raw(channel))

    values.sort()

    values = values[ADC_DISCARD:-ADC_DISCARD]

    return sum(values) / len(values)


def ReadVoltage():

    raw = ReadADC(0)

    adc_voltage = (raw - ADC_ZERO) * ADC_GAIN

    return adc_voltage * DIVIDER_RATIO


def CalibrateADC():

    # ---------- NULA ----------
    relay_gnd.on()
    sleep(3)

    for _ in range(10):
        ads.read_raw(0)

    zero = ads.read_average(0, 256)

    relay_gnd.off()      # <-- chybí

    relay_ref.on()
    sleep(3)

    for _ in range(10):
        ads.read_raw(0)

    ref = ads.read_average(0, 256)

    relay_ref.off()

    delta = ref - zero

    if delta < 100:
        SignalError()
        raise Exception("ADC calibration failed")

    gain = (4.205 / DIVIDER_RATIO) / delta

    return zero, gain


# ==================================================
# SIGNALIZACE HW LED
# ==================================================

def BlinkHW(count=3, ton=0.05, toff=0.15):

    for _ in range(count):

        ledHW.on()
        sleep(ton)

        ledHW.off()
        sleep(toff)


def SignalOK():
    BlinkHW(1)


def SignalRS232():
    BlinkHW(2)


def SignalStart():
    BlinkHW(3)


def SignalError():
    BlinkHW(5)

def update_leds(stav):

    led1.off()
    led2.off()
    led3.off()
    led4.off()

    if stav == STAV_UREF_3V:
        led1.on()

    elif stav == STAV_UREF_4V:
        led2.on()

    elif stav == STAV_UREF_8V:
        led3.on()

    elif stav == STAV_UREF_12V:
        led4.on()


def show_intro():

    display.fill(0)

    display.text("calibration", 0, 0, 1)
    display.text("Schulze", 0, 12, 1)
    display.text("ISL8-936G", 0, 24, 1)
    display.text("Prometheuss80", 0, 36, 1)
    display.text("3/4/8/12V", 0, 48, 1)

    display.show()


def show_schulze():

    try:

        with open("schulze.pbm", "rb") as f:

            f.readline()              # P4
            f.readline()              # komentář

            size = f.readline().decode().strip()

            width, height = map(int, size.split())

            data = bytearray(f.read())

        fbuf = framebuf.FrameBuffer(
            data,
            width,
            height,
            framebuf.MONO_HLSB
        )

        x = (128 - width) // 2
        y = (64 - height) // 2

        display.fill(0)
        display.blit(fbuf, x, y)
        display.show()

    except Exception as e:

        print("LOGO ERROR:", e)

        display.fill(0)
        display.text("LOGO ERROR", 0, 28, 1)
        display.show()

def show_xcal():

    try:

        with open("xcal.pbm", "rb") as f:

            f.readline()              # P4
            f.readline()              # komentář

            size = f.readline().decode().strip()

            width, height = map(int, size.split())

            data = bytearray(f.read())

        fbuf = framebuf.FrameBuffer(
            data,
            width,
            height,
            framebuf.MONO_HLSB
        )

        x = (128 - width) // 2
        y = (64 - height) // 2

        display.fill(0)
        display.blit(fbuf, x, y)
        display.show()

    except Exception as e:

        print("LOGO ERROR:", e)

        display.fill(0)
        display.text("LOGO ERROR", 0, 28, 1)
        display.show()

def ReadSerial():

    global serial_buffer

    while uart.any():

        c = uart.read(1)

        if c:

            c = c.decode()

            if c == "\n":

                line = serial_buffer.strip()

                serial_buffer = ""

                return line

            else:

                serial_buffer += c

    return None


def ProcessCommand(cmd):

    global last_cmd
    global stav

    last_cmd = cmd

    print("RX:", cmd)

    # ----------------------------------------
    # SetUref
    # ----------------------------------------

    if "SetUref:" in cmd:

        value = int(cmd.split(":")[1].strip())

        if value == 10:

            stav = STAV_UREF_3V
            print("UREF = 3.000V")

        elif value == 8:

            stav = STAV_UREF_4V
            print("UREF = 4.200V")

        elif value == 4:

            stav = STAV_UREF_8V
            print("UREF = 8.400V")

        elif value == 0:

            stav = STAV_UREF_12V
            print("UREF = 12.600V")

        update_leds(stav)
        SignalRS232()

    # ----------------------------------------
    # Konec kalibrace
    # ----------------------------------------

    elif "(CHK=" in cmd and "fertig" in cmd:

        stav = STAV_IDLE

        update_leds(stav)
        SignalRS232()

        print("CALIBRATION FINISHED")

def ShowMainScreen(volty, stav, mode):

    display.fill(0)

    display.text("AUTO CALIB", 0, 0, 1)

    if mode == MODE_AUTO:
        display.text("MODE:AUTO", 0, 12, 1)
    else:
        display.text("MODE:MAN", 0, 12, 1)

    if stav in UREF_TABLE:

        txt = "REF {:.3f}V".format(UREF_TABLE[stav])

    else:

        txt = "Waiting RS232"

    display.text(txt, 0, 24, 1)

    display.text(
        "U={:.3f}V".format(volty),
        0,
        36,
        1
    )

    bar = int((volty / ADC_MAX_VOLTAGE) * 128)

    if bar > 128:
        bar = 128

    display.fill_rect(
        0,
        54,
        bar,
        8,
        1
    )

    display.show()


# ==================================================
# HARDWARE
# ==================================================


ledHW = Pin(25, Pin.OUT)


# I2C
i2c = I2C(
    0,
    scl=Pin(17),
    sda=Pin(16),
    freq=400000
)

# ADS1115
ads = ADS1115(i2c)

if not ads.self_test():

    print("ADS1115 NOT FOUND")

    SignalError()

    while True:
        pass

print("ADS1115 OK")

led1 = Pin(15, Pin.OUT)
led2 = Pin(14, Pin.OUT)
led3 = Pin(13, Pin.OUT)
led4 = Pin(12, Pin.OUT)

relay_gnd = Pin(6, Pin.OUT)
relay_ref = Pin(7, Pin.OUT)

relay_gnd.off()
relay_ref.off()

update_leds(STAV_IDLE)

tl_plus = Pin(9, Pin.IN, Pin.PULL_DOWN)
tl_reset = Pin(10, Pin.IN, Pin.PULL_DOWN)
tl_minus = Pin(11, Pin.IN, Pin.PULL_DOWN)

display = SSD1306_I2C(
    128,
    64,
    i2c
)

# UART0
# případně změníme piny podle zapojení

uart = UART(
    0,
    baudrate=9600,
    tx=Pin(0),
    rx=Pin(1)
)

# ==================================================
# START
# ==================================================

ledHW.off()

SignalStart()

show_xcal()
sleep(2)


# Start programu
SignalStart()

show_schulze()
#sleep(1.5)

ADC_ZERO,ADC_GAIN = CalibrateADC()


print("ZERO =", ADC_ZERO)
print("GAIN =", ADC_GAIN)

raw = ads.read_average(0, 256)
print("RAW =", raw)
print("OFFSET =", (raw - ADC_ZERO) * ADC_GAIN * DIVIDER_RATIO)

relay_gnd.off()
relay_ref.off()

# Intro zobrazeno
SignalStart()

show_intro()
sleep(2)

# Logo zobrazeno
SignalStart()

# ==================================================
# STAVY
# ==================================================

mode = MODE_AUTO
stav = STAV_IDLE

old_plus = 0
old_minus = 0
old_reset = 0

# inicializace filtru
filtrovane_napeti = ReadVoltage()

# ==================================================
# HLAVNÍ SMYČKA
# ==================================================

while True:

    plus = tl_plus.value()
    minus = tl_minus.value()
    reset = tl_reset.value()

    # ------------------------------------------
    # PLUS
    # ------------------------------------------

    if plus and not old_plus:

        mode = MODE_MANUAL

        if stav == STAV_IDLE:
            stav = STAV_UREF_3V

        elif stav < STAV_UREF_12V:
            stav += 1

# na 12.6V zůstane stát

        update_leds(stav)
        SignalOK()
        
    # ------------------------------------------
    # MINUS
    # ------------------------------------------

    if minus and not old_minus:

        mode = MODE_MANUAL

        if stav == STAV_IDLE:
            stav = STAV_UREF_12V

        elif stav > STAV_UREF_3V:
            stav -= 1

# na 3.0V zůstane stát

        update_leds(stav)
        SignalOK()
        
    # ------------------------------------------
    # RESET
    # ------------------------------------------

    if reset and not old_reset:

        mode = MODE_AUTO
        stav = STAV_IDLE

        update_leds(stav)
        SignalOK()
        
    # ------------------------------------------
    # RS232
    # ------------------------------------------

    if mode == MODE_AUTO:

        cmd = ReadSerial()

        if cmd:

            ProcessCommand(cmd)

    # ------------------------------------------
    # ULOŽENÍ STAVŮ TLAČÍTEK
    # ------------------------------------------

    old_plus = plus
    old_minus = minus
    old_reset = reset

    # ------------------------------------------
    # ADC + OLED
    # ------------------------------------------

    volty = ReadVoltage()

    filtrovane_napeti = (
        filtrovane_napeti * 0.40 +
        volty * 0.60
    )

    ShowMainScreen(
        #filtrovane_napeti,
        volty,
        stav,
        mode
    )

    sleep(0.01)