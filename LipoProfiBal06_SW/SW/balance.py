# ==========================================================
# LipoProfiBal06
# Module : balance.py
# Active Balancing Controller
# ==========================================================

from time import ticks_ms, ticks_diff

import inithw
import led
import battery
import settings
import system
import display

MODE_OFF  = 0
MODE_HALF = 1
MODE_FULL = 2

TASK_PERIOD_MS = 100
STABILIZE_TIME_MS = 5000
STABILITY_MAX_SPAN_MV = 50

PWM_OFF  = 0
PWM_HALF = 32768
PWM_FULL = 65535

Mode = [MODE_OFF] * 6

_lastTask = ticks_ms()
_lastTimeUpdate = ticks_ms()

_phase = 0
_phaseStart = ticks_ms()

_stabMin = [0] * 6
_stabMax = [0] * 6
_balanceMs = [0] * 6


def Init():

    global _phase
    global _lastTask
    global _lastTimeUpdate

    _phase = 0
    _lastTask = ticks_ms()
    _lastTimeUpdate = ticks_ms()

    AllOff()


def _SetPWM(cell, duty):

    if 0 <= cell < len(inithw.PWMOut):
        inithw.PWMOut[cell].duty_u16(duty)


def AllOff():

    for cell in range(6):
        Mode[cell] = MODE_OFF
        _SetPWM(cell, PWM_OFF)


def Arm():

    global _phase
    global _phaseStart
    global _lastTask
    global _lastTimeUpdate

    AllOff()

    _phase = 1
    _phaseStart = ticks_ms()
    _lastTask = ticks_ms()
    _lastTimeUpdate = ticks_ms()

    count = battery.CellCount

    for cell in range(6):

        if cell < count:
            mv = battery.CellVoltageSigned[cell]
            _stabMin[cell] = mv
            _stabMax[cell] = mv
        else:
            _stabMin[cell] = 0
            _stabMax[cell] = 0

    led.ShowCells(count)

    print("BALANCE: 5s stabilization")


def Stop():

    global _phase

    _phase = 0
    AllOff()
    led.AllOff()


def IsStabilizing():
    return _phase == 1


def IsActive():
    return _phase == 2


def ActiveCells():

    result = []

    for cell in range(battery.CellCount):

        if Mode[cell] != MODE_OFF:
            result.append(cell + 1)

    return result


def _UpdateBalanceTime(now):

    global _lastTimeUpdate

    dt = ticks_diff(now, _lastTimeUpdate)

    if dt < 0:
        dt = 0

    _lastTimeUpdate = now

    for cell in range(battery.CellCount):

        if Mode[cell] != MODE_OFF:
            _balanceMs[cell] += dt

        battery.BalanceTime[cell] = (
            _balanceMs[cell] // 1000
        )


def _Stabilize(now):

    global _phase

    count = battery.CellCount

    AllOff()
    led.ShowCells(count)

    for cell in range(count):

        mv = battery.CellVoltageSigned[cell]

        if mv < _stabMin[cell]:
            _stabMin[cell] = mv

        if mv > _stabMax[cell]:
            _stabMax[cell] = mv

    if ticks_diff(
        now,
        _phaseStart
    ) < STABILIZE_TIME_MS:
        return

    for cell in range(count):

        span = _stabMax[cell] - _stabMin[cell]

        print(
            "STABILITY CELL{}: min={} max={} span={}mV".format(
                cell + 1,
                _stabMin[cell],
                _stabMax[cell],
                span
            )
        )

        if span > STABILITY_MAX_SPAN_MV:

            battery.ErrorCell = cell
            battery.ErrorVoltage = battery.CellVoltageSigned[cell]
            battery.CheckResult = battery.ERR_UNSTABLE

            Stop()
            system.EnterError()
            display.ShowBatteryError()
            return

    if battery.RuntimeSafetyCheck() != battery.CHECK_OK:

        Stop()
        system.EnterError()
        display.ShowBatteryError()
        return

    _phase = 2

    led.AllOff()

    print("BALANCE: active")


def _Calculate():

    count = battery.CellCount

    if count < 2:
        AllOff()
        return

    active = battery.CellVoltage[:count]
    umin = min(active)

    delta = settings.BalanceDelta

    if delta < 1:
        delta = 1

    for cell in range(6):

        if cell >= count:

            Mode[cell] = MODE_OFF
            _SetPWM(cell, PWM_OFF)
            continue

        diff = battery.CellVoltage[cell] - umin

        if diff <= delta:

            Mode[cell] = MODE_OFF
            _SetPWM(cell, PWM_OFF)

        elif diff < (2 * delta):

            Mode[cell] = MODE_HALF
            _SetPWM(cell, PWM_HALF)

        else:

            Mode[cell] = MODE_FULL
            _SetPWM(cell, PWM_FULL)


def _UpdateLED(now):

    led.AllOff()

    half_on = ((now // 1000) & 1) == 0

    for cell in range(battery.CellCount):

        mode = Mode[cell]

        if mode == MODE_FULL:
            led.CellOn(cell)

        elif mode == MODE_HALF and half_on:
            led.CellOn(cell)


def Task():

    global _lastTask

    now = ticks_ms()

    if not system.IsReady():

        if _phase != 0:
            Stop()

        return

    if ticks_diff(
        now,
        _lastTask
    ) < TASK_PERIOD_MS:
        return

    _lastTask = now

    _UpdateBalanceTime(now)

    if _phase == 1:
        _Stabilize(now)
        return

    if _phase != 2:
        AllOff()
        return

    _Calculate()
    _UpdateLED(now)
