#ifndef ADC_H
#define ADC_H

#define ADC_CH_BATT_MINUS  1u
#define ADC_CH_A1_VOLTAGE  2u
#define ADC_CH_BATT_PLUS   3u
#define ADC_CH_A2_VOLTAGE  6u
#define ADC_CH_A3_VOLTAGE  7u
#define ADC_CH_A3_CURRENT  11u
#define ADC_CH_A2_CURRENT  5u
#define ADC_CH_A1_CURRENT  4u
#define ADC_CH_A2_DCDC     10u

typedef struct
{
    unsigned char coarse[32];
    unsigned char range[32];
    unsigned char dapr[32];
    unsigned char fine[32];
    unsigned int sum;
    unsigned int raw;
} ADC_DEBUG;

void ADC_Init(void);
unsigned int ADC_Measure(unsigned char channel);
unsigned int ADC_MeasureDebug(unsigned char channel, ADC_DEBUG *dbg);
typedef struct
{
    unsigned int raw;
    signed long mv_uncal;
    signed long mv_cal;
} ADC_VOLTAGE_RESULT;

void ADC_MeasureA1(signed char tol,signed char ofs,ADC_VOLTAGE_RESULT *r);
void ADC_MeasureA2(signed char tol,signed char ofs,ADC_VOLTAGE_RESULT *r);
void ADC_MeasureA3(signed char tol,signed char ofs,ADC_VOLTAGE_RESULT *r);

typedef struct
{
    unsigned int raw;
    unsigned int mv;
} ADC_BATT_RESULT;

void ADC_MeasureBattPlus(signed char batt_factor,ADC_BATT_RESULT *r);
unsigned int ADC_BattPlusRawToMv(unsigned int raw,signed char batt_factor);

/* A3 current: original physical input P8.3 / ADC11.
 * Returns the reconstructed A_9C23 filtered internal value.
 * The final proof that one internal unit equals 1 mA is intentionally
 * kept separate from this acquisition routine. */
unsigned int ADC_MeasureA3Current(void);
void ADC_SendDebugAll(void);
signed long ADC_SchulzeMv(unsigned int raw,unsigned long k, signed char tol, signed char ofs, unsigned long *base_out, signed long *cal_out);

#define ADC_K_A1_MV 17792UL
#define ADC_K_A2_MV 14895UL
#define ADC_K_A3_MV  3888UL

#endif
