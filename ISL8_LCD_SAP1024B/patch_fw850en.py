#!/usr/bin/env python3
"""Patch an owner-provided ORIGINAL Schulze ISL8-936G firmware 8.50 EN.

Copyright (c) 2026 contributors. Patch logic only, no original firmware included.
Refuses all input ROMs other than the specifically verified factory image.
"""
import argparse
import hashlib
from pathlib import Path

EXPECTED_SHA256 = "f9d461cb2541e4584cf9811e5b68850b6dd13b36725339e09f433699b96aa5ed"
EXPECTED_OUTPUT_SHA256 = "74a0654397cda82d4a91ed2c82aeb753bf158293ed8c8de81501636dd84a8020"
ROM_SIZE = 0x10000

def build_driver():
    base=0x1e; code=bytearray();labels={};fix=[]
    def mark(s):
     if s in labels: raise ValueError(s)
     labels[s]=base+len(code)
    def emit(*v): code.extend(v)
    def movri(r,v): emit(0x78+r,v)
    def lcall(s):fix.append((len(code)+1,s,'abs'));emit(0x12,0,0)
    def ljmp(s):fix.append((len(code)+1,s,'abs'));emit(0x02,0,0)
    def rel(op,s):emit(op,0);fix.append((len(code)-1,s,'rel'))
    def orp(v):emit(0x43,0xfa,v)
    def anp(v):emit(0x53,0xfa,v)
    def movp(v):emit(0x75,0xf8,v)
    def data(v):movri(7,v);lcall('sap_data')
    def cmd(v):movri(7,v);lcall('sap_cmd')
    def cmd2(c,l,h):data(l);data(h);cmd(c)
    mark('SAP_ENTRY')
    emit(0xc0,0xe0,0xc0,0xf0,0xc0,0x82,0xc0,0x83,0xc0,0xd0)
    for i in range(8):emit(0xc0,i)
    orp(0x0f);anp(0xef)
    movri(2,255)
    mark('reset1');emit(0xb2,0x94,0,0,0);rel(0xda,'reset1')
    orp(0x10);movri(2,255)
    mark('reset2');emit(0xb2,0x94,0,0,0);rel(0xda,'reset2')
    cmd(0x90);cmd2(0x40,0,8);cmd2(0x41,30,0);cmd2(0x42,0,0);cmd2(0x43,30,0);cmd(0x80);cmd2(0x24,0,0);cmd(0x98)
    movri(4,64)
    mark('clear_row');movri(5,30)
    mark('clear_column');data(0);cmd(0xc0)
    rel(0xdd,'clear_column');rel(0xdc,'clear_row')
    cmd2(0x24,0,0)
    emit(0x75,0x46,0,0x75,0x47,0x77,0xd2,0x48,0xd2,0x0a)
    for i in range(7,-1,-1):emit(0xd0,i)
    emit(0xd0,0xd0,0xd0,0x83,0xd0,0x82,0xd0,0xf0,0xd0,0xe0,0x22)
    def save_regs():
     for direct in (0xe0,0xd0,0xf0,0x82,0x83,*range(8)):emit(0xc0,direct)
     emit(0x53,0xd0,0xe7)
    def restore_regs():
     for direct in (*range(7,-1,-1),0x83,0x82,0xf0,0xd0,0xe0):emit(0xd0,direct)
     emit(0x22)
    mark('SAP_SET_ADDRESS');save_regs()
    emit(0xa9,0x57,0xaa,0x56,0x20,0x48,0);fix.append((len(code)-1,'offset','rel'))
    movri(4,0);movri(5,0)
    mark('mode40_loop');emit(0xea);rel(0x70,'mode40_sub');emit(0xe9,0xc3,0x94,0x28);rel(0x40,'mode40_done')
    mark('mode40_sub');emit(0xe9,0xc3,0x94,0x28,0xf9,0xea,0x94,0,0xfa,0xec,0x24,0x1e,0xfc,0xed,0x34,0,0xfd);ljmp('mode40_loop')
    mark('mode40_done');emit(0xec,0x29,0xf9,0xed,0x34,0,0xfa);ljmp('address_ready')
    mark('offset');emit(0xe9,0xc3,0x94,0x40,0xf9,0xea,0x94,0x01,0xfa)
    mark('address_ready');emit(0xe9,0xfc,0xea,0xfd,0xec,0xff);lcall('sap_data');emit(0xed,0xff);lcall('sap_data');cmd(0x24);restore_regs()
    mark('SAP_WRITE_GRAPHIC');save_regs();emit(0xef,0xfe);movri(7,0);movri(5,8)
    mark('reverse_loop');emit(0xc3,0xee,0x13,0xfe,0xef,0x33,0xff);rel(0xdd,'reverse_loop');lcall('sap_data');cmd(0xc0);restore_regs()
    # PASS10F: First clear graphic RAM quickly. Then let the ORIGINAL Schulze
    # A_F770 run all its 8x40-space handling for internal cursor/state updates.
    # Only the physical LCD write of a space from that call context is skipped.
    mark('SAP_SWITCH_MODE');emit(0x75,0x3b,0,0x75,0x3c,0x75,0xc2,0x48);lcall('SAP_CLEAR');emit(0x12,0xf7,0x70,0x22)
    # PASS10C: optimize ONLY the clear called during transition into graphic mode.
    # Preserve init/startup clear as in PASS10B, so performance/compatibility can be compared.
    mark('SAP_CLEAR');save_regs();cmd2(0x24,0,0);cmd(0xb0);movri(4,64)
    mark('clear2_row');movri(5,30)
    mark('clear2_col');movri(7,0);lcall('sap_auto_data');rel(0xdd,'clear2_col');rel(0xdc,'clear2_row')
    # B2 must be issued in auto mode after STA3=1, not through normal sap_cmd.
    movri(7,0xb2);lcall('sap_auto_cmd');restore_regs()
    mark('SAP_CLEAR_BIT');save_regs();emit(0xef,0x54,0x07,0x64,0x07,0x44,0xf0,0xff);lcall('sap_cmd');restore_regs()
    mark('SAP_SET_BIT');save_regs();emit(0xef,0x54,0x07,0x64,0x07,0x44,0xf8,0xff);lcall('sap_cmd');restore_regs()
    # Fast text renderer, same glyph mapping as PASS9. Read-modify-write entire byte(s) using C5/C4.
    mark('SAP_TEXT_WRITE');save_regs()
    # Only skip a SPACE character (0x20) coming from the A_F770 clear loop.
    # A_F770 executes LCALL A_EB25 at F78Bh; return address F78Eh is on stack.
    # A_EB25 calls SAP_TEXT_WRITE via the patched LCALL at EB70h.
    # On SAP_TEXT_WRITE entry the stack is [..., F78E, EB73]. save_regs()
    # pushes 13 bytes, so SP-15 is high(F78E)=F7 and SP-16 is low=8E.
    # This avoids allocating any IRAM/XRAM flag and does not affect normal spaces.
    emit(0xbf,0x20,0);fix.append((len(code)-1,'text_normal_draw','rel')) # CJNE R7,#20h
    emit(0xe5,0x81,0xc3,0x94,0x0f,0xf8) # MOV A,SP; SUBB #15; MOV R0,A
    emit(0xe6,0xb4,0xf7,0);fix.append((len(code)-1,'text_normal_draw','rel'))
    emit(0x18,0xe6,0xb4,0x8e,0);fix.append((len(code)-1,'text_normal_draw','rel'))
    ljmp('text_exit')
    mark('text_normal_draw')
    for special in (0xf5,0xef,0xdf,0xe1,0x26,0x7e,0x7f):
     emit(0xef,0xb4,special,0);fix.append((len(code)-1,'not_special_%02x'%special,'rel'))
     emit(0x90,0,0);fix.append((len(code)-2,'glyph_%02x'%special,'abs'))
     ljmp('text_font_ready');mark('not_special_%02x'%special)
    emit(0xef,0xc3,0x94,0x20);rel(0x50,'text_char_ok1');ljmp('text_exit');mark('text_char_ok1')
    emit(0xc3,0x94,0x60);rel(0x40,'text_char_ok2');ljmp('text_exit');mark('text_char_ok2')
    emit(0x24,0x60,0x75,0xf0,0x05,0xa4,0x24,0xd9,0xf5,0x82,0xe5,0xf0,0x34,0x72,0xf5,0x83)
    mark('text_font_ready')
    emit(0xa9,0x57,0xaa,0x56,0xe5,0x45,0x14,0x29,0xf9,0xea,0x34,0,0xfa)
    movri(3,0)
    mark('text_div40');emit(0xea);rel(0x70,'text_sub40');emit(0xe9,0xc3,0x94,0x28);rel(0x40,'text_div_done')
    mark('text_sub40');emit(0xe9,0xc3,0x94,0x28,0xf9,0xea,0x94,0,0xfa,0x0b);ljmp('text_div40')
    mark('text_div_done');emit(0xe9,0xfc,0xeb,0xc3,0x94,0x08);rel(0x40,'text_row_ok');ljmp('text_exit');mark('text_row_ok')
    # PASS10D FIX2: calculate the graphic base address once, keep offset in B during font rendering.
    # R3=graphic address high, R4=graphic address low, R1=horizontal bit offset 0/2/4/6.
    # Previous: R3=text row, R4=text col.
    # A=(textcol*6)/8, B=(textcol*6)%8; all text cols 0..39 fit in 8 bits.
    emit(0xec,0x75,0xf0,0x06,0xa4,0x75,0xf0,0x08,0x84,0xa9,0xf0,0xfc)
    # A=row*240 low, B=high; add byte index (R4), preserving carry.
    emit(0xeb,0x75,0xf0,0xf0,0xa4,0x2c,0xfc,0xe5,0xf0,0x34,0,0xfb)
    emit(0x89,0xf0) # MOV B,R1: save constant x offset before R1 is used for ROM glyph bytes
    movri(6,0) # Y
    mark('text_y_loop')
    # Build six-bit glyph row pattern (bit5=leftmost, bit0=rightmost blank).
    movri(5,0);movri(0,0)
    mark('text_pattern_loop')
    emit(0xed,0x23,0xfd) # A=R5; RL A; R5=A
    emit(0xe8,0x93,0xf9) # A=R0; MOVC A,@A+DPTR; MOV R1,A: remember glyph column
    emit(0xee);rel(0x60,'text_no_shift')
    emit(0xee,0xfa,0xe9) # MOV A,R6(Y); MOV R2,A; MOV A,R1(font column)
    mark('text_shift_font');emit(0xc3,0x13);rel(0xda,'text_shift_font')
    ljmp('text_col_bit')
    mark('text_no_shift');emit(0xe9) # MOV A,R1: zero-shift path still needs font column
    mark('text_col_bit');emit(0x54,0x01,0x4d,0xfd) # ANL #1 ; ORL A,R5 ; R5=A
    emit(0x08,0xe8,0xc3,0x94,0x05);rel(0x40,'text_pattern_loop')
    emit(0xed,0x23,0xfd) # sixth zero column
    # Preserve ROM font DPTR across graphic-RAM addressing for all eight glyph scanlines.
    emit(0xc0,0x82,0xc0,0x83)  # PUSH DPL; PUSH DPH
    # Save R3(text row), R4(text col), R6(y) on stack.
    emit(0xc0,0x03,0xc0,0x04,0xc0,0x06)
    # DPTR = address of current glyph scanline.
    # Do NOT increment R3:R4 while saved copies are still on the stack;
    # POP would restore the previous address and erase the increment.
    emit(0x8c,0x82,0x8b,0x83)
    # Shift 16-bit pixel data and mask left by (10 - fixed xoffset).
    emit(0x74,0x0a,0xc3,0x95,0xf0,0xfe) # A=10-B(xoffset); R6=count
    emit(0x7b,0,0x8d,0x04,0x7f,0,0x78,0x3f) # R3=data_hi0, R4=data_lo=pattern, R7=mask_hi0,R0=mask_lo3F
    mark('text_align')
    emit(0xec,0xc3,0x33,0xfc,0xeb,0x33,0xfb) # shift data lo/hi
    emit(0xe8,0xc3,0x33,0xf8,0xef,0x33,0xff) # shift mask lo/hi
    rel(0xde,'text_align') # DJNZ R6
    # byte1 if mask_hi !=0 (always), write using saved values; DPTR first byte.
    emit(0xc0,0x00,0xc0,0x04) # PUSH R0(mask_lo); PUSH R4(data_lo) for second byte
    emit(0x8b,0x05) # MOV R5,R3 (datahi)
    # R7 maskhi already
    lcall('sap_rmw')
    emit(0xd0,0x05,0xd0,0x07) # POP R5(data_lo), POP R7(mask_lo)
    emit(0xef);rel(0x60,'text_skip_second') # if mask2==0 skip second
    emit(0xa3) # INC DPTR
    lcall('sap_rmw')
    mark('text_skip_second')
    emit(0xd0,0x06,0xd0,0x04,0xd0,0x03) # restore pixel Y, cached address low and high
    # Advance the restored cached graphic address to the NEXT scanline.
    emit(0xec,0x24,0x1e,0xfc,0xeb,0x34,0,0xfb)
    emit(0xd0,0x83,0xd0,0x82) # POP DPH; POP DPL: restore ROM font pointer
    emit(0x0e,0xee,0xc3,0x94,0x08);rel(0x50,'text_y_done');ljmp('text_y_loop')
    mark('text_y_done');mark('text_exit');restore_regs()
    # RMW: DPTR = graphic address, R5 = desired pixel bits, R7 = mask. Preserve DPTR and other outer state.
    mark('sap_rmw')
    emit(0xc0,0x07,0xc0,0x05) # mask, data
    emit(0xaf,0x82);lcall('sap_data')
    emit(0xaf,0x83);lcall('sap_data')
    cmd(0x24);cmd(0xc5)
    lcall('sap_read_byte') # A=old bitmap byte, CY=1 on timeout
    rel(0x40,'sap_rmw_abort') # JC: do not write if LCD read failed
    emit(0xd0,0x05,0xd0,0x07) # restore data/mask
    emit(0xfe,0xef,0xf4,0x5e,0x4d,0xff) # MOV R6,A (old), MOV A,R7; CPL A; ANL A,R6; ORL A,R5; MOV R7,A
    lcall('sap_data');cmd(0xc4) # write, address unchanged
    emit(0x22)
    mark('sap_rmw_abort');emit(0xd0,0x05,0xd0,0x07,0x22) # balance stack, preserve LCD byte
    # Read data from SAP1024B: bus C/D=0,/RD=0,/CE=0. status STA0+STA1 checked first.
    mark('sap_read_byte')
    movri(2,255)
    mark('read_poll')
    emit(0xb2,0x94);orp(0x0f);movp(0xff);emit(0,0,0,0);anp(0xf5);emit(0,0,0,0,0)
    emit(0xe5,0xf8);orp(0x0a);movp(0);emit(0x54,0x03,0xb4,0x03,0)
    fix.append((len(code)-1,'read_not_ready','rel'))
    # data read
    orp(0x0b);anp(0xfb);movp(0xff);anp(0xf5);emit(0,0,0,0,0)
    emit(0xe5,0xf8);orp(0x0a);movp(0);emit(0xc3,0x22) # CLR C means read success
    mark('read_not_ready');rel(0xda,'read_poll')
    # Timeout: return CY=1, caller must not write
    emit(0xd3,0x22)
    # Auto Write mode uses STA3 (D3). Keep the same bus pulses as PASS10B.
    # R6=0 for data, R6=1 for auto reset command B2h.
    mark('sap_auto_data');movri(6,0);ljmp('sap_auto_send')
    mark('sap_auto_cmd');movri(6,1)
    mark('sap_auto_send');movri(2,255)
    mark('auto_poll');emit(0xb2,0x94);orp(0x0f);movp(0xff);emit(0,0,0,0);anp(0xf5);emit(0,0,0,0,0)
    emit(0xe5,0xf8);orp(0x0a);movp(0);emit(0x54,0x08);rel(0x60,'auto_not_ready')
    ljmp('send_ready')
    mark('auto_not_ready');rel(0xda,'auto_poll')
    # If auto mode never becomes ready, fail closed (skip this byte) and avoid a hang.
    emit(0x22)
    mark('sap_data');movri(6,0);ljmp('send')
    mark('sap_cmd');movri(6,1)
    mark('send');movri(2,255)
    mark('poll');emit(0xb2,0x94);orp(0x0f);movp(0xff);emit(0,0,0,0);anp(0xf5);emit(0,0,0,0,0)
    emit(0xe5,0xf8);orp(0x0a);movp(0);emit(0x54,0x03,0xb4,0x03,0);fix.append((len(code)-1,'not_ready','rel'))
    mark('send_ready');emit(0xee);rel(0x60,'is_data');orp(0x04);ljmp('do_write')
    mark('is_data');anp(0xfb)
    mark('do_write');orp(0x0b);emit(0x8f,0xf8,0,0,0,0,0);anp(0xf6);emit(0,0,0,0,0);orp(0x09);movp(0);emit(0x22)
    mark('not_ready');rel(0xda,'poll');mark('timeout');emit(0x22)
    for name,bitmap in (
     ('glyph_f5',(0x08,0x1c,0x3e,0x08,0x08)),('glyph_ef',(0x08,0x08,0x3e,0x1c,0x08)),
     ('glyph_df',(0x08,0x14,0x22,0x14,0x08)),('glyph_e1',(0x08,0x1c,0x3e,0x7f,0x00)),
     ('glyph_26',(0x36,0x49,0x55,0x22,0x50)),('glyph_7e',(0x08,0x08,0x2a,0x1c,0x08)),
     ('glyph_7f',(0x08,0x1c,0x2a,0x08,0x08))):mark(name);emit(*bitmap)
    for p,s,typ in fix:
     if typ=='abs':
      a=labels[s];code[p:p+2]=bytes((a>>8,a&255))
     else:
      dist=labels[s]-(base+p+1)
      if not -128<=dist<=127: raise ValueError((s,dist,hex(base+p)))
      code[p]=dist&255
    assert len(code)<0x08cf-base,(len(code),0x08cf-base)
    bytestream=bytes(code)+bytes(0x08cf-base-len(code))+b'\x02\x00\x00'
    assert len(bytestream)==0x08d2-base

    return bytes(bytestream), labels

