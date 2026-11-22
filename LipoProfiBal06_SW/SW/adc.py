# ==========================================================
# LipoProfiBal06
# ADS131M08 ADC driver
#
# CONFIG mode:
#   ADC channels disabled -> RREG/WREG without DRDY dependency
#
# RUN mode:
#   ADC channels enabled -> Task() services DRDY and reads
#   complete 10-word / 30-byte frames
# ==========================================================

from machine import Pin, SPI, PWM
from time import ticks_ms, ticks_diff, sleep_ms

PIN_DRDY  = 8
PIN_CS    = 9
PIN_SCLK  = 10
PIN_MOSI  = 11
PIN_MISO  = 12
PIN_CLKIN = 28

CLKIN_FREQ = 8192000
SPI_BAUDRATE = 2000000

ADC_CHANNELS = 8
ADC_WORDS = 10
ADC_FRAME_SIZE = 30

REG_ID     = 0x00
REG_STATUS = 0x01
REG_MODE   = 0x02
REG_CLOCK  = 0x03
REG_GAIN1  = 0x04
REG_GAIN2  = 0x05

ADC_CH_CFG_BASE = 0x09

# OSR bits 4:2 = 111, HR power bits = 10
CLOCK_CONFIG = 0x001E
CLOCK_RUN    = 0xFF1E

MODE_24BIT = 0x0510
GAIN1_DEFAULT = 0x0000
GAIN2_DEFAULT = 0x0000

spi = None
cs = None
drdy = None
clkin = None

adc_raw = [0] * ADC_CHANNELS
adc_frame = bytearray(ADC_FRAME_SIZE)
adc_offset = [0] * ADC_CHANNELS
adc_gcal = [0x800000] * ADC_CHANNELS

_initialized = False
_running = False
_frame_count = 0
_last_frame_ok = False


def ADC_CH_REG(ch):
    return ADC_CH_CFG_BASE + ch * 5


def ADC_OCAL_MSB_REG(ch):
    return ADC_CH_REG(ch) + 1


def ADC_OCAL_LSB_REG(ch):
    return ADC_CH_REG(ch) + 2


def ADC_GCAL_MSB_REG(ch):
    return ADC_CH_REG(ch) + 3


def ADC_GCAL_LSB_REG(ch):
    return ADC_CH_REG(ch) + 4


def ADC_Convert24(b0, b1, b2):
    value = (b0 << 16) | (b1 << 8) | b2
    if value & 0x800000:
        value -= 0x1000000
    return value


def ADC_GetWord16(frame, word=0):
    p = word * 3
    return (frame[p] << 8) | frame[p + 1]


def ADC_GetWord24(frame, word):
    p = word * 3
    return (frame[p] << 16) | (frame[p + 1] << 8) | frame[p + 2]


def _wait_low(timeout_ms=200):
    start = ticks_ms()
    while drdy.value() == 1:
        if ticks_diff(ticks_ms(), start) > timeout_ms:
            return False
    return True


def WaitDRDY(timeout_ms=200):
    start = ticks_ms()

    while drdy.value() == 0:
        if ticks_diff(ticks_ms(), start) > timeout_ms:
            return False

    return _wait_low(timeout_ms)


def ADC_TransferFrame(tx=None):
    global adc_frame

    if tx is None:
        tx = bytearray(ADC_FRAME_SIZE)

    if len(tx) != ADC_FRAME_SIZE:
        return None

    rx = bytearray(ADC_FRAME_SIZE)

    cs.value(0)
    spi.write_readinto(tx, rx)
    cs.value(1)

    adc_frame = rx
    return rx


def _make_wreg(reg, value):
    command = 0x6000 | ((reg & 0x7F) << 7)
    tx = bytearray(ADC_FRAME_SIZE)

    tx[0] = (command >> 8) & 0xFF
    tx[1] = command & 0xFF

    tx[3] = (value >> 8) & 0xFF
    tx[4] = value & 0xFF

    return tx


