#ifndef KEYPAD_H
#define KEYPAD_H

#include "PINS.h"
#include "TM4C123.h"
#include "Nokia5110.h"
#include "DEEP_SLEEP.h"


typedef struct{
	volatile uint8_t key;
	volatile float number;
	volatile int activate;
	volatile int deepSleepActivate;
	char nokiaOutput[20];
	void(*init)(void);
	void(*read)(void);
	void(*deinit)(void);
	
}KEYPAD_Struct;


extern KEYPAD_Struct* const keypad;

#endif	 //KEYPAD_H