#ifndef LCD_H
#define LCD_H
void LCD_InitGraphic(void);
void LCD_ClearBuffer(void);
void LCD_SetPixel(unsigned int x,unsigned char y);
void LCD_ClearPixel(unsigned int x,unsigned char y);
void LCD_Rect(unsigned int x1,unsigned char y1,unsigned int x2,unsigned char y2);
void LCD_FillRect(unsigned int x1,unsigned char y1,unsigned int x2,unsigned char y2);
void LCD_ClearRect(unsigned int x1,unsigned char y1,unsigned int x2,unsigned char y2);
void LCD_DrawChar(unsigned int x,unsigned char y,unsigned char c);
void LCD_DrawCharInverse(unsigned int x,unsigned char y,unsigned char c);
void LCD_Print(unsigned int x,unsigned char y,const char *text);
void LCD_PrintInverse(unsigned int x,unsigned char y,const char *text);
void LCD_Refresh(void);
void LCD_RefreshDiff(void);
#endif
