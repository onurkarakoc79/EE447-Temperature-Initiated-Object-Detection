#ifndef DEEP_SLEEP_H
#define DEEP_SLEEP_H

#include "TM4C123.h"


static uint16_t DEEP_SLEEP_ENABLE=1;


void BUTTON_DEEP_SLEEP(void);

void LM35_AC0_Init(void);


void LM35_AC0_DeInit(void);


void WAKE_UP(void);


void DEEP_SLEEP(void);


void POWER_LED_ON(void);


void POWER_LED_OFF(void);


void COMP0_Handler(void);


void DEEP_SLEEP_INIT(void);

void CLASSICAL_OPERATION(void);

#endif