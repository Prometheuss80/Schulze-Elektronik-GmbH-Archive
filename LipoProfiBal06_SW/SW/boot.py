# ==========================================================
# LipoProfiBal06
# Module : boot.py
#
# Boot sequence
# ==========================================================

from time import sleep_ms

import display
import led
import system
import uart
import battery
import settings


def Boot():

    # ------------------------------------------------------
    # BOOT SCREEN
    # ------------------------------------------------------

    display.ShowBoot()
    sleep_ms(1000)

    display.ShowLogo()
    sleep_ms(2000)

    led.Test()

    # ------------------------------------------------------
    # UART HEADER
    # ------------------------------------------------------

    uart.Header()

    if settings.UartEnable:
        sleep_ms(1000)

    uart.Enable()

    # ------------------------------------------------------
    # BATTERY CHECK
    # ------------------------------------------------------
    #
    # IMPORTANT:
    # Boot must NEVER enter READY directly.
    # READY is enabled exclusively by main.py after
    # battery.CheckBattery() returns CHECK_OK.
    # ------------------------------------------------------

    battery.ResetCheck()

    display.ShowInfo(
        "Battery check",
        "Please wait"
    )

    system.EnterCheck()
