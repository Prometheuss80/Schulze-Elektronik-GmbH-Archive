# ==========================================================
# LipoProfiBal06
# Module : battery.py
#
# Battery Data Model + ADC voltage conversion
# ==========================================================

import settings
import fram
import adc


# ==========================================================
# BATTERY
# ==========================================================

CellCount = 0
Chemistry = 0

SerialPort = 1
State = "L"
Current = 150
Temperature = None


# ==========================================================
# ADC / CELL MAPPING
# ==========================================================

# First implementation: CELL1..CELL6 use ADC CH0..CH5.
# This can later be changed in one place if PCB routing differs.
ADC_CHANNEL_MAP = (0, 1, 2, 3, 4, 5)

# External system-calibration gain in ppm.
# 1_000_000 = factor 1.000000
VoltageCalPPM = [
    1000000,
    1000000,
    1000000,
    1000000,
    1000000,
    1000000
]


# ==========================================================
# FRAM CALIBRATION AREA
# ==========================================================
#
# Existing settings occupy low addresses through 0x0018.
# Keep voltage calibration in a separate area.
#
# 0x0100  signature
# 0x0110  CH0 factor u32
# 0x0114  CH1 factor u32
# ...
# ==========================================================

CAL_SIGNATURE_ADDR = 0x0100
CAL_SIGNATURE = 0xA6
CAL_BASE = 0x0110


def _ReadU32(addr):
    data = fram.ReadBlock(addr, 4)

    return (
        data[0]
        | (data[1] << 8)
        | (data[2] << 16)
        | (data[3] << 24)
    )


def _WriteU32(addr, value):
    value &= 0xFFFFFFFF

    fram.WriteBlock(
        addr,
        bytes([
            value & 0xFF,
            (value >> 8) & 0xFF,
            (value >> 16) & 0xFF,
            (value >> 24) & 0xFF
        ])
    )


def LoadVoltageCalibration():
    global VoltageCalPPM

    if fram.ReadByte(CAL_SIGNATURE_ADDR) != CAL_SIGNATURE:

        for ch in range(6):
            _WriteU32(
                CAL_BASE + ch * 4,
                1000000
            )

        fram.WriteByte(
            CAL_SIGNATURE_ADDR,
            CAL_SIGNATURE
        )

    for ch in range(6):

        value = _ReadU32(
            CAL_BASE + ch * 4
        )

        # Reject obviously corrupted values.
        if value < 800000 or value > 1200000:
            value = 1000000

        VoltageCalPPM[ch] = value


def SaveVoltageCalibration():
    for ch in range(6):
        _WriteU32(
            CAL_BASE + ch * 4,
            VoltageCalPPM[ch]
        )

    fram.WriteByte(
        CAL_SIGNATURE_ADDR,
        CAL_SIGNATURE
    )


def SetVoltageCalibration(cell, reference_mV):
    """
    Future SET-menu helper.

    Uses the currently displayed/measured cell voltage and adjusts
    that channel's external calibration coefficient.
    """

    if cell < 0 or cell >= 6:
        return False

    measured = CellVoltage[cell]

    if measured <= 0:
        return False

    new_ppm = int(round(
        VoltageCalPPM[cell]
        * reference_mV
        / measured
    ))

    if new_ppm < 800000 or new_ppm > 1200000:
        return False

    VoltageCalPPM[cell] = new_ppm
    SaveVoltageCalibration()

    return True


# ==========================================================
# CELL DATA
# ==========================================================

CellVoltage = [0, 0, 0, 0, 0, 0]
CellVoltageSigned = [0, 0, 0, 0, 0, 0]

TotalVoltage = 0
MaxVoltage = 0
MinVoltage = 0
DeltaVoltage = 0

CHECK_PENDING        = 0
CHECK_OK             = 1
ERR_NO_BATTERY       = 2
ERR_SEQUENCE         = 3
ERR_REVERSED         = 4
ERR_LOW              = 5
ERR_HIGH             = 6
ERR_CELL_COUNT       = 7
ERR_UNSTABLE         = 8

CheckResult = CHECK_PENDING
DetectedCellCount = 0
ErrorCell = -1
ErrorVoltage = 0

ABSENT_MAX_MV = 500

# Battery Check does not trust a single ADC sample. A real cell must
# remain reasonably stable over several fresh ADC conversions.
CHECK_SAMPLES = 24
CHECK_MAX_SPAN_MV = 200


# ==========================================================
# BALANCER
# ==========================================================

BalanceActive = []

BalanceTime = [
    0, 0, 0, 0, 0, 0
]

EnergyMoved = [
    0, 0, 0, 0, 0, 0
]


# ==========================================================
# INIT
# ==========================================================

def Init():
    LoadVoltageCalibration()
    Update()


# ==========================================================
# ADC -> mV
# ==========================================================

def UpdateVoltages():

    for cell in range(6):

        ch = ADC_CHANNEL_MAP[cell]
        raw = adc.adc_raw[ch]

        voltage = adc.ADC_RawToVoltage(raw)

        voltage *= (
            VoltageCalPPM[cell]
            / 1000000.0
        )

        mv = int(round(voltage * 1000.0))

        CellVoltageSigned[cell] = mv
        CellVoltage[cell] = mv if mv > 0 else 0


# ==========================================================
# UPDATE
# ==========================================================

def Update():

    global CellCount
    global Chemistry
    global TotalVoltage
    global MaxVoltage
    global MinVoltage
    global DeltaVoltage

    Chemistry = settings.Chemistry

    if getattr(adc, "_initialized", False):
        UpdateVoltages()

    # Runtime CellCount is assigned ONLY by successful Battery Check.
    count = CellCount

    if count < 1 or count > 6:
        count = 0

    TotalVoltage = sum(CellVoltage[:count])

    active = CellVoltage[:count]

    if active:
        MaxVoltage = max(active)
        MinVoltage = min(active)
        DeltaVoltage = MaxVoltage - MinVoltage
    else:
        MaxVoltage = 0
        MinVoltage = 0
        DeltaVoltage = 0



# ==========================================================
# BATTERY CHECK
# ==========================================================

def ResetCheck():

    global CheckResult
    global DetectedCellCount
    global CellCount
    global ErrorCell
    global ErrorVoltage

    CheckResult = CHECK_PENDING
    DetectedCellCount = 0
    CellCount = 0
    ErrorCell = -1
    ErrorVoltage = 0


def _CellMilliVoltsFromRaw(cell):
    ch = ADC_CHANNEL_MAP[cell]
    raw = adc.adc_raw[ch]

    voltage = adc.ADC_RawToVoltage(raw)

    voltage *= (
        VoltageCalPPM[cell]
        / 1000000.0
    )

    return int(round(voltage * 1000.0))


def CheckBattery():

    global CheckResult, DetectedCellCount, CellCount
    global ErrorCell, ErrorVoltage

    CheckResult = CHECK_PENDING
    DetectedCellCount = 0
    CellCount = 0
    ErrorCell = -1
    ErrorVoltage = 0

    sums = [0] * 6
    samples = 0

    for _ in range(CHECK_SAMPLES):
        if not adc.ADC_ReadAll():
            continue
        samples += 1
        for cell in range(6):
            sums[cell] += _CellMilliVoltsFromRaw(cell)

    if samples == 0:
        CheckResult = ERR_NO_BATTERY
        return CheckResult

    for cell in range(6):
        mv = int(round(sums[cell] / samples))
        CellVoltageSigned[cell] = mv
        CellVoltage[cell] = mv if mv > 0 else 0

    print()
    print("BATTERY CHECK")
    print("Mode={} Chemistry={} Umin={} Umax={}".format(
        "AUTO" if settings.CellCount == settings.CELL_AUTO
        else "{}S".format(settings.CellCount),
        settings.CHEM_NAME[settings.Chemistry],
        settings.UnderVoltage,
        settings.OverVoltage
    ))
    for cell in range(6):
        print("CELL{} = {} mV".format(cell + 1, CellVoltageSigned[cell]))

    # Count contiguous cells that are valid for selected chemistry.
    count = 0
    while count < 6:
        mv = CellVoltageSigned[count]
        if settings.UnderVoltage <= mv <= settings.OverVoltage:
            count += 1
        else:
            break

    DetectedCellCount = count

    # CELL1 not valid -> no usable battery, regardless of fixed setting.
    if count == 0:
        mv0 = CellVoltageSigned[0]

        if mv0 < -ABSENT_MAX_MV:
            ErrorCell = 0
            ErrorVoltage = mv0
            CheckResult = ERR_REVERSED
            return CheckResult

        if mv0 > settings.OverVoltage:
            ErrorCell = 0
            ErrorVoltage = mv0
            CheckResult = ERR_HIGH
            return CheckResult

        # A valid later cell with invalid CELL1 = wiring error.
        for cell in range(1, 6):
            mv = CellVoltageSigned[cell]
            if settings.UnderVoltage <= mv <= settings.OverVoltage:
                ErrorCell = 0
                ErrorVoltage = mv0
                CheckResult = ERR_SEQUENCE
                return CheckResult

        CheckResult = ERR_NO_BATTERY
        return CheckResult

    # After contiguous valid cells, check if there is a valid cell higher up.
    # If not, the first invalid channel is simply END OF PACK.
    if count < 6:
        later_valid = False
        for cell in range(count + 1, 6):
            mv = CellVoltageSigned[cell]
            if settings.UnderVoltage <= mv <= settings.OverVoltage:
                later_valid = True
                break

        if later_valid:
            ErrorCell = count
            ErrorVoltage = CellVoltageSigned[count]
            CheckResult = ERR_SEQUENCE
            return CheckResult

        # IMPORTANT:
        # With this divider topology an unused next channel can be strongly
        # negative because P is pulled to AGND while N still sees the last
        # pack node. That is a normal end-of-pack signature.

    # Only now compare detected count with requested mode.
    if settings.CellCount == settings.CELL_AUTO:
        CellCount = count
        CheckResult = CHECK_OK
        Update()
        return CheckResult

    if settings.CellCount != count:
        ErrorCell = count if settings.CellCount > count else settings.CellCount
        ErrorVoltage = CellVoltageSigned[ErrorCell] if 0 <= ErrorCell < 6 else 0
        CheckResult = ERR_CELL_COUNT
        return CheckResult

    CellCount = count
    CheckResult = CHECK_OK
    Update()
    return CheckResult


