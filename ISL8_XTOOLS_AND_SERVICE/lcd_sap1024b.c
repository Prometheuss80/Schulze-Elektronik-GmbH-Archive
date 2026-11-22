/* SAP1024B / T6963C-compatible LCD backend -- first bench test revision.
 * POWERTIP PG24064E-PD. Verify connector, FS, VO/VEE before power-on.
 * MCU P5: data; P6.0=/WR P6.1=/RD P6.2=C/D P6.3=/CE P6.4=/RST.
 * Bits P6.5 and P6.6 are NOT changed (shared hardware latch strobes).
 */
#include <intrins.h>
#include "hardware.h"
#include "config.h"
#include "lcd.h"
#include "lcd_hw.h"
#include "watchdog.h"

#if LCD_CONTROLLER == LCD_CONTROLLER_SAP1024B
#define SAP_WR   LCD_E
#define SAP_RD   LCD_RW
#define SAP_CD   LCD_RS
#define SAP_CE   LCD_CS
#define SAP_RST  LCD_RESET

/* Diagnostic flag: nonzero after busy timeout. Never hang forever. */
static unsigned char sap_fault = 0u;

static void SAP_BusDelay(void)
{
    _nop_(); _nop_(); _nop_(); _nop_();
    _nop_(); _nop_(); _nop_();
}

/* C/D=1: command or status. C/D=0: data. */
static unsigned char SAP_Status(void)
{
    unsigned char v, et=ET0;
    ET0=0;
    P6 |= (unsigned char)(SAP_WR | SAP_RD | SAP_CE | SAP_CD);
    P5=0xFFu; /* 8051 quasi-bidirectional port: release data lines */
    SAP_BusDelay();
    P6 &= (unsigned char)~(SAP_RD | SAP_CE);
    SAP_BusDelay();
    v=P5;
    P6 |= (unsigned char)(SAP_RD | SAP_CE);
    P5=0x00u;
    ET0=et;
    return v;
}

static unsigned char SAP_WaitReady(void)
{
    unsigned int n;
    for(n=0u;n<40000u;n++)
        if((SAP_Status() & 0x03u)==0x03u) return 1u;
    sap_fault=1u;
    return 0u;
}

static void SAP_WriteRaw(unsigned char v,unsigned char command)
{
    unsigned char et=ET0;
    ET0=0;
    P6 |= (unsigned char)(SAP_WR | SAP_RD | SAP_CE);
    if(command) P6 |= SAP_CD;
    else P6 &= (unsigned char)~SAP_CD;
    P5=v;
    SAP_BusDelay();
    P6 &= (unsigned char)~(SAP_WR | SAP_CE);
    SAP_BusDelay();
    P6 |= (unsigned char)(SAP_WR | SAP_CE);
    P5=0x00u;
    ET0=et;
}

static void SAP_Send(unsigned char v,unsigned char command)
{
    if(sap_fault) return;
    if(!SAP_WaitReady()) return;
    SAP_WriteRaw(v,command);
}

static void SAP_Command(unsigned char cmd){ SAP_Send(cmd,1u); }
static void SAP_Data(unsigned char data_byte){ SAP_Send(data_byte,0u); }
static void SAP_Command2(unsigned char cmd,unsigned char lo,unsigned char hi)
{
    SAP_Data(lo); SAP_Data(hi); SAP_Command(cmd);
}

static unsigned char SAP_Reverse(unsigned char v)
{
#if SAP1024B_REVERSE_BITS
    v=(unsigned char)((v>>4)|(v<<4));
    v=(unsigned char)(((v&0xCCu)>>2)|((v&0x33u)<<2));
    v=(unsigned char)(((v&0xAAu)>>1)|((v&0x55u)<<1));
#endif
    return v;
}

void LCD_HW_SetAddress(unsigned int address)
{
    SAP_Command2(0x24u,(unsigned char)address,(unsigned char)(address>>8));
}

void LCD_HW_WriteByte(unsigned char value)
{
    SAP_Data(SAP_Reverse(value));
    SAP_Command(0xC0u); /* write byte and increment address pointer */
}

void LCD_HW_Init(void)
{
    sap_fault=0u;
    P6 |= (unsigned char)(SAP_WR | SAP_RD | SAP_CE | SAP_CD);
    P6 &= (unsigned char)~SAP_RST;
    HW_Delay(); HW_Delay();
    P6 |= SAP_RST;
    HW_Delay(); HW_Delay();

    SAP_Command(0x90u);              /* display off */
    SAP_Command2(0x40u,0x00u,0x08u); /* text home 0800h, unused */
    SAP_Command2(0x41u,0x1Eu,0x00u); /* text area: 30 bytes */
    SAP_Command2(0x42u,0x00u,0x00u); /* graphic home 0000h */
    SAP_Command2(0x43u,0x1Eu,0x00u); /* graphic area: 30 bytes per row */
    SAP_Command(0x80u);              /* OR mode, internal CG */
    LCD_ClearBuffer();
    LCD_Refresh();                   /* clears display RAM before enabling */
    SAP_Command(0x98u);              /* graphic on, text off */
    /* If sap_fault!=0 the status or bus wiring must be diagnosed. */
}
#endif