def _expect(rom, address, expected):
    actual = bytes(rom[address:address + len(expected)])
    if actual != expected:
        raise ValueError(f"Unexpected bytes at 0x{address:04X}: got {actual.hex(' ')}, expected {expected.hex(' ')}")


def patch_rom(original):
    if len(original) != ROM_SIZE:
        raise ValueError(f"Expected 65536 bytes, got {len(original)}")
    sha = hashlib.sha256(original).hexdigest()
    if sha != EXPECTED_SHA256:
        raise ValueError("Unsupported original firmware image. Only exact 8.50 EN factory ROM accepted.\n"
                         f"Expected SHA-256: {EXPECTED_SHA256}\nFound SHA-256:    {sha}")
    rom = bytearray(original)
    driver, labels = build_driver()
    assert len(driver) == 0x08D2 - 0x001E
    assert driver[-3:] == b"\x02\x00\x00"  # safety LJMP reset at 08CF

    expected = {
        0xBAF7: '12 00 1e',  # original factory calibration entry
        0xFD96: '12 fe d5',  # original LCD initialization
        0xFDA5: '12 b9 d9',
        0xF943: '12 b9 d9',
        0xF9BA: '12 b9 d9',
        0xF9D7: '12 b9 d9',
        0xE336: '12 fd a5',
        0xF925: '12 fd a5',
        0xEB70: '12 f9 25',
        0xEEB3: '78 3b 76 00',
        0xF770: '12 fe df',
        0xFB6F: '12 fd a5',
    }
    for address, hexstr in expected.items():
        _expect(rom, address, bytes.fromhex(hexstr))

    rom[0x001E:0x08D2] = driver  # calibration ROM area replaced with independent display driver
    rom[0xBAF7:0xBAFA] = b'\x00\x00\x00'  # disable call into factory calibration
    rom[0xFD96:0xFD99] = b'\x02\x00\x1E'  # init jumps to new driver
    for address in (0xFDA5, 0xF943):
        rom[address:address + 3] = b'\x22\x00\x00'  # old LCD busy routines disabled
    for address, routine in (
        (0xF9BA, 'SAP_CLEAR_BIT'),
        (0xF9D7, 'SAP_SET_BIT'),
        (0xE336, 'SAP_SET_ADDRESS'),
        (0xF925, 'SAP_WRITE_GRAPHIC'),
    ):
        target = labels[routine]
        rom[address:address + 3] = bytes((0x02, target >> 8, target & 0xFF))
    target = labels['SAP_TEXT_WRITE']
    rom[0xEB70:0xEB73] = bytes((0x12, target >> 8, target & 0xFF))
    target = labels['SAP_SWITCH_MODE']
    rom[0xEEB3:0xEEB7] = bytes((0x02, target >> 8, target & 0xFF, 0x00))
    rom[0xF770:0xF773] = b'\x00\x00\x00'
    rom[0xFB6F:0xFB72] = b'\x22\x00\x00'
    result = bytes(rom)
    sha = hashlib.sha256(result).hexdigest()
    if sha != EXPECTED_OUTPUT_SHA256:
        raise RuntimeError('Output digest mismatch; aborting. ' + sha)
    return result