def _make_rreg(reg):
    command = 0xA000 | ((reg & 0x7F) << 7)
    tx = bytearray(ADC_FRAME_SIZE)

    tx[0] = (command >> 8) & 0xFF
    tx[1] = command & 0xFF

    return tx


def _expected_wreg_response(reg):
    return 0x4000 | ((reg & 0x7F) << 7)


def ADC_Reset():
    """
    Send SPI RESET unconditionally.

    Important for soft reboot recovery: the ADC may have been left
    in CONFIG mode with all channels disabled, in which case DRDY
    remains HIGH forever. RESET must therefore not wait for DRDY
    before transmitting.
    """

    tx = bytearray(ADC_FRAME_SIZE)
    tx[0] = 0x00
    tx[1] = 0x11

    print("ADC RESET")

    ADC_TransferFrame(tx)
    sleep_ms(20)

    if not _wait_low(200):
        print("ADC RESET: no DRDY after reset")
        return False

    print("ADC RESET: DRDY restored")
    return True


def _config_read(reg, verbose=False):
    ADC_TransferFrame(_make_rreg(reg))
    rx = ADC_TransferFrame()

    if rx is None:
        return None

    value = ADC_GetWord16(rx, 0)

    if verbose:
        print("RREG 0x{:02X} = 0x{:04X}".format(reg, value))

    return value


def _config_write(reg, value, verbose=False):
    """
    Robust CONFIG-mode WREG.

    In practice the first response frame can rarely contain a stale
    STATUS/response word even with channels disabled. Retry the same
    WREG once before declaring failure.
    """

    expected = _expected_wreg_response(reg)

    for attempt in range(2):

        ADC_TransferFrame(_make_wreg(reg, value))
        rx = ADC_TransferFrame()

        if rx is None:
            return False

        response = ADC_GetWord16(rx, 0)
        ok = (response == expected)

        if verbose:
            print(
                "WREG 0x{:02X} <- 0x{:04X}  ACK=0x{:04X} EXPECT=0x{:04X} {}{}".format(
                    reg,
                    value,
                    response,
                    expected,
                    "OK" if ok else "FAIL",
                    "" if attempt == 0 else " RETRY"
                )
            )

        if ok:
            return True

    return False


def ADC_EnterConfig():
    global _running

    expected = _expected_wreg_response(REG_CLOCK)

    # While RUN mode is active, synchronize the first command to DRDY.
    # Occasionally the first response can still be the previous STATUS
    # word. Retry once; after the first CLOCK write the channels are
    # already disabled, so no further DRDY wait is needed.
    for attempt in range(2):

        if attempt == 0 and _running:
            if not _wait_low(200):
                print("CONFIG: DRDY timeout before channel disable")
                return False

        ADC_TransferFrame(_make_wreg(REG_CLOCK, CLOCK_CONFIG))
        rx = ADC_TransferFrame()

        if rx is None:
            print("CONFIG: no CLOCK response")
            return False

        ack = ADC_GetWord16(rx, 0)

        print(
            "CHANNELS OFF: ACK=0x{:04X} EXPECT=0x{:04X}{}".format(
                ack,
                expected,
                "" if attempt == 0 else " RETRY"
            )
        )

        if ack == expected:
            _running = False
            return True

    print("CONFIG: CLOCK disable ACK failed")
    return False


def ADC_EnterRun():
    global _running

    expected = _expected_wreg_response(REG_CLOCK)

    print("WREG 0x{:02X} <- 0x{:04X}".format(REG_CLOCK, CLOCK_RUN))

    ADC_TransferFrame(_make_wreg(REG_CLOCK, CLOCK_RUN))

    ack_ok = False

    for i in range(4):
        rx = ADC_TransferFrame()

        if rx is None:
            break

        ack = ADC_GetWord16(rx, 0)

        print("RUN ACK FRAME {}: 0x{:04X}".format(i, ack))

        if ack == expected:
            ack_ok = True
            break

    if not ack_ok:
        print("CHANNEL ENABLE FAILED - expected 0x{:04X}".format(expected))
        return False

    _running = True
    sleep_ms(20)

    if not ADC_SyncRun():
        print("RUN SYNC FAILED")
        return False

    return True


