#ifndef __UART_H__
#define __UART_H__

void UART_Init(void);
void UART_PutChar(unsigned char c);
void UART_PutString(const char code *s);
void UART_PutHex8(unsigned char value);
void UART_PutHex16(unsigned int value);
void UART_PutUInt16(unsigned int value);
bit UART_CharAvailable(void);
unsigned char UART_GetChar(void);

#endif
