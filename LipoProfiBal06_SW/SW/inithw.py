# ==========================================================
# LipoProfiBal06
# ----------------------------------------------------------
# Module : inithw.py
#
# Hardware initialization
# ==========================================================

from machine import Pin, I2C, UART, PWM
from ssd1306 import SSD1306_I2C
from machine import UART

import config

# ==========================================================
# GLOBAL OBJECTS
# ==========================================================

display = None
uart = None
i2c = None

# LED

DebugLED = None
ReadyLED = None
CellLED = []

# BUTTONS

ButtonMenu = None
ButtonSet = None

# PWM

PWMOut = []

# ==========================================================
# GPIO
# ==========================================================

def InitGPIO():

    global DebugLED
    global ReadyLED
    global ButtonMenu
    global ButtonSet
    global CellLED

    DebugLED = Pin(config.PIN_DEBUG_LED, Pin.OUT)

    ReadyLED = Pin(config.PIN_READY_LED, Pin.OUT)

    ButtonMenu = Pin(config.PIN_MENU, Pin.IN, Pin.PULL_DOWN)

    ButtonSet = Pin(config.PIN_SET, Pin.IN, Pin.PULL_DOWN)

    CellLED = []

    for pin in config.CELL_LED_PINS:

        led = Pin(pin, Pin.OUT)

        led.off()

        CellLED.append(led)

    DebugLED.off()
    ReadyLED.off()

# ==========================================================
# I2C
# ==========================================================

def InitI2C():

    global i2c

    i2c = I2C(

        0,

        scl=Pin(config.PIN_SCL),

        sda=Pin(config.PIN_SDA),

        freq=400000

    )

# ==========================================================
# OLED
# ==========================================================

def InitDisplay():

    global display

    display = SSD1306_I2C(

        config.OLED_WIDTH,

        config.OLED_HEIGHT,

        i2c

    )

    display.fill(0)

    display.show()

# ==========================================================
# UART
# ==========================================================

def InitUART():

    global uart

    uart = UART(

        0,

        baudrate=config.UART_BAUD,

        tx=Pin(config.PIN_UART_TX),

        rx=Pin(config.PIN_UART_RX)

    )

# ==========================================================
# PWM
# ==========================================================

def InitPWM():

    global PWMOut

    PWMOut = []

    for pin in config.PWM_PINS:

        pwm = PWM(Pin(pin))

        pwm.freq(config.PWM_FREQ)

        pwm.duty_u16(0)

        PWMOut.append(pwm)

# ==========================================================
# INIT
# ==========================================================

def Init():

    print("HW Init")

    InitGPIO()

    InitI2C()

    InitDisplay()

    InitUART()

    InitPWM()

    print("HW Ready")