#ifndef CONFIG_H
#define CONFIG_H

#define LCD_WIDTH       240u
#define LCD_HEIGHT       64u
#define LCD_BYTES_LINE   30u
#define LCD_SIZE       1920u

/* ================================================================
 *  LCD CONTROLLER SELECTION -- CHANGE ONLY THE LINE BELOW IN KEIL
 * ================================================================
 *  LCD_CONTROLLER_HD61830 : original Solomon LM6061SYS
 *  LCD_CONTROLLER_SAP1024B: POWERTIP PG24064E-PD
 *
 *  Both backends remain part of the same Keil project.
 *  Build only one backend per firmware image using this selection.
 */
#define LCD_CONTROLLER_HD61830    0u
#define LCD_CONTROLLER_SAP1024B   1u
#define LCD_CONTROLLER LCD_CONTROLLER_HD61830
//#define LCD_CONTROLLER LCD_CONTROLLER_SAP1024B

/* NEW LCD parameters. FS=LOW -> 8 dots/byte must be verified on PCB.
 * The controller uses MSB as leftmost dot, XTOOLS uses LSB leftmost.
 * Set to 1 for bit reversal before transmitting.
 */
#define SAP1024B_REVERSE_BITS 1u

#if (LCD_CONTROLLER != LCD_CONTROLLER_HD61830) && (LCD_CONTROLLER != LCD_CONTROLLER_SAP1024B)
#error Invalid LCD_CONTROLLER selection
#endif
#endif
