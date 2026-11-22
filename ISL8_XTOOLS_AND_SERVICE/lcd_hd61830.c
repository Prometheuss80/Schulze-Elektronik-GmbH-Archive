/* Original HD61830A hardware driver, extracted without timing changes. */
#include <intrins.h>
#include "hardware.h"
#include "config.h"
#include "lcd.h"
#include "lcd_hw.h"
#if LCD_CONTROLLER == LCD_CONTROLLER_HD61830
static void LCD_PulseE(void){P6|=LCD_E;_nop_();_nop_();P6&=(unsigned char)~LCD_E;}
static void LCD_Command(unsigned char command){
 unsigned char et=ET0; ET0=0;
 P6&=(unsigned char)~LCD_RW;P6|=LCD_RS;P5=command;LCD_PulseE();
 ET0=et;
}
static void LCD_Data(unsigned char value){
 unsigned char et=ET0; ET0=0;
 P6&=(unsigned char)~LCD_RW;P6&=(unsigned char)~LCD_RS;P5=value;LCD_PulseE();
 ET0=et;
}
static void LCD_Register(unsigned char reg,unsigned char value){LCD_Command(reg);LCD_Data(value);HW_Delay();}
void LCD_HW_SetAddress(unsigned int address){LCD_Register(0x0A,(unsigned char)address);LCD_Register(0x0B,(unsigned char)(address>>8));}
void LCD_HW_WriteByte(unsigned char value){LCD_Command(0x0C);LCD_Data(value);}

void LCD_HW_Init(void){
 P6&=(unsigned char)~LCD_RESET;HW_Delay();HW_Delay();
 P6|=LCD_RESET;HW_Delay();HW_Delay();
 P6&=(unsigned char)~LCD_CS;

 /* Keep display OFF until both CPU framebuffer and HD61830 RAM are clean. */
 LCD_Register(0x00,0x12);
 LCD_Register(0x01,0x07);
 LCD_Register(0x02,0x1D);
 LCD_Register(0x03,0x3F);
 LCD_Register(0x08,0x00);
 LCD_Register(0x09,0x00);

 LCD_ClearBuffer();
 LCD_Refresh();

 /* Only now enable the visible display. */
 LCD_Register(0x00,0x32);
}


#endif
