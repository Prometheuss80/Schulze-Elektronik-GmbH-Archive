#ifndef BATTERY_PROTECTION_H
#define BATTERY_PROTECTION_H
/* Independent current-loss / A3 discharge overvoltage monitor.
 * Return 1 only after a confirmed disconnect indication.
 * Caller must immediately disable the corresponding power stage. */
#define BP_A2_CHARGE  1u
#define BP_A3_CHARGE  2u
#define BP_A3_DISCH   3u
void BatteryProtection_Init(void);
void BatteryProtection_Start(unsigned char mode,unsigned int idle_raw,unsigned int now);
void BatteryProtection_Stop(void);
unsigned char BatteryProtection_Task(unsigned int now,signed char a3_tol,signed char a3_ofs);
unsigned char BatteryProtection_IsArmed(void);
#endif
