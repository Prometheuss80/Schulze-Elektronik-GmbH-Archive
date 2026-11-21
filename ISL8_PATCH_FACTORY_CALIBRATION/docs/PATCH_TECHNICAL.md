# Technical documentation — factory calibration entry patch

This document consolidates the project's original Keil A51 notes from `PATCH_850EN.txt`, `PATCH_850DE(1).txt`, and the broader 8.50 EN flow trace `PATCH_CALIB.txt`. The snippets below describe the *same algorithm* in EN and DE variants but **addresses are not interchangeable**. The patcher uses each image's independently verified signature.

## Where the original decision occurs

In original 8.50 EN the decision begins at `BAA7h` (in 8.50 DE at `BABAh`). Its original assembly includes:

```asm
    JNB 22h, SKIP_CALIBRATION   ; original activation condition
    JB  51h, SKIP_CALIBRATION   ; existing state gate
    MOV R0,#68h                ; check three RAM bytes
    MOV A,@R0
    JZ SKIP_CALIBRATION
    INC R0
    MOV A,@R0
    JZ SKIP_CALIBRATION
    INC R0
    MOV A,@R0
    JZ SKIP_CALIBRATION
    JNB 50h, SKIP_CALIBRATION
    SETB 51h
    CLR 37h
    MOV R0,#5Bh
    LCALL ORIGINAL_TARGET      ; target differs by firmware version
    ; NOP padding follows
```

This is a **simplified symbolic transcription** of the original notes: `SKIP_CALIBRATION` and `ORIGINAL_TARGET` are descriptive placeholders, not labels in the firmware. The original `LCALL` target is `3BD5h` in 8.50 EN and `3BD8h` in 8.50 DE; in 8.10 EN/DE it is `3BDEh`/`3BE1h`.

## Replacement decision (same 35 bytes)

```asm
    JNB 14h, SKIP_CALIBRATION  ; F2 instead of original 22h
    SETB 50h
    SETB 51h
    MOV R0,#68h
    MOV @R0,#01h
    INC R0
    MOV @R0,#01h
    INC R0
    MOV @R0,#01h
    INC R0
    JNB 50h, SKIP_CALIBRATION
    SETB 51h
    CLR 37h
    MOV R0,#5Bh
    LCALL ORIGINAL_TARGET      ; original absolute 16-bit address retained
    NOP
    NOP
    NOP
    NOP
    NOP
```

`JNB` is a relative branch; the new relative byte value in the assembled patch is `38h` for the first jump, and `26h` for the later `JNB`. The 35-byte replacement uses the **same ROM footprint** and preserves the original branch destination. The `LCALL` uses opcode `12h` plus a **16-bit absolute target**, *not* a displacement calculated from the patch address. The tool copies its two bytes from the original code. The entire routine following the 35-byte block (including its `LCALL` and `SETB 5Dh`) remains unchanged.

## What happens after the decision

The 8.50 EN flow trace in the original notes shows an existing calibration entry via `LCALL A_001E` after the control flow selects the appropriate state. The patch only provides the required input/key and RAM-state conditions for the original firmware's existing procedure. It does **not** insert XTOOLS code or modify that factory-calibration implementation.

## Variant addresses

| Firmware | Start | End of replacement (inclusive) | Original `LCALL` | Code immediately after replacement |
|---|---|---|---|---|
| 8.10 DE | `B9B5h` | `B9D7h` | `3BE1h` | `LCALL` at `B9D8h` |
| 8.10 EN | `B9A2h` | `B9C4h` | `3BDEh` | `LCALL` at `B9C5h` |
| 8.50 DE | `BABAh` | `BADCh` | `3BD8h` | `LCALL` at `BADDh` |
| 8.50 EN | `BAA7h` | `BAC9h` | `3BD5h` | `LCALL` at `BACAh` |

The older handwritten 8.50 DE notes contain a label `A_BAA7` in their *patch source* subsection even though the Keil listing and actual DE ROM place it at **`BABAh`**. The binary address confirmed by listing and ROM is authoritative. The patcher does not depend on manually entered labels.

## LCD version and date/time

The version field is zero terminated; changing the fourth ASCII character (`0` to `9`) preserves string length and terminator. The date/time values are **static firmware display strings**, not the actual real-time clock. Both the edit-field value and associated explanatory text are patched; the original weekday language marker remains unchanged (`we` EN, `Mi` DE).

See [HEX_CHANGES.md](HEX_CHANGES.md) for exact per-version byte comparisons.
