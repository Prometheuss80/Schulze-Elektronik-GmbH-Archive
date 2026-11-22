; XTOOLS for ISL8 - independently written reset entry for 80C537.
; Copyright (c) 2026 XTOOLS project contributors.
; Purpose: preserve the established internal-RAM reset policy and enter
; the Keil C51 runtime initializer. No external RAM or Timekeeper is erased.
; Hardware initialization is intentionally performed later by HW_Init().
;
$NOMOD51
                NAME    XTOOLS_BOOT

?XTOOLS_BOOT    SEGMENT CODE
?STACK          SEGMENT IDATA

                RSEG    ?STACK
                DS      1

                EXTRN   CODE (?C_START)
                PUBLIC  ?C_STARTUP

                CSEG    AT 0
?C_STARTUP:     LJMP    XTOOLS_RESET

                RSEG    ?XTOOLS_BOOT
XTOOLS_RESET:
                CLR     0AFH       ; EA bit (IEN0.7), interrupts disabled
                MOV     R0,#07FH   ; clear IDATA 00h through 7Fh
                CLR     A
XTOOLS_CLR:     MOV     @R0,A
                DJNZ    R0,XTOOLS_CLR
                MOV     @R0,A      ; include address 00h
                MOV     SP,#?STACK-1
                LJMP    ?C_START   ; C51 initialization then main()
                END
