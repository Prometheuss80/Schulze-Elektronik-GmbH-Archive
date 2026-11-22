#ifndef LCD_HW_H
#define LCD_HW_H
/* Private backend interface; graphics and user interface remain unchanged. */
void LCD_HW_Init(void);
void LCD_HW_SetAddress(unsigned int address);
void LCD_HW_WriteByte(unsigned char value);
#endif
