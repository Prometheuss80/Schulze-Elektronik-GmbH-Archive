#ifndef __ISL6_HW_H__
#define __ISL6_HW_H__
sfr P0=0x80; sfr SP=0x81; sfr DPL=0x82; sfr DPH=0x83; sfr PCON=0x87; sfr TCON=0x88; sfr TMOD=0x89; sfr TL0=0x8A; sfr TL1=0x8B; sfr TH0=0x8C; sfr TH1=0x8D;
sfr P1=0x90; sfr SCON=0x98; sfr SBUF=0x99; sfr P2=0xA0;
sfr IEN0=0xA8; sfr IP0=0xA9; sfr P3=0xB0; sfr IEN1=0xB8; sfr IP1=0xB9;
sfr IRCON=0xC0; sfr CCEN=0xC1; sfr CCL1=0xC2; sfr CCH1=0xC3; sfr CCL2=0xC4; sfr CCH2=0xC5; sfr CCL3=0xC6; sfr CCH3=0xC7;
sfr T2CON=0xC8; sfr CRCL=0xCA; sfr CRCH=0xCB; sfr TL2=0xCC; sfr TH2=0xCD;
sfr ADCON=0xD8; sfr ADDAT=0xD9; sfr DAPR=0xDA; sfr P6=0xDB;
sfr P4=0xE8; sfr P5=0xF8;
sbit TR0=TCON^4; sbit TF0=TCON^5;
sbit BEEPER=P1^1; sbit WDT_EXT=P1^4; sbit KEY_LEFT=P1^6; sbit KEY_RIGHT=P1^7;
sbit EEPROM_CS=P5^4; sbit EEPROM_CLK=P5^5; sbit EEPROM_DATA=P5^6;
sbit P5_1_UNKNOWN=P5^1; sbit P5_2_UNKNOWN=P5^2;
sbit RI=SCON^0; sbit TI=SCON^1;
sbit EA=IEN0^7; sbit CPU_WDT=IEN0^6; sbit CPU_SWDT=IEN1^6;
void HW_Init(void);
#endif