def AcceptDetectedAsAuto():

    if CheckResult != ERR_CELL_COUNT:
        return False

    settings.CellCount = settings.CELL_AUTO
    settings.Save()
    ResetCheck()
    return True


def RejectDetectedCount():

    return CheckResult == ERR_CELL_COUNT


def CheckText():

    if CheckResult == ERR_NO_BATTERY:
        return ("NO BATTERY", "")

    if CheckResult == ERR_SEQUENCE:
        return ("WIRING ERROR", "Cell {}".format(ErrorCell + 1))

    if CheckResult == ERR_REVERSED:
        return (
            "CELL {} REVERSED".format(ErrorCell + 1),
            "{} mV".format(ErrorVoltage)
        )

    if CheckResult == ERR_LOW:
        return (
            "CELL {} LOW".format(ErrorCell + 1),
            "{} mV".format(ErrorVoltage)
        )

    if CheckResult == ERR_HIGH:
        return (
            "CELL {} HIGH".format(ErrorCell + 1),
            "{} mV".format(ErrorVoltage)
        )

    if CheckResult == ERR_CELL_COUNT:
        return (
            "CELL COUNT",
            "Set {}S / Det {}S".format(
                settings.CellCount,
                DetectedCellCount
            )
        )

    if CheckResult == ERR_UNSTABLE:
        return (
            "CELL {} UNSTABLE".format(ErrorCell + 1),
            "{} mV".format(ErrorVoltage)
        )

    return ("BATTERY OK", "{}S".format(DetectedCellCount))



# ==========================================================
# RUNTIME SAFETY CHECK
# ==========================================================

def RuntimeSafetyCheck():
    """
    Validate the already-detected pack during READY / BALANCE.

    This does NOT perform AUTO detection and does NOT change CellCount.
    It only checks whether the pack that previously passed Battery
    Check is still electrically valid.

    Returns:
        CHECK_OK or one of ERR_*.
    """

    global CheckResult
    global ErrorCell
    global ErrorVoltage

    if CellCount < 1 or CellCount > 6:
        CheckResult = ERR_NO_BATTERY
        ErrorCell = -1
        ErrorVoltage = 0
        return CheckResult

    # Use the latest ADC values.
    UpdateVoltages()

    # ------------------------------------------------------
    # Validate every cell that was confirmed at startup.
    # ------------------------------------------------------

    for cell in range(CellCount):

        mv = CellVoltageSigned[cell]

        # Physically missing / disconnected input.
        if abs(mv) < ABSENT_MAX_MV:
            ErrorCell = cell
            ErrorVoltage = mv
            CheckResult = ERR_NO_BATTERY
            return CheckResult

        # Reversed / broken balance lead can create negative voltage.
        if mv < 0:
            ErrorCell = cell
            ErrorVoltage = mv
            CheckResult = ERR_REVERSED
            return CheckResult

        if mv < settings.UnderVoltage:
            ErrorCell = cell
            ErrorVoltage = mv
            CheckResult = ERR_LOW
            return CheckResult

        if mv > settings.OverVoltage:
            ErrorCell = cell
            ErrorVoltage = mv
            CheckResult = ERR_HIGH
            return CheckResult

    # ------------------------------------------------------
    # There must not suddenly appear another valid cell above the
    # count that was confirmed by startup Battery Check.
    # ------------------------------------------------------

    for cell in range(CellCount, 6):

        mv = CellVoltageSigned[cell]

        if settings.UnderVoltage <= mv <= settings.OverVoltage:
            ErrorCell = cell
            ErrorVoltage = mv
            CheckResult = ERR_SEQUENCE
            return CheckResult

    ErrorCell = -1
    ErrorVoltage = 0
    CheckResult = CHECK_OK

    return CHECK_OK


# ==========================================================
# TASK
# ==========================================================

def Task():
    Update()
