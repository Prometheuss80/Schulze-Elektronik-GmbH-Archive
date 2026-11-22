#include "serial.h"
#include "timekeeper.h"
#include "service.h"

#if 0 /* UNUSED L16: H; kept for reference */
static char H(unsigned char v){v&=15u;return(v<10u)?('0'+v):('A'+v-10u);}
#endif /* UNUSED: H */
#if 0 /* UNUSED L16: Hex8; retained for future work */
static void Hex8(unsigned char v,char *s){s[0]=H(v>>4);s[1]=H(v);s[2]=0;}
#endif /* UNUSED: Hex8 */
static void Dec8(unsigned char v,char *s)
{
    unsigned char n=0,i;char t[4];
    do{t[n++]='0'+v%10u;v/=10u;}while(v);
    for(i=0;i<n;i++)s[i]=t[n-1u-i];s[n]=0;
}
static void Dec16(unsigned int v,char *s)
{
    unsigned char n=0,i;char t[6];
    do{t[n++]='0'+v%10u;v/=10u;}while(v&&n<5u);
    for(i=0;i<n;i++)s[i]=t[n-1u-i];s[n]=0;
}
static void LineValue(const char *name,unsigned char v)
{
    char s[4];Serial_Print(name);Dec8(v,s);Serial_Print(s);
}
static void OK(unsigned char v){Serial_Print(v?"OK\r\n":"FAULT\r\n");}

void Service_SendStartupReport(const TIMEKEEPER_INFO *t)
{
    char s[8];
    Serial_Print("\r\n========================================\r\n");
    Serial_Print("XTOOLS for ISL8 SERVICE V1.04\r\n");
    Serial_Print("SAFE START / SERVICE STATUS\r\n");
    Serial_Print("========================================\r\n");

    Serial_Print("SERIAL NUMBER : ");Dec16(t->copy[0].serial_number,s);Serial_Print(s);
    Serial_Print("  STATUS: ");OK(t->sn_ok);

    Serial_Print("USER          : ");Serial_Print(t->copy[0].owner);
    Serial_Print("  STATUS: ");OK(t->owner_ok);

    Serial_Print("PASSWORD      : NOT VERIFIED\r\n");

    LineValue("A1 TOL: ",t->copy[0].tol_a1);LineValue("  OFS: ",t->copy[0].ofs_a1);
    Serial_Print("  STATUS: ");OK(t->cal_a1_ok);
    LineValue("A2 TOL: ",t->copy[0].tol_a2);LineValue("  OFS: ",t->copy[0].ofs_a2);
    Serial_Print("  STATUS: ");OK(t->cal_a2_ok);
    LineValue("A3 TOL: ",t->copy[0].tol_a3);LineValue("  OFS: ",t->copy[0].ofs_a3);
    Serial_Print("  STATUS: ");OK(t->cal_a3_ok);

    Serial_Print("ZERO BYTE     : STORED / NOT TESTED / NOT DISPLAYED\r\n");
    Serial_Print("MEASUREMENTS  : 3x ADC FILTER / ENTER = DEBUG\r\n");
    Serial_Print("EEPROM        : NOT READY\r\n");
    Serial_Print("========================================\r\n");
}
