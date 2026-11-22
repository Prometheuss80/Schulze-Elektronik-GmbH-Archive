NAME EEPROM_ASM

; XTOOLS ISL8 - EEPROM DATA-address diagnostic
; Address is supplied through fixed internal DATA byte 30h.
; No C parameter is passed to ASM.
; READ ONLY.

PUBLIC  EEPROM_READ_DATAADDR
PUBLIC  EEPROM_WRITETEST00_55AA
PUBLIC  EEPROM_WRITETEST00_FFFF
PUBLIC  EEPROM_WRITE_DATAADDR

?PR?EEPROM_READ_DATAADDR?EEPROM_ASM SEGMENT CODE
RSEG    ?PR?EEPROM_READ_DATAADDR?EEPROM_ASM

EEPROM_READ_DATAADDR:
        ; EEPROM inactive while synchronizing WDT
        CLR     097H            ; CS = 0
        CLR     096H            ; CLK = 0
        CLR     095H            ; DI = 0

        ; Proven W2B sequence: complete A_F961 structure, R7=2
        MOV     R7,#02H
WEDGEDA:
        MOV     C,094H
        CPL     C
        MOV     094H,C

        CLR     A
        MOV     R0,#049H
        MOV     @R0,A
DLY1DA:
        INC     @R0
        MOV     A,@R0
        CJNE    A,#083H,DLY1DA

        CLR     A
        MOV     R0,#049H
        MOV     @R0,A
DLY2DA:
        INC     @R0
        MOV     A,@R0
        CJNE    A,#083H,DLY2DA
        DJNZ    R7,WEDGEDA

        ; Original A_E66D releases PORT14 before EEPROM read.
        SETB    094H

        ; READ command, matching original helper sequence:
        ; A_F32C: CS=1, clock, DI=1
        SETB    097H
        SETB    096H
        CLR     096H
        SETB    095H

        ; A_E672: clock start bit
        SETB    096H
        CLR     096H

        ; A_F319: clock with DI=1, then DI=0 and another clock
        ; Together these transmit READ opcode '10'.
        SETB    096H
        CLR     096H
        CLR     095H
        SETB    096H
        CLR     096H

        ; Six address bits from fixed DATA byte 30h.
        ; Fully unrolled: no address loop and no C parameter/register handoff.

        ; b5
        MOV     A,030H
        ANL     A,#020H
        JZ      A5_ZERO
        SETB    095H
        SJMP    A5_CLK
A5_ZERO:
        CLR     095H
A5_CLK:
        SETB    096H
        CLR     096H

        ; b4
        MOV     A,030H
        ANL     A,#010H
        JZ      A4_ZERO
        SETB    095H
        SJMP    A4_CLK
A4_ZERO:
        CLR     095H
A4_CLK:
        SETB    096H
        CLR     096H

        ; b3
        MOV     A,030H
        ANL     A,#008H
        JZ      A3_ZERO
        SETB    095H
        SJMP    A3_CLK
A3_ZERO:
        CLR     095H
A3_CLK:
        SETB    096H
        CLR     096H

        ; b2
        MOV     A,030H
        ANL     A,#004H
        JZ      A2_ZERO
        SETB    095H
        SJMP    A2_CLK
A2_ZERO:
        CLR     095H
A2_CLK:
        SETB    096H
        CLR     096H

        ; b1
        MOV     A,030H
        ANL     A,#002H
        JZ      A1_ZERO
        SETB    095H
        SJMP    A1_CLK
A1_ZERO:
        CLR     095H
A1_CLK:
        SETB    096H
        CLR     096H

        ; b0
        MOV     A,030H
        ANL     A,#001H
        JZ      A0_ZERO
        SETB    095H
        SJMP    A0_CLK
A0_ZERO:
        CLR     095H
A0_CLK:
        SETB    096H
        CLR     096H
        CLR     095H

        ; Read 16 data bits MSB first into R7:R6.
        MOV     R7,#00H
        MOV     R6,#00H
        MOV     R5,#10H
READ_LOOPDA:
        SETB    096H
        CLR     096H

        ; 16-bit value <<= 1
        MOV     A,R6
        CLR     C
        RLC     A
        MOV     R6,A
        MOV     A,R7
        RLC     A
        MOV     R7,A

        ; Sample released P1.4 / EEPROM DO
        JNB     094H,NO_ONEDA
        ORL     06H,#01H        ; R6 bit0
NO_ONEDA:
        DJNZ    R5,READ_LOOPDA

        ; Original A_E66D termination
        CLR     097H            ; CS = 0
        SETB    096H
        CLR     096H
        RET



; ---------------------------------------------------------------------------
; WRITE TEST, WORD 00h ONLY
;
; Based on original Schulze sequence:
;   A_F00C  : write-enable + erase selected word
;   A_DD15  : WRITE command/address/data
;   A_F961  : watchdog service/delay after erase/write
;   A_F312/A_FDC3 termination helpers
;
; These two entry points are deliberately FIXED to address 00h.
; They are for first hardware validation only.
; ---------------------------------------------------------------------------

