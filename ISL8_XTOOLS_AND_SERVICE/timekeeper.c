#include "timekeeper.h"
#include "watchdog.h"
#define XBYTE ((unsigned char volatile xdata *)0)

static const unsigned int code sn_addr[3]={0x9F82u,0x9FACu,0x9FD6u};
static const unsigned int code owner_addr[3]={0x9F84u,0x9FAEu,0x9FD8u};
static const unsigned int code password_addr[3]={0x9F7Au,0x9FA4u,0x9FCEu};
static const unsigned int code cal_addr[3]={0x9F9Du,0x9FC7u,0x9FF1u};

unsigned char Timekeeper_ReadByte(unsigned int address)
{
    return XBYTE[address];
}

void Timekeeper_WriteByte(unsigned int address,unsigned char value)
{
    XBYTE[address]=value;
}

unsigned char Timekeeper_BcdToBin(unsigned char value)
{
    return (unsigned char)(((value>>4)*10u)+(value&0x0Fu));
}

static unsigned char BinToBcd(unsigned char value)
{
    return (unsigned char)(((value/10u)<<4)|(value%10u));
}

static void RtcWriteOn(void)
{
    Timekeeper_WriteByte(0x9FF8u,0x80u);
}

static void RtcWriteOff(void)
{
    Timekeeper_WriteByte(0x9FF8u,0x00u);
}

static unsigned char SameBytes(const unsigned char *a,const unsigned char *b,unsigned char len)
{
    unsigned char i;
    for(i=0u;i<len;i++)
        if(a[i]!=b[i]) return 0u;
    return 1u;
}

static void ReadCopy(unsigned char n,TMK_COPY *c)
{
    unsigned int a;
    unsigned char i,v,hi,lo;

    a=sn_addr[n];
    hi=Timekeeper_ReadByte(a);
    lo=Timekeeper_ReadByte(a+1u);
    c->serial_number=((unsigned int)hi<<8)|lo;

    a=owner_addr[n];
    for(i=0u;i<TMK_OWNER_LEN;i++)
    {
        v=Timekeeper_ReadByte(a+i);
        c->owner_raw[i]=v;
        c->owner[i]=(v>=0x20u && v<=0x7Eu)?(char)v:0;
        if(v==0u)
        {
            unsigned char j;
            for(j=i;j<TMK_OWNER_LEN;j++) c->owner[j]=0;
            break;
        }
    }
    c->owner[TMK_OWNER_LEN]=0;

    a=password_addr[n];
    for(i=0u;i<TMK_PASSWORD_LEN;i++)
    {
        v=Timekeeper_ReadByte(a+i);
        c->password_raw[i]=v;
        c->password[i]=(v>=0x20u && v<=0x7Eu)?(char)v:0;
        if(v==0u)
        {
            unsigned char j;
            for(j=i;j<TMK_PASSWORD_LEN;j++) c->password[j]=0;
            break;
        }
    }
    c->password[TMK_PASSWORD_LEN]=0;

    a=cal_addr[n];
    c->tol_a1=Timekeeper_ReadByte(a+0u);
    c->tol_a2=Timekeeper_ReadByte(a+1u);
    c->zero  =Timekeeper_ReadByte(a+2u);
    c->ofs_a1=Timekeeper_ReadByte(a+3u);
    c->ofs_a2=Timekeeper_ReadByte(a+4u);
    c->ofs_a3=Timekeeper_ReadByte(a+5u);
    c->tol_a3=Timekeeper_ReadByte(a+6u);
}

static unsigned char Same3(unsigned char a,unsigned char b,unsigned char c)
{
    return (a==b && a==c)?1u:0u;
}

