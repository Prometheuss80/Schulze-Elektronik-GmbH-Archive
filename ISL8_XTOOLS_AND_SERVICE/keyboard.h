#ifndef KEYBOARD_H
#define KEYBOARD_H

typedef enum
{
    KEY_NONE = 0,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_MINUS,
    KEY_PLUS,
    KEY_ENTER,
    KEY_ESC,
    KEY_UNKNOWN
} KEY_CODE;

unsigned char Keyboard_ReadRaw(void);
KEY_CODE Keyboard_Decode(unsigned char raw);
KEY_CODE Keyboard_GetKey(void);

#endif
