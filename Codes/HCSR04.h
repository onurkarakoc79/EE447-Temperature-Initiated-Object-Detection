#ifndef HCSR04_H
#define HCSR04_H

#include <stdint.h>

/* Public struct so user can do hcsr04->period, etc. */
typedef struct {
    uint8_t  timer_num;
    volatile uint16_t timerLow;
    volatile uint16_t timerHigh;
    volatile uint32_t periodInt;
    volatile float distance;
    volatile uint16_t rawDistance;
    volatile float pulseWidth;
    volatile float dutyCycle;
    volatile float period;

    /* Function pointers for direct usage. */
    void (*init)(void);
    void (*read)(float temperature);
		void (*deinit)(void);

} HCSR04_Struct;

/* This is your single global instance pointer */
extern HCSR04_Struct* const hcsr04;

#endif // HCSR04_H
