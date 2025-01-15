#ifndef HCSR04_H
#define HCSR04_H

#include "TM4C123.h"

struct HCSR04{
	uint8_t timer_num;
	TIMER0_Type* timer;
	volatile float distance;
	volatile uint16_t rawDistance;
	volatile uint16_t pulseWidth; 
};


void HCSR04_init();
void HCSR04_read(float temperature);



#endif