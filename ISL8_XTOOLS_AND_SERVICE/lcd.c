#include "hardware.h"
#include "config.h"
#include "lcd.h"
#include "lcd_font.h"
#include "watchdog.h"
#include "lcd_hw.h"

unsigned char xdata lcd_buffer[LCD_SIZE];
static unsigned char xdata lcd_shadow[LCD_SIZE];
static unsigned char lcd_shadow_valid=0u;

void LCD_InitGraphic(void)
{
    LCD_HW_Init();
}

void LCD_ClearBuffer(void){
 unsigned int i;
 for(i=0;i<LCD_SIZE;i++){lcd_buffer[i]=0;if((i&0x007Fu)==0u)WDT_Kick();}
}

void LCD_SetPixel(unsigned int x,unsigned char y){
 unsigned int o; unsigned char m;
 if(x>=LCD_WIDTH||y>=LCD_HEIGHT)return;
 o=((unsigned int)y*LCD_BYTES_LINE)+(x>>3);
 m=(unsigned char)(0x01u<<(x&7u));
 lcd_buffer[o]|=m;
}

void LCD_ClearPixel(unsigned int x,unsigned char y){
 unsigned int o; unsigned char m;
 if(x>=LCD_WIDTH||y>=LCD_HEIGHT)return;
 o=((unsigned int)y*LCD_BYTES_LINE)+(x>>3);
 m=(unsigned char)(0x01u<<(x&7u));
 lcd_buffer[o]&=(unsigned char)~m;
}

void LCD_Rect(unsigned int x1,unsigned char y1,unsigned int x2,unsigned char y2){
 unsigned int x; unsigned char y;
 for(x=x1;x<=x2;x++){LCD_SetPixel(x,y1);LCD_SetPixel(x,y2);}
 for(y=y1;y<=y2;y++){LCD_SetPixel(x1,y);LCD_SetPixel(x2,y);}
}

void LCD_FillRect(unsigned int x1,unsigned char y1,unsigned int x2,unsigned char y2){
 unsigned int x; unsigned char y;
 for(y=y1;y<=y2;y++)for(x=x1;x<=x2;x++)LCD_SetPixel(x,y);
}

#if 0 /* UNUSED L16: LCD_ClearRect; retained for future work */
void LCD_ClearRect(unsigned int x1,unsigned char y1,unsigned int x2,unsigned char y2){
 unsigned int x; unsigned char y;
 for(y=y1;y<=y2;y++)for(x=x1;x<=x2;x++)LCD_ClearPixel(x,y);
}
#endif /* UNUSED: LCD_ClearRect */

void LCD_DrawChar(unsigned int x,unsigned char y,unsigned char c){
 unsigned char col,row,bits;
 if(c<0x20u||c>0x7Eu)c='?';
 for(col=0;col<5u;col++){
  bits=Font5x7[c-0x20u][col];
  for(row=0;row<7u;row++)
   if(bits&(1u<<row))LCD_SetPixel(x+col,y+row);
 }
}

void LCD_DrawCharInverse(unsigned int x,unsigned char y,unsigned char c){
 unsigned char col,row,bits;
 if(c<0x20u||c>0x7Eu)c='?';
 for(col=0;col<6u;col++){
  bits=(col<5u)?Font5x7[c-0x20u][col]:0;
  for(row=0;row<8u;row++){
   if(col<5u&&row<7u&&(bits&(1u<<row)))LCD_ClearPixel(x+col,y+row);
   else LCD_SetPixel(x+col,y+row);
  }
 }
}

void LCD_Print(unsigned int x,unsigned char y,const char *text){
 while(*text){if(x+4u>=LCD_WIDTH)break;LCD_DrawChar(x,y,(unsigned char)*text++);x+=6u;}
}

void LCD_PrintInverse(unsigned int x,unsigned char y,const char *text){
 while(*text){if(x+5u>=LCD_WIDTH)break;LCD_DrawCharInverse(x,y,(unsigned char)*text++);x+=6u;}
}

void LCD_Refresh(void){
 unsigned int i;
 lcd_shadow_valid=0u;
 LCD_HW_SetAddress(0);
 for(i=0;i<LCD_SIZE;i++){
  LCD_HW_WriteByte(lcd_buffer[i]);
  if((i&0x003Fu)==0u)WDT_Kick();
 }
}

/* A2 only: update changed byte runs instead of transmitting all 1920 bytes. */
void LCD_RefreshDiff(void)
{
    unsigned int i,j;
    if(!lcd_shadow_valid)
    {
        LCD_Refresh();
        for(i=0u;i<LCD_SIZE;i++) lcd_shadow[i]=lcd_buffer[i];
        lcd_shadow_valid=1u;
        return;
    }
    i=0u;
    while(i<LCD_SIZE)
    {
        if(lcd_shadow[i]==lcd_buffer[i]) {++i;continue;}
        LCD_HW_SetAddress(i);
        j=i;
        while(j<LCD_SIZE && lcd_shadow[j]!=lcd_buffer[j])
        {
            LCD_HW_WriteByte(lcd_buffer[j]);
            lcd_shadow[j]=lcd_buffer[j];
            ++j;
            if((j&0x003Fu)==0u)WDT_Kick();
        }
        i=j;
    }
}

/* ================================================================
 * LCD BACKEND LINKAGE -- BOTH IMPLEMENTATIONS ARE INCLUDED HERE.
 * In Keil target keep lcd.c, but DO NOT add lcd_hd61830.c or
 * lcd_sap1024b.c as separate compilation units.
 * The selected backend is controlled only by config.h.
 * ================================================================ */
#include "lcd_hd61830.c"
#include "lcd_sap1024b.c"