?PR?EEPROM_WRITE_TEST00?EEPROM_ASM SEGMENT CODE
RSEG    ?PR?EEPROM_WRITE_TEST00?EEPROM_ASM

; One EEPROM clock pulse
WT_CLK:
        SETB    096H
        CLR     096H
        RET

; Original A_F961 structure with R7 already loaded.
WT_WDT:
WT_WEDGE:
        MOV     C,094H
        CPL     C
        MOV     094H,C
        CLR     A
        MOV     R0,#049H
        MOV     @R0,A
WT_D1:
        INC     @R0
        MOV     A,@R0
        CJNE    A,#083H,WT_D1
        CLR     A
        MOV     R0,#049H
        MOV     @R0,A
WT_D2:
        INC     @R0
        MOV     A,@R0
        CJNE    A,#083H,WT_D2
        DJNZ    R7,WT_WEDGE
        RET

; Original A_FC4E command sequence (EWEN helper).
; IMPORTANT: original helpers A_FC3B/A_FC40/A_FC43/A_FC46 are
; contiguous and FALL THROUGH.  A call to A_FC3B therefore makes
; FIVE clocks, not one.  This was the bug in FIX1.
WT_EWEN:
        ; A_F312 = 3 clocks: idle/dummy, start=1, opcode bit=0
        SETB    097H
        LCALL   WT_CLK
        SETB    095H
        LCALL   WT_CLK
        CLR     095H
        LCALL   WT_CLK

        ; A_FC51 = second opcode bit 0
        LCALL   WT_CLK

        ; A_FC54 = address/control bit 1
        SETB    095H
        LCALL   WT_CLK

        ; A_FC59 calls A_FC3B.  Because FC3B falls through FC40,
        ; FC43 and FC46, this produces 5 clocks total. DI is high
        ; for the first clock, then low for the remaining four.
        LCALL   WT_CLK
        CLR     095H
        LCALL   WT_CLK
        LCALL   WT_CLK
        LCALL   WT_CLK
        LCALL   WT_CLK

        CLR     097H
        LCALL   WT_CLK
        RET

; Original A_F312 prefix.
WT_F312:
        SETB    097H
        LCALL   WT_CLK
        SETB    095H
        LCALL   WT_CLK
        CLR     095H
        LCALL   WT_CLK
        RET

; Exact expansion of original A_FDC3.
; A_FDC3 -> A_F32C -> A_FC3B -> A_FC40 -> CS=0 -> A_FC49.
; Again A_FC3B falls through, so it contributes FIVE clocks.
WT_FDC3:
        ; A_F32C
        SETB    097H
        LCALL   WT_CLK
        SETB    095H

        ; A_FC3B + fall-through FC40/FC43/FC46/inline FC49
        LCALL   WT_CLK
        CLR     095H
        LCALL   WT_CLK
        LCALL   WT_CLK
        LCALL   WT_CLK
        LCALL   WT_CLK

        ; A_FDCC
        CLR     097H
        LCALL   WT_CLK
        RET

; A_F00C equivalent for fixed address 00h:
; EWEN twice, then ERASE word 00h, WDT delay, termination.
WT_ERASE00_BEFORE_WRITE:
        LCALL   WT_EWEN
        LCALL   WT_EWEN

        ; Original A_F32C + A_FC43 = ERASE command prefix.
        SETB    097H
        LCALL   WT_CLK
        SETB    095H
        LCALL   WT_CLK
        LCALL   WT_CLK
        LCALL   WT_CLK

        ; address 00h = six zero bits
        CLR     095H
        LCALL   WT_CLK
        LCALL   WT_CLK
        LCALL   WT_CLK
        LCALL   WT_CLK
        LCALL   WT_CLK
        LCALL   WT_CLK

        CLR     097H
        CLR     095H
        MOV     R7,#0AH
        LCALL   WT_WDT

        ; Original A_F046..A_F049
        LCALL   WT_F312
        CLR     097H
        LCALL   WT_CLK
        RET

; Common WRITE command for fixed address 00h.
; R4:R5 = logical 16-bit word, MSB first.
WT_WRITE00_COMMON:
        ; Original A_DD25: A_F312 then DI=1 + clock -> WRITE prefix.
        LCALL   WT_F312
        SETB    095H
        LCALL   WT_CLK

        ; address 00h = six zero bits
        CLR     095H
        LCALL   WT_CLK
        LCALL   WT_CLK
        LCALL   WT_CLK
        LCALL   WT_CLK
        LCALL   WT_CLK
        LCALL   WT_CLK

        ; 16 data bits MSB first: R4 then R5.
        MOV     A,R4
        MOV     R6,#08H