def ADC_SyncRun():
    global adc_frame, _last_frame_ok

    if not _wait_low(200):
        print("SYNC: DRDY timeout")
        return False

    # Drain both FIFO positions after a pause.
    frame1 = ADC_TransferFrame()
    frame2 = ADC_TransferFrame()

    if frame1 is None or frame2 is None:
        return False

    adc_frame = frame2
    _decode_frame(frame2)
    _last_frame_ok = True

    return True


def ReadRegister(reg, verbose=True):
    was_running = _running

    if was_running:
        if not ADC_EnterConfig():
            return None

    value = _config_read(reg, verbose=verbose)

    if was_running:
        if not ADC_EnterRun():
            return None

    return value


def WriteRegister(reg, value, verbose=True):
    was_running = _running

    if was_running:
        if not ADC_EnterConfig():
            return False

    ok = _config_write(reg, value, verbose=verbose)

    if was_running:
        if not ADC_EnterRun():
            return False

    return ok


def WriteRegisterVerify(reg, value):
    was_running = _running

    if was_running:
        if not ADC_EnterConfig():
            return False

    ok = _config_write(reg, value, verbose=True)

    if ok:
        readback = _config_read(reg, verbose=True)
        ok = (readback == value)

        print(
            "VERIFY REG=0x{:02X} EXPECT=0x{:04X} READ=0x{:04X} -> {}".format(
                reg,
                value,
                readback if readback is not None else 0,
                ok
            )
        )

    if was_running:
        if not ADC_EnterRun():
            return False

    return ok


def ADC_GetChannel(frame, ch):
    if frame is None or len(frame) != ADC_FRAME_SIZE:
        return None

    if ch < 0 or ch >= ADC_CHANNELS:
        return None

    p = (ch + 1) * 3

    return ADC_Convert24(
        frame[p],
        frame[p + 1],
        frame[p + 2]
    )


def ADC_GetStatus(frame):
    if frame is None or len(frame) != ADC_FRAME_SIZE:
        return None

    return ADC_GetWord16(frame, 0)


def _decode_frame(frame):
    for ch in range(ADC_CHANNELS):
        adc_raw[ch] = ADC_GetChannel(frame, ch)


def ADC_ReadFrame():
    if not _running:
        return None

    if not _wait_low(200):
        return None

    ADC_TransferFrame()
    frame = ADC_TransferFrame()

    if frame is not None:
        _decode_frame(frame)

    return frame


def ADC_ReadAll():
    return ADC_ReadFrame() is not None


def ADC_Update():
    global adc_frame, _frame_count, _last_frame_ok

    if not _running or drdy is None:
        return False

    if drdy.value() != 0:
        return False

    frame1 = ADC_TransferFrame()
    frame2 = ADC_TransferFrame()

    if frame1 is None or frame2 is None:
        _last_frame_ok = False
        return False

    adc_frame = frame2
    _decode_frame(frame2)

    _frame_count += 1
    _last_frame_ok = True

    return True


def Task():
    return ADC_Update()