static unsigned char CalibrationValid(const TIMEKEEPER_INFO *t)
{
    unsigned char zeros=0u;

    if(!Same3(t->copy[0].tol_a1,t->copy[1].tol_a1,t->copy[2].tol_a1)) return 0u;
    if(!Same3(t->copy[0].tol_a2,t->copy[1].tol_a2,t->copy[2].tol_a2)) return 0u;
    if(!Same3(t->copy[0].ofs_a1,t->copy[1].ofs_a1,t->copy[2].ofs_a1)) return 0u;
    if(!Same3(t->copy[0].ofs_a2,t->copy[1].ofs_a2,t->copy[2].ofs_a2)) return 0u;
    if(!Same3(t->copy[0].ofs_a3,t->copy[1].ofs_a3,t->copy[2].ofs_a3)) return 0u;
    if(!Same3(t->copy[0].tol_a3,t->copy[1].tol_a3,t->copy[2].tol_a3)) return 0u;

    /* Third byte is BATT+ TOL. It is checked separately through batt_tol_ok. BATT+ OFS is FIX0.
       Max. one zero among the six A1/A2/A3 calibration values. */
    if(t->copy[0].tol_a1==0u) zeros++;
    if(t->copy[0].tol_a2==0u) zeros++;
    if(t->copy[0].ofs_a1==0u) zeros++;
    if(t->copy[0].ofs_a2==0u) zeros++;
    if(t->copy[0].ofs_a3==0u) zeros++;
    if(t->copy[0].tol_a3==0u) zeros++;

    return (zeros<=1u)?1u:0u;
}

void Timekeeper_ReadInfo(TIMEKEEPER_INFO *t)
{
    unsigned char i;
    for(i=0;i<3u;i++) ReadCopy(i,&t->copy[i]);

    t->sn_ok=(t->copy[0].serial_number==t->copy[1].serial_number &&
              t->copy[0].serial_number==t->copy[2].serial_number);

    t->owner_ok=(SameBytes(t->copy[0].owner_raw,t->copy[1].owner_raw,TMK_OWNER_LEN) &&
                 SameBytes(t->copy[0].owner_raw,t->copy[2].owner_raw,TMK_OWNER_LEN));

    t->password_ok=(SameBytes(t->copy[0].password_raw,t->copy[1].password_raw,TMK_PASSWORD_LEN) &&
                    SameBytes(t->copy[0].password_raw,t->copy[2].password_raw,TMK_PASSWORD_LEN));

    t->cal_a1_ok=(Same3(t->copy[0].tol_a1,t->copy[1].tol_a1,t->copy[2].tol_a1) &&
                  Same3(t->copy[0].ofs_a1,t->copy[1].ofs_a1,t->copy[2].ofs_a1));
    t->cal_a2_ok=(Same3(t->copy[0].tol_a2,t->copy[1].tol_a2,t->copy[2].tol_a2) &&
                  Same3(t->copy[0].ofs_a2,t->copy[1].ofs_a2,t->copy[2].ofs_a2));
    t->cal_a3_ok=(Same3(t->copy[0].tol_a3,t->copy[1].tol_a3,t->copy[2].tol_a3) &&
                  Same3(t->copy[0].ofs_a3,t->copy[1].ofs_a3,t->copy[2].ofs_a3));
    t->cal_ok=CalibrationValid(t);

    /* Confirmed: copy[n].zero is actually the BATT+ TOL factor. */
    t->batt_tol[0]=t->copy[0].zero;
    t->batt_tol[1]=t->copy[1].zero;
    t->batt_tol[2]=t->copy[2].zero;
    t->batt_tol_ok=Same3(t->batt_tol[0],t->batt_tol[1],t->batt_tol[2]);

    /* Freeze a coherent RTC snapshot exactly as the original firmware does. */
    Timekeeper_WriteByte(0x9FF8u,0x40u);
    {
        unsigned char sec=Timekeeper_ReadByte(0x9FF9u);
        t->rtc_running=(sec&0x80u)?0u:1u;
        t->second_bcd =sec&0x7Fu;
        t->minute_bcd =Timekeeper_ReadByte(0x9FFAu)&0x7Fu;
        t->hour_bcd   =Timekeeper_ReadByte(0x9FFBu)&0x3Fu;
        t->weekday_bcd=Timekeeper_ReadByte(0x9FFCu)&0x07u;
        t->day_bcd    =Timekeeper_ReadByte(0x9FFDu)&0x3Fu;
        t->month_bcd  =Timekeeper_ReadByte(0x9FFEu)&0x1Fu;
        t->year_bcd   =Timekeeper_ReadByte(0x9FFFu);
    }
    Timekeeper_WriteByte(0x9FF8u,0x00u);
}

void Timekeeper_WriteCalibration(unsigned char channel,signed char tol,signed char ofs)
{
    unsigned char i;
    unsigned int a;

    for(i=0u;i<3u;i++)
    {
        a=cal_addr[i];
        if(channel==TMK_CAL_A1)
        {
            Timekeeper_WriteByte(a+0u,(unsigned char)tol);
            Timekeeper_WriteByte(a+3u,(unsigned char)ofs);
        }
        else if(channel==TMK_CAL_A2)
        {
            Timekeeper_WriteByte(a+1u,(unsigned char)tol);
            Timekeeper_WriteByte(a+4u,(unsigned char)ofs);
        }
        else if(channel==TMK_CAL_A3)
        {
            Timekeeper_WriteByte(a+6u,(unsigned char)tol);
            Timekeeper_WriteByte(a+5u,(unsigned char)ofs);
        }
    }
}

void Timekeeper_WriteBattTol(signed char tol)
{
    Timekeeper_WriteByte(TMK_BATT_TOL_1,(unsigned char)tol);
    Timekeeper_WriteByte(TMK_BATT_TOL_2,(unsigned char)tol);
    Timekeeper_WriteByte(TMK_BATT_TOL_3,(unsigned char)tol);
}


void Timekeeper_ResetOwner(void)
{
    static const unsigned char code def_owner[TMK_OWNER_LEN]={
        's','c','h','u','l','z','e',' ',
        'e','l','e','k','t','r','o','n','i','k',' ',
        'g','m','b','h',0
    };
    unsigned char c,i;
    for(c=0u;c<3u;c++)
        for(i=0u;i<TMK_OWNER_LEN;i++)
            Timekeeper_WriteByte(owner_addr[c]+i,def_owner[i]);
}

void Timekeeper_ResetPassword(void)
{
    static const unsigned char code def_password[TMK_PASSWORD_LEN]={
        'k','e','y','w','o','r','d',0
    };
    unsigned char c,i;
    for(c=0u;c<3u;c++)
        for(i=0u;i<TMK_PASSWORD_LEN;i++)
            Timekeeper_WriteByte(password_addr[c]+i,def_password[i]);
}



unsigned char Timekeeper_Erase55(void)
{
    unsigned int a;
    unsigned char ok=1u;

    /* New M48T08 state used by Schulze: fill SRAM only.
     * RTC/control registers 9FF8..9FFF are deliberately untouched. */
    for(a=0x8000u;a<0x9FF8u;a++)
    {
        Timekeeper_WriteByte(a,0x55u);
        if((a & 0x003Fu)==0u) WDT_Kick();
    }

    for(a=0x8000u;a<0x9FF8u;a++)
    {
        if(Timekeeper_ReadByte(a)!=0x55u) ok=0u;
        if((a & 0x003Fu)==0u) WDT_Kick();
    }
    return ok;
}

void Timekeeper_WriteSerial(unsigned int serial_number)
{
    unsigned char i;
    unsigned char hi=(unsigned char)(serial_number>>8);
    unsigned char lo=(unsigned char)(serial_number&0xFFu);
    for(i=0u;i<3u;i++)
    {
        Timekeeper_WriteByte(sn_addr[i],hi);
        Timekeeper_WriteByte(sn_addr[i]+1u,lo);
        WDT_Kick();
    }
}

void Timekeeper_RtcWriteValue(unsigned char field,unsigned char value)
{
    unsigned int a;
    if(field==TMK_RTC_HOUR) a=0x9FFBu;
    else if(field==TMK_RTC_MINUTE) a=0x9FFAu;
    else if(field==TMK_RTC_DAY) a=0x9FFDu;
    else if(field==TMK_RTC_MONTH) a=0x9FFEu;
    else if(field==TMK_RTC_YEAR) a=0x9FFFu;
    else if(field==TMK_RTC_SECOND) a=0x9FF9u;
    else a=0x9FFCu;

    RtcWriteOn();
    if(field==TMK_RTC_SECOND)
    {
        unsigned char st=Timekeeper_ReadByte(0x9FF9u)&0x80u;
        Timekeeper_WriteByte(a,BinToBcd(value)|st);
    }
    else Timekeeper_WriteByte(a,BinToBcd(value));
    RtcWriteOff();
}

void Timekeeper_RtcStart(void)
{
    unsigned char sec;
    RtcWriteOn();
    sec=Timekeeper_ReadByte(0x9FF9u);
    Timekeeper_WriteByte(0x9FF9u,sec&0x7Fu);
    RtcWriteOff();
}

void Timekeeper_RtcStop(void)
{
    unsigned char sec;
    RtcWriteOn();
    sec=Timekeeper_ReadByte(0x9FF9u);
    Timekeeper_WriteByte(0x9FF9u,sec|0x80u);
    RtcWriteOff();
}
