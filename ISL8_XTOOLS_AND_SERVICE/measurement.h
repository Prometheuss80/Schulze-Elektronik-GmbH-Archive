#ifndef ISL8_MEASUREMENT_H
#define ISL8_MEASUREMENT_H
/* Background RAW scan: one ADC channel per 100 ms. */
typedef struct {
    unsigned int raw[12];
    unsigned int valid_mask;
    unsigned int last_tick;
} MEASUREMENT_DATA;
extern MEASUREMENT_DATA xdata measurement;
void Measurement_Init(void);
void Measurement_Task(void);
#endif
