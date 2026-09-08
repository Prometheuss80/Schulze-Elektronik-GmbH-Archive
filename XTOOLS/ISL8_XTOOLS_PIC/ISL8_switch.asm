;========================================================================================
; Switch EPROM pro ISL8-936G                                            27.6.2025
; PIC12F509, freq 4.0MHz                                     
;========================================================================================
;                           http://www.spirit-pro.com                        
;========================================================================================
;                                  PIC12F509                              
;                             --------\/--------                         
;                       +5V  -|VSS          VDD|-  0V                     
;           GPI5 /     EPROM -|                |-  LED_GREEN  / GPI0                   
;           GPI4 /      FREE -|                |-  LED_RED    / GPI1           
;           GPI3 / SWITCH_IN -|                |-  RESET_ISL8 / GPI2                          
;                             ------------------                         
;                                                                      Martenss (c) 2025
;========================================================================================
; Popis funkce programu a zarizeni
;========================================================================================
; 
; Po stisku tlacitka provede reset 600ms a prepne OE pro EPROM
;
;========================================================================================
;****************************************************************************************

;  		IFNDEF __12F683
;        		MESSG "Processor-header file mismatch.  Verify selected processor."
;      		ENDIF

		IFNDEF __12F509
          		MESSG "Processor-header file mismatch.  Verify selected processor."
        		ENDIF

		LIST P=12F509, R=DEC	;Direktivy pøekladaèe
		INCLUDE "p12F509.inc"	;Definice názvù registrù

;		ERRORLEVEL      -302    	;SUPPRESS BANK SELECTION MESSAGES

;========================================================================================
; Promenne a definice PIC
;========================================================================================

;RAM		EQU	20h		; zacatek pamìti RAM pro PIC16F84A - 68bajtù (0C-2F)
RAM		EQU	10h		; zacatek pamìti RAM pro PIC16F509

TMP0		EQU	RAM+1		; cekaci smycka
TMP1		EQU	RAM+2		; cekaci smycka
TMP2		EQU	RAM+3		; cekaci smycka
TMP3		EQU	RAM+4		; cekaci smycka
TMP4		EQU	RAM+5		; cekaci smycka
TMP5		EQU	RAM+6		; cekaci smycka

#define		GREEN	GPIO, GP0
#define		RED	GPIO, GP1
#define		RESET_ISL8  GPIO, GP2
#define		SWITCH	GPIO, GP3
#define		EPROM	GPIO, GP5

;DELETE
					
;========================================================================================
; Konfigurace PIC
;========================================================================================
		
;		org	2007h
		
;__CONFIG _INTRC_OSC_CLKOUT & _WDT_OFF & _CP_OFF & _MCLRE_OFF & _PWRTE_ON & _BOREN_OFF & _IESO_OFF & _FCMEN_OFF 
	
		org         H'FFF'
 __CONFIG _WDT_OFF & _CP_OFF & _MCLRE_OFF & _OSC_IntRC

	            

				
;========================================================================================
; Zacatek programu
;========================================================================================

		org	.0		; zacatek programu

		goto	INIT_PIC		; skok na inicializaci		

;****************************************************************************************
; Hlavni program PIC
;****************************************************************************************
	
		org	.6

MAIN_PIC		BTFSS 	SWITCH		 ; testuj jestli je LOW (nepøeskoèí pokud LOW)
		GOTO	SWITCH_EPROM
		GOTO  	MAIN_PIC

SWITCH_EPROM	BSF 	RESET_ISL8
		CALL	CEK200m
		MOVF 	GPIO, W
		XORLW 	b'00100011'		 ; toggluj EPROM, RED, GREEN
		MOVWF 	GPIO
		CALL	CEK200m
		BCF	RESET_ISL8
		GOTO	MAIN_PIC
			
;----------------------------------------------------------------------------------------

;========================================================================================
; Inicializace procesoru
;========================================================================================

INIT_PIC		NOP
;		BANKSEL 	OPTION_REG
		MOVLW 	b'00000000'		;
		OPTION
;		MOVWF	OPTION_REG 
;		BANKSEL	OSCCON
;		MOVLW 	b'01100000' 	; 4 MHz
;		MOVWF 	OSCCON
;		BANKSEL	INTCON
;		MOVLW 	b'00000000'		;
;		MOVWF	INTCON
;		BANKSEL	PIE1
;		MOVLW 	b'00000000'		;
;		MOVWF	PIE1
;		BANKSEL	PIR1
;		MOVLW 	b'00000000'		;
;		MOVWF	PIR1
;		BANKSEL	PCON
;		MOVLW 	b'00000000'		;
;		MOVWF	PCON
;		BANKSEL	CMCON0		
;		MOVLW 	07h 		;Set GP<2:0> to
;		MOVWF 	CMCON0 		;digital I/O
;		BANKSEL 	ANSEL 		;
;		CLRF 	ANSEL 		;digital I/O
;		BANKSEL	TRISIO
;		MOVLW	b'00011000' 	;
;		MOVWF 	TRISIO 		;
		
		BANKSEL	GPIO
 		CLRF        GPIO           	; vynuluj výstupy
       		MOVLW	b'00011000' 	;
      		TRIS        GPIO

		BSF	RESET_ISL8
		CALL	CEK200m	
		BCF	EPROM
		BSF	GREEN
		BCF	RED
		CALL	CEK200m
		BCF	RESET_ISL8
		goto	MAIN_PIC

;========================================================================================
; Podprogram PAUSE pro LCD
;========================================================================================
;
; casy pro frekvenci krystalu 4,00Mhz
; casy jsou spocitane od call CEKxxm do navratu na volani CEKxxm
; celkovy vypocet  = 2+6+(2+(TMP0-1)*3+2)+3)*TMP1-1+2
; zkraceny vypocet = 8+((TMP0-1)*3+7)*TMP1+1
; mala smycka = (TMP2-1)*3+2
;

		
CEK200m		movlw	.2		; TMP2 = 2  cekej 0,2 sekundu
		movwf	TMP3
		goto	SMYCKA2	

CEK100m		movlw	0CFh		; cas = 100,009 ms
		movwf	TMP0
		movlw	0A0h
		movwf	TMP1
		goto	SMYCKA

SMYCKA2		call	CEK100m
		decfsz	TMP3,F		; nejvetsi smycka
		goto	SMYCKA2
		retlw	00h
	
SMYCKA		movf	TMP0,W
		movwf	TMP2
		
SMYCKA1		decfsz	TMP2,F		; mala smycka
		goto	SMYCKA1
		decfsz	TMP1,F		; velka smycka
		goto	SMYCKA
		retlw	00h

;========================================================================================
; Konec programu
;========================================================================================

		end