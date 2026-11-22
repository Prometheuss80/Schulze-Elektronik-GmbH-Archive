#ifndef TIMEKEEPER_H
#define TIMEKEEPER_H

#define TMK_OWNER_LEN 24u
#define TMK_PASSWORD_LEN 8u

#define TMK_CAL_A1 0u
#define TMK_CAL_A2 1u
#define TMK_CAL_A3 2u

/* CONFIRMED ON ORIGINAL SCHULZE: the third byte (historically labelled ZERO)
 * is the BATT+ TOL factor. BATT+ OFS is fixed at 0 (FIX0).
 *   copy1 9F9F, copy2 9FC9, copy3 9FF3
 */
#define TMK_BATT_TOL_1 0x9F9Fu
#define TMK_BATT_TOL_2 0x9FC9u
#define TMK_BATT_TOL_3 0x9FF3u

typedef struct
{
    unsigned int serial_number;
    unsigned char tol_a1;
    unsigned char tol_a2;
    unsigned char zero;
    unsigned char ofs_a1;
    unsigned char ofs_a2;
    unsigned char ofs_a3;
    unsigned char tol_a3;
    char owner[TMK_OWNER_LEN+1u];
    unsigned char owner_raw[TMK_OWNER_LEN];
    char password[TMK_PASSWORD_LEN+1u];
    unsigned char password_raw[TMK_PASSWORD_LEN];
} TMK_COPY;

typedef struct
{
    TMK_COPY copy[3];

    unsigned char second_bcd;
    unsigned char minute_bcd;
    unsigned char hour_bcd;
    unsigned char weekday_bcd;
    unsigned char day_bcd;
    unsigned char month_bcd;
    unsigned char year_bcd;
    unsigned char rtc_running;

    unsigned char sn_ok;
    unsigned char owner_ok;
    unsigned char password_ok;
    unsigned char cal_a1_ok;
    unsigned char cal_a2_ok;
    unsigned char cal_a3_ok;
    unsigned char cal_ok;

    unsigned char batt_tol[3];
    unsigned char batt_tol_ok;
} TIMEKEEPER_INFO;

unsigned char Timekeeper_ReadByte(unsigned int address);
void Timekeeper_WriteByte(unsigned int address,unsigned char value);
void Timekeeper_ReadInfo(TIMEKEEPER_INFO *info);
unsigned char Timekeeper_BcdToBin(unsigned char value);

void Timekeeper_WriteCalibration(unsigned char channel,signed char tol,signed char ofs);
void Timekeeper_WriteBattTol(signed char tol);
void Timekeeper_ResetOwner(void);
void Timekeeper_ResetPassword(void);
void Timekeeper_WriteSerial(unsigned int serial_number);
unsigned char Timekeeper_Erase55(void);

#define TMK_RTC_HOUR     0u
#define TMK_RTC_MINUTE   1u
#define TMK_RTC_DAY      2u
#define TMK_RTC_MONTH    3u
#define TMK_RTC_YEAR     4u
#define TMK_RTC_WEEKDAY  5u
#define TMK_RTC_SECOND   6u

void Timekeeper_RtcWriteValue(unsigned char field,unsigned char value);
void Timekeeper_RtcStart(void);
void Timekeeper_RtcStop(void);

#endif
