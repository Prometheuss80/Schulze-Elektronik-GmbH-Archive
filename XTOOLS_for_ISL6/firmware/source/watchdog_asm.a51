$NOMOD51

NAME    WDTASM

; C51 may decorate a register-parameter function name with "_".
; Export both spellings as aliases so either convention links.
PUBLIC  _WDT_Kick
PUBLIC  WDT_Kick

?PR?WDT_KICK?WDTASM SEGMENT CODE

RSEG    ?PR?WDT_KICK?WDTASM
USING   0

_WDT_Kick:
WDT_Kick:

        ; SAB80C535 internal watchdog refresh.
        ; THESE TWO INSTRUCTIONS MUST BE IMMEDIATELY CONSECUTIVE.
        SETB    0AEH            ; IEN0.6 = WDT
        SETB    0BEH            ; IEN1.6 = SWDT

        ; External ADM706 WDI on P1.4 / bit address 94h.
        ; Exact logic used by the original ISL6 firmware.
        MOV     C,094H
        CPL     C
        MOV     094H,C

        RET

END