def Init():
    global spi, cs, drdy, clkin
    global _initialized, _running

    print("ADS131M08 INIT")
    print("SPI MODE1 / 24-bit / 30-byte frames")
    print("CONFIG -> RUN architecture")

    cs = Pin(PIN_CS, Pin.OUT, value=1)
    drdy = Pin(PIN_DRDY, Pin.IN)

    clkin = PWM(Pin(PIN_CLKIN))
    clkin.freq(CLKIN_FREQ)
    clkin.duty_u16(32768)

    spi = SPI(
        1,
        baudrate=SPI_BAUDRATE,
        polarity=0,
        phase=1,
        bits=8,
        firstbit=SPI.MSB,
        sck=Pin(PIN_SCLK),
        mosi=Pin(PIN_MOSI),
        miso=Pin(PIN_MISO)
    )

    sleep_ms(10)

    if not ADC_Reset():
        print("ADC RESET FAILED")
        return False

    print("RESET OK")

    if not ADC_EnterConfig():
        print("ADC CONFIG ENTRY FAILED")
        return False

    print("CONFIG MODE")

    if not _config_write(REG_MODE, MODE_24BIT, verbose=True):
        print("MODE WRITE FAILED")
        return False

    if not _config_write(REG_GAIN1, GAIN1_DEFAULT, verbose=True):
        print("GAIN1 WRITE FAILED")
        return False

    if not _config_write(REG_GAIN2, GAIN2_DEFAULT, verbose=True):
        print("GAIN2 WRITE FAILED")
        return False

    print()
    print("CONFIG REGISTER CHECK")

    id_value     = _config_read(REG_ID, verbose=True)
    status_value = _config_read(REG_STATUS, verbose=True)
    mode_value   = _config_read(REG_MODE, verbose=True)
    clock_value  = _config_read(REG_CLOCK, verbose=True)
    gain1_value  = _config_read(REG_GAIN1, verbose=True)
    gain2_value  = _config_read(REG_GAIN2, verbose=True)

    print()
    print("ID     = 0x{:04X}".format(id_value if id_value is not None else 0))
    print("STATUS = 0x{:04X}".format(status_value if status_value is not None else 0))
    print("MODE   = 0x{:04X}".format(mode_value if mode_value is not None else 0))
    print("CLOCK  = 0x{:04X}".format(clock_value if clock_value is not None else 0))
    print("GAIN1  = 0x{:04X}".format(gain1_value if gain1_value is not None else 0))
    print("GAIN2  = 0x{:04X}".format(gain2_value if gain2_value is not None else 0))

    if mode_value != MODE_24BIT:
        print("MODE VERIFY FAILED")
        return False

    if clock_value != CLOCK_CONFIG:
        print("CLOCK CONFIG VERIFY FAILED")
        return False

    if gain1_value != GAIN1_DEFAULT or gain2_value != GAIN2_DEFAULT:
        print("GAIN VERIFY FAILED")
        return False

    if id_value is None or id_value in (0x0000, 0xFFFF, 0x05FF):
        print("ID VERIFY FAILED")
        return False

    print()
    print("ENTER RUN MODE")

    if not ADC_EnterRun():
        print("RUN MODE FAILED")
        return False

    _initialized = True

    print("ADC INIT OK")
    return True


def ADC_PrintRaw():
    print()
    print("ADC RAW VALUES")

    for ch in range(ADC_CHANNELS):
        print(
            "CH{} = {:10d}  0x{:06X}".format(
                ch,
                adc_raw[ch],
                adc_raw[ch] & 0xFFFFFF
            )
        )

    print()


def PrintFrame(frame, title="FRAME"):
    if frame is None:
        print(title, "= NONE")
        return

    print()
    print(title)
    print("--------------------------------")

    for word in range(ADC_WORDS):
        p = word * 3

        print(
            "W{:02d}: {:02X} {:02X} {:02X}  0x{:06X}".format(
                word,
                frame[p],
                frame[p + 1],
                frame[p + 2],
                ADC_GetWord24(frame, word)
            )
        )

    print("--------------------------------")


def ADC_Status():
    print("initialized =", _initialized)
    print("running     =", _running)
    print("DRDY        =", None if drdy is None else drdy.value())
    print("frames      =", _frame_count)
    print("last ok     =", _last_frame_ok)
    print("STATUS W00  = 0x{:04X}".format(ADC_GetStatus(adc_frame) or 0))


