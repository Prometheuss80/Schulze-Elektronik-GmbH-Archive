#ifndef ISL8_CHARGE_H
#define ISL8_CHARGE_H
#define CHARGE_CHARGE 0u
#define CHARGE_DISCHARGE 1u
#define CHARGE_STOPPED 0u
#define CHARGE_RUNNING 1u
#define CHARGE_FAULT 2u
typedef struct {
    unsigned long u_set_mv;
    unsigned int i_set_ma;
    unsigned int time_set_min;
    unsigned int cap_set_mah;
    unsigned char mode;
    unsigned char state;
    unsigned long elapsed_sec;
    unsigned long capacity_mas;
} CHARGE_CHANNEL;
extern CHARGE_CHANNEL xdata charge[3];
void Charge_Init(void);
#endif