def to_intel_hex(payload):
    lines = []
    for address in range(0, len(payload), 16):
        chunk = payload[address:address + 16]
        record = bytes([len(chunk), address >> 8 & 255, address & 255, 0]) + chunk
        checksum = (-sum(record)) & 0xFF
        lines.append(':' + (record + bytes([checksum])).hex().upper())
    lines.append(':00000001FF')
    return '\n'.join(lines) + '\n'


def main():
    parser = argparse.ArgumentParser(description='Schulze 8.50 EN SAP1024B graphic LCD driver patch (PASS 10F)')
    parser.add_argument('input', nargs='?', type=Path, default=None,
                        help='Your original unmodified fw850en.bin (optional; defaults to file next to script)')
    parser.add_argument('-o', '--output', type=Path, default=None, help='Output .bin path')
    parser.add_argument('--hex', action='store_true', help='Also save an Intel HEX image next to the BIN')
    args = parser.parse_args()
    # IDE-friendly: pressing Run without arguments uses the ROM beside this script.
    source = (args.input if args.input is not None else Path(__file__).resolve().parent / 'fw850en.bin').resolve()
    output = (args.output or source.with_name('fw850en_sap1024b.bin')).resolve()
    if output == source:
        parser.error('Output must differ from input (the original ROM will never be overwritten).')
    if not source.is_file():
        parser.exit(2, f'ERROR: Input ROM not found: {source}\n'
                       'Place your original fw850en.bin next to patch_fw850en.py, or pass its path as an argument.\n')
    try:
        rom = patch_rom(source.read_bytes())
    except (ValueError, RuntimeError, OSError) as error:
        parser.exit(2, f'ERROR: {error}\n')
    output.write_bytes(rom)
    # With no CLI input argument, generate both formats by default (IDE Run).
    if args.hex or args.input is None:
        output.with_suffix('.hex').write_text(to_intel_hex(rom), encoding='ascii')
    print(f'Patched {source.name} -> {output.name}')
    print(f'SHA-256: {hashlib.sha256(rom).hexdigest()}')
    print('Factory calibration entry disabled (0xBAF7); calibration code replaced (0x001E–0x08D1).')


if __name__ == '__main__':
    main()
