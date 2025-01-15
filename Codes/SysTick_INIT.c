#include "TM4C123GH6PM.h"
#include <stdint.h>

void SysTick_INIT(void){
	SysTick-> CTRL = 0;
	SysTick-> LOAD = 16000-1;//1ms
	SysTick-> VAL = 0;
	SysTick-> CTRL = 0x07;
}