def Test_RREG_Frames(reg, frames=4):
    """
    Compatibility diagnostic retained from earlier test driver.

    In RUN mode this safely enters CONFIG mode first. The actual RREG
    response is printed, followed by extra NULL frames while channels
    remain disabled, then RUN mode is restored.
    """
    was_running = _running

    print()
    print("################################")
    print("RREG FRAME TEST")
    print("REG = 0x{:02X}".format(reg))
    print("################################")

    if was_running:
        if not ADC_EnterConfig():
            print("ENTER CONFIG FAILED")
            return False

    command_rx = ADC_TransferFrame(_make_rreg(reg))
    PrintFrame(command_rx, "RREG COMMAND RX FRAME")

    for i in range(frames):
        rx = ADC_TransferFrame()
        PrintFrame(rx, "RREG RESPONSE FRAME {}".format(i))

    if was_running:
        if not ADC_EnterRun():
            print("RESTORE RUN FAILED")
            return False

    return True


def ADC_SetChannelMUX(ch, mux):
    if ch < 0 or ch >= ADC_CHANNELS:
        return False

    return WriteRegister(ADC_CH_REG(ch), mux, verbose=True)


def ADC_SetChannelMUXVerify(ch, mux):
    if ch < 0 or ch >= ADC_CHANNELS:
        return False

    return WriteRegisterVerify(ADC_CH_REG(ch), mux)


def ADC_GetChannelMUX(ch):
    if ch < 0 or ch >= ADC_CHANNELS:
        return None

    return ReadRegister(ADC_CH_REG(ch), verbose=True)


def Test_ChannelMUX(ch):
    print()
    print("CHANNEL MUX TEST CH{}".format(ch))

    if not ADC_SetChannelMUXVerify(ch, 0x0001):
        return False

    sleep_ms(50)

    if not ADC_SetChannelMUXVerify(ch, 0x0000):
        return False

    print("CHANNEL MUX TEST OK")
    return True


def _config_write_cal24(msb_reg, lsb_reg, value24, verbose=False):
    """
    Write one ADS131M08 24-bit calibration value.

    MSB register contains bits 23:8.
    LSB register bits 15:8 contain calibration bits 7:0.
    """
    value24 &= 0xFFFFFF

    msb = (value24 >> 8) & 0xFFFF
    lsb = (value24 & 0xFF) << 8

    if not _config_write(msb_reg, msb, verbose=verbose):
        return False

    if not _config_write(lsb_reg, lsb, verbose=verbose):
        return False

    return True


def _average_channels(count):
    sums = [0] * ADC_CHANNELS

    for _ in range(count):
        frame = ADC_ReadFrame()

        if frame is None:
            return None

        for ch in range(ADC_CHANNELS):
            sums[ch] += ADC_GetChannel(frame, ch)

    return [
        int(round(sums[ch] / count))
        for ch in range(ADC_CHANNELS)
    ]


