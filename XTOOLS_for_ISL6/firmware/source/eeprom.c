#include "isl6_hw.h"
#include "watchdog.h"
#include "uart.h"
#include "delay.h"
#include "eeprom.h"
#include "hw_profile.h"

/* 93C46/93LC46B, 64 x 16 bit.
 * MODE 1 / 330D: P5.7 CS, P5.6 CLK, P5.5 DI, P5.4 DO (4-wire)
 * MODE 2 / 430+: P5.4 CS, P5.5 CLK, P5.6 shared DI/DO (3-wire)
 */

static void dly(void){ unsigned char i; for(i=0;i<12;i++); }

static unsigned char cs_mask(void){ return (HWProfile_GetMode()==HW_MODE_330)?0x80:0x10; }
static unsigned char clk_mask(void){ return (HWProfile_GetMode()==HW_MODE_330)?0x40:0x20; }
static unsigned char di_mask(void){ return (HWProfile_GetMode()==HW_MODE_330)?0x20:0x40; }
static unsigned char do_mask(void){ return (HWProfile_GetMode()==HW_MODE_330)?0x10:0x40; }

static void set_hi(unsigned char m){ P5|=m; dly(); }
static void set_lo(unsigned char m){ P5&=(unsigned char)~m; dly(); }
static void CS_L(void){set_lo(cs_mask());}
static void CS_H(void){set_hi(cs_mask());}
static void CLK_L(void){set_lo(clk_mask());}
static void CLK_H(void){set_hi(clk_mask());}
static void DI_H(void){set_hi(di_mask());}
static void DI_L(void){set_lo(di_mask());}
static void DO_Release(void){set_hi(do_mask());}

static void SendBit(unsigned char b)
{
    CLK_L(); if(b) DI_H(); else DI_L(); CLK_H(); CLK_L();
}

static unsigned char ReadBit(void)
{
    unsigned char b;
    CLK_L();
    DO_Release();
    /* On MODE2 DI and DO are the same pin, so release is also DI high. */
    CLK_H(); b=(P5 & do_mask())?1:0; CLK_L();
    return b;
}

static void Begin(void)
{
    CS_L(); CLK_L(); DI_H(); DO_Release(); CS_H();
}

static void End(void)
{
    CS_L(); CLK_L(); DI_H(); DO_Release();
}

static void SendAddress6(unsigned char a)
{
    unsigned char i; a&=0x3F;
    for(i=0;i<6;i++){SendBit((a&0x20)?1:0);a<<=1;}
}

static void EWEN(void)
{
    Begin(); SendBit(1);SendBit(0);SendBit(0);SendAddress6(0x30);End();
}
static void EWDS(void)
{
    Begin(); SendBit(1);SendBit(0);SendBit(0);SendAddress6(0x00);End();
}

unsigned int EEPROM_ReadWord(unsigned char address)
{
    unsigned char i; unsigned int v=0;
    Begin(); SendBit(1);SendBit(1);SendBit(0);SendAddress6(address);
    for(i=0;i<16;i++){v<<=1;if(ReadBit())v|=1;WDT_Kick();}
    End(); return v;
}

bit EEPROM_WriteWord(unsigned char address,unsigned int value)
{
    unsigned char i,ready=0; unsigned int mask=0x8000;
    EWEN(); Begin(); SendBit(1);SendBit(0);SendBit(1);SendAddress6(address);
    for(i=0;i<16;i++){SendBit((value&mask)?1:0);mask>>=1;WDT_Kick();}
    CS_L();CLK_L();DI_H();DO_Release();
    CS_H();DO_Release();
    for(i=0;i<25;i++){
        if(P5 & do_mask()){ready=1;break;}
        WDT_Kick();DelayMs(1);
    }
    End();EWDS();
    if(!ready)return 0;
    return (EEPROM_ReadWord(address)==value)?1:0;
}

void EEPROM_DumpAll(void)
{
    unsigned char a; unsigned int v;
    UART_PutString("\r\nEEPROM 93LC46B DUMP 00-3F\r\n");
    UART_PutString("-------------------------\r\n");
    for(a=0;a<0x40;a++){v=EEPROM_ReadWord(a);UART_PutHex8(a);UART_PutChar(' ');UART_PutHex16(v);UART_PutString("\r\n");WDT_Kick();}
    UART_PutString("-------------------------\r\nEEPROM DUMP DONE\r\n");
}
