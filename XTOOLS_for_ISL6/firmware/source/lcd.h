#ifndef __LCD_H__
#define __LCD_H__

void LCD_Init(void);
void LCD_Command(unsigned char command);
void LCD_PutChar(unsigned char c);
void LCD_SetCursor(unsigned char row, unsigned char column);
void LCD_WriteLine(unsigned char row, char *text);
void LCD_SetMode(unsigned char mode);

#endif
