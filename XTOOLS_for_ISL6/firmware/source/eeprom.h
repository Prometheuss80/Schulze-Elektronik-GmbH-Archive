#ifndef __EEPROM_H__
#define __EEPROM_H__

unsigned int EEPROM_ReadWord(unsigned char address);
bit EEPROM_WriteWord(unsigned char address, unsigned int value);
void EEPROM_DumpAll(void);

#endif
