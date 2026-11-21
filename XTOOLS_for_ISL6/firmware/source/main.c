#include "isl6_hw.h"
#include "watchdog.h"
#include "uart.h"
#include "lcd.h"
#include "eeprom.h"
#include "beeper.h"
#include "service.h"
#include "serial_cmd.h"
#include "hw_profile.h"

void main(void)
{
    WDT_Kick();
    HW_Init();
    WDT_Init();
    UART_Init();
    Beeper_Init();

    UART_PutString("\r\nXTOOLS for Schulze ISL6 V0.8 UNIVERSAL\r\n");
    UART_PutString("HW INIT OK\r\n");

    LCD_Init();
    UART_PutString("LCD INIT OK\r\n");
    UART_PutString("SELECT HARDWARE MODE ON CHARGER\r\n");

    HWProfile_Select();

    if (HWProfile_GetMode() == HW_MODE_330)
    {
        UART_PutString("MODE: 1\r\nMODEL: ISL6-330D\r\n");
        UART_PutString("EEPROM: 93C46B 4-WIRE\r\nLCD: 1x16\r\n");
    }
    else
    {
        UART_PutString("MODE: 2\r\nMODEL: ISL6-430/530/636 family\r\n");
        UART_PutString("EEPROM: 93C46B 3-WIRE\r\nLCD: 2x16\r\n");
    }

    LCD_WriteLine(0, "XTOOLS ISL6");

    UART_PutString("EEPROM READ START\r\n");
    Service_ReadEEPROM();
    UART_PutString("EEPROM READ OK\r\n");
    Service_ReportUART();

    UART_PutString("\r\nSTARTUP OK - BEEP\r\n");
    Beeper_OK();
    SerialCmd_Window();
    UART_PutString("LCD OVERVIEW ACTIVE\r\n");
    Service_LCDLoop();
}