WT_HI:
        MOV     C,ACC.7
        RL      A
        JC      WT_HI1
        CLR     095H
        SJMP    WT_HIC
WT_HI1:
        SETB    095H
WT_HIC:
        LCALL   WT_CLK
        DJNZ    R6,WT_HI

        MOV     A,R5
        MOV     R6,#08H
WT_LO:
        MOV     C,ACC.7
        RL      A
        JC      WT_LO1
        CLR     095H
        SJMP    WT_LOC
WT_LO1:
        SETB    095H
WT_LOC:
        LCALL   WT_CLK
        DJNZ    R6,WT_LO

        CLR     095H
        CLR     097H

        ; Original post-write WDT service/delay.
        MOV     R7,#0AH
        LCALL   WT_WDT

        ; Original A_DD8D..A_DD98 termination.
        LCALL   WT_F312
        CLR     097H
        LCALL   WT_CLK
        LCALL   WT_FDC3
        LCALL   WT_FDC3
        RET

EEPROM_WRITETEST00_55AA:
        CLR     097H
        CLR     096H
        CLR     095H
        LCALL   WT_ERASE00_BEFORE_WRITE
        MOV     R4,#055H
        MOV     R5,#0AAH
        LJMP    WT_WRITE00_COMMON

EEPROM_WRITETEST00_FFFF:
        CLR     097H
        CLR     096H
        CLR     095H
        LCALL   WT_ERASE00_BEFORE_WRITE
        MOV     R4,#0FFH
        MOV     R5,#0FFH
        LJMP    WT_WRITE00_COMMON

; Generic proven WRITE using DATA globals:
; 30h = word address 00..3F, 31h = logical MSB, 32h = logical LSB.
; Same sequence as the hardware-proven WORD00 test, only address/data vary.
EEPROM_WRITE_DATAADDR:
        CLR     097H
        CLR     096H
        CLR     095H

        ; A_F00C equivalent: EWEN twice, then erase selected word.
        LCALL   WT_EWEN
        LCALL   WT_EWEN

        ; ERASE prefix = A_F32C + A_FC43
        SETB    097H
        LCALL   WT_CLK
        SETB    095H
        LCALL   WT_CLK
        LCALL   WT_CLK
        LCALL   WT_CLK

        ; Six address bits, MSB first (20h..01h).
        MOV     A,030H
        MOV     R6,#020H
        MOV     R3,#06H
WT_EA_LOOP:
        MOV     B,A
        ANL     A,R6
        JZ      WT_EA_ZERO
        SETB    095H
        SJMP    WT_EA_CLOCK
WT_EA_ZERO:
        CLR     095H
WT_EA_CLOCK:
        LCALL   WT_CLK
        MOV     A,R6
        CLR     C
        RRC     A
        MOV     R6,A
        MOV     A,B
        DJNZ    R3,WT_EA_LOOP

        CLR     097H
        CLR     095H
        MOV     R7,#0AH
        LCALL   WT_WDT
        LCALL   WT_F312
        CLR     097H
        LCALL   WT_CLK

        ; WRITE prefix.
        LCALL   WT_F312
        SETB    095H
        LCALL   WT_CLK

        ; Six address bits again.
        MOV     A,030H
        MOV     R6,#020H
        MOV     R3,#06H
WT_WA_LOOP:
        MOV     B,A
        ANL     A,R6
        JZ      WT_WA_ZERO
        SETB    095H
        SJMP    WT_WA_CLOCK
WT_WA_ZERO:
        CLR     095H
WT_WA_CLOCK:
        LCALL   WT_CLK
        MOV     A,R6
        CLR     C
        RRC     A
        MOV     R6,A
        MOV     A,B
        DJNZ    R3,WT_WA_LOOP

        ; 16 logical data bits, MSB first.
        MOV     R4,031H
        MOV     R5,032H
        MOV     A,R4
        MOV     R6,#08H
WT_GHI:
        MOV     C,ACC.7
        RL      A
        JC      WT_GHI1
        CLR     095H
        SJMP    WT_GHIC
WT_GHI1:
        SETB    095H
WT_GHIC:
        LCALL   WT_CLK
        DJNZ    R6,WT_GHI

        MOV     A,R5
        MOV     R6,#08H
WT_GLO:
        MOV     C,ACC.7
        RL      A
        JC      WT_GLO1
        CLR     095H
        SJMP    WT_GLOC
WT_GLO1:
        SETB    095H
WT_GLOC:
        LCALL   WT_CLK
        DJNZ    R6,WT_GLO

        CLR     095H
        CLR     097H
        MOV     R7,#0AH
        LCALL   WT_WDT
        LCALL   WT_F312
        CLR     097H
        LCALL   WT_CLK
        LCALL   WT_FDC3
        LCALL   WT_FDC3
        RET


END
