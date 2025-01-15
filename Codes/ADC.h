#include "TM4C123GH6PM.h"


void ADC_INIT(void);

void ADC_DEINIT(void);

uint16_t ADC_READ(void);

float ADC_TO_TEMP(uint16_t ADC_VALUE);

float LM35_THRESHOLD_READ(void);