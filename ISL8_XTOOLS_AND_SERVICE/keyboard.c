#include <REG80C537_ISL8.H>
#include "keyboard.h"

/*
 * 74HC147 -> P4.0..P4.3
 *
 * Confirmed ISL8 mapping:
 * NONE   0x0F
 * F1     0x05
 * F2     0x0B
 * F3     0x03
 * F4     0x0D
 * F5     0x07
 * MINUS  0x09
 * PLUS   0x01
 * ENTER  0x0E
 * ESC    0x06
 */

unsigned char Keyboard_ReadRaw(void)
{
    return (unsigned char)(P4 & 0x0Fu);
}

KEY_CODE Keyboard_Decode(unsigned char raw)
{
    switch (raw & 0x0Fu)
    {
        case 0x0F: return KEY_NONE;
        case 0x05: return KEY_F1;
        case 0x0B: return KEY_F2;
        case 0x03: return KEY_F3;
        case 0x0D: return KEY_F4;
        case 0x07: return KEY_F5;
        case 0x09: return KEY_MINUS;
        case 0x01: return KEY_PLUS;
        case 0x0E: return KEY_ENTER;
        case 0x06: return KEY_ESC;
        default:   return KEY_UNKNOWN;
    }
}

KEY_CODE Keyboard_GetKey(void)
{
    return Keyboard_Decode(Keyboard_ReadRaw());
}