def ADC_CalibrateInternal(count=200):
    """
    Complete internal ADS131M08 channel calibration.

    Step 1: reset OCAL=0 and GCAL=1.0
    Step 2: MUX=01 (AGND), measure zero offset
    Step 3: write measured zero offset to OCALn
    Step 4: MUX=10 (positive internal DC test signal)
    Step 5: calculate and write GCALn
    Step 6: restore MUX=00 normal analog inputs

    The internal positive test signal is nominally 2/15 of ADC
    differential full scale, therefore its ideal code is 2/15 * 2^23.
    """
    global adc_offset
    global adc_gcal

    print()
    print("================================")
    print("ADC INTERNAL CALIBRATION")
    print("================================")

    # ------------------------------------------------------
    # Establish neutral calibration coefficients.
    # ------------------------------------------------------

    if not ADC_EnterConfig():
        return False

    for ch in range(ADC_CHANNELS):

        # OCAL = 0
        if not _config_write_cal24(
            ADC_OCAL_MSB_REG(ch),
            ADC_OCAL_LSB_REG(ch),
            0x000000
        ):
            print("OCAL RESET FAILED CH{}".format(ch))
            return False

        # GCAL = 1.0
        if not _config_write_cal24(
            ADC_GCAL_MSB_REG(ch),
            ADC_GCAL_LSB_REG(ch),
            0x800000
        ):
            print("GCAL RESET FAILED CH{}".format(ch))
            return False

        # MUX = 01 -> both ADC inputs to AGND
        if not _config_write(
            ADC_CH_REG(ch),
            0x0001,
            verbose=False
        ):
            print("GND MUX FAILED CH{}".format(ch))
            return False

    # ------------------------------------------------------
    # OFFSET / GND
    # ------------------------------------------------------

    if not ADC_EnterRun():
        return False

    # Allow delta-sigma filters to settle after MUX change.
    for _ in range(20):
        if ADC_ReadFrame() is None:
            return False

    offsets = _average_channels(count)

    if offsets is None:
        return False

    adc_offset = offsets[:]

    print("GND OFFSETS:")
    for ch in range(ADC_CHANNELS):
        print("CH{} = {}".format(ch, offsets[ch]))

    # Program OCAL. Datasheet applies:
    # corrected = (raw - OCAL) * GCAL / 2^23
    if not ADC_EnterConfig():
        return False

    for ch in range(ADC_CHANNELS):
        if not _config_write_cal24(
            ADC_OCAL_MSB_REG(ch),
            ADC_OCAL_LSB_REG(ch),
            offsets[ch]
        ):
            print("OCAL WRITE FAILED CH{}".format(ch))
            return False

        # MUX = 10 -> positive internal DC test signal
        if not _config_write(
            ADC_CH_REG(ch),
            0x0002,
            verbose=False
        ):
            print("TEST MUX FAILED CH{}".format(ch))
            return False

    # ------------------------------------------------------
    # INTERNAL TEST SIGNAL / GAIN
    # ------------------------------------------------------

    if not ADC_EnterRun():
        return False

    for _ in range(20):
        if ADC_ReadFrame() is None:
            return False

    measured = _average_channels(count)

    if measured is None:
        return False

    # Positive internal test signal = 2/15 of differential FS.
    ideal = int(round((2.0 / 15.0) * 8388608.0))

    print("INTERNAL REF TEST:")
    print("IDEAL =", ideal)

    gcal = [0] * ADC_CHANNELS

    for ch in range(ADC_CHANNELS):

        value = measured[ch]

        if value <= 0:
            print("GAIN MEASURE FAILED CH{} = {}".format(ch, value))
            return False

        coeff = int(round(
            8388608.0 * ideal / value
        ))

        if coeff < 1:
            coeff = 1

        if coeff > 0xFFFFFF:
            coeff = 0xFFFFFF

        gcal[ch] = coeff

        print(
            "CH{} RAW={} GCAL=0x{:06X} FACTOR={:.8f}".format(
                ch,
                value,
                coeff,
                coeff / 8388608.0
            )
        )

    # Program GCAL and restore normal analog inputs.
    if not ADC_EnterConfig():
        return False

    for ch in range(ADC_CHANNELS):

        if not _config_write_cal24(
            ADC_GCAL_MSB_REG(ch),
            ADC_GCAL_LSB_REG(ch),
            gcal[ch]
        ):
            print("GCAL WRITE FAILED CH{}".format(ch))
            return False

        if not _config_write(
            ADC_CH_REG(ch),
            0x0000,
            verbose=False
        ):
            print("MUX RESTORE FAILED CH{}".format(ch))
            return False

    adc_gcal = gcal[:]

    if not ADC_EnterRun():
        return False

    print("ADC INTERNAL CALIBRATION OK")
    print("================================")
    print()

    return True


def ADC_CalibrateGND(count=200):
    """
    Compatibility alias. The new calibration performs both
    GND/offset and internal-reference gain calibration.
    """
    return ADC_CalibrateInternal(count)


ADC_REFERENCE = 1.1846
ADC_GAIN = 1.0
R_TOP = 68000.0
R_BOTTOM = 3000.0


def ADC_RawToVoltage(raw):
    full_scale = 8388607.0
    adc_voltage = raw * ADC_REFERENCE / (full_scale * ADC_GAIN)
    divider_ratio = 71.0 / 3.0
    return adc_voltage * divider_ratio
