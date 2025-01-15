#ifndef LED_BUZZER_H
#define LED_BUZZER_H

#include "stdio.h"
#include "TM4C123.h"
#include "PINS.h"
#include "Nokia5110.h"
#include "KEYPAD.h"
#include "RADAR.h"
#include "BMP280.h"
#include "ADC.h"



#define TIMER1_BASE_NUM 1
#define TIMER1_INTERVAL_LOW 0xFFF //0XFFFF IDI DEGISTIRDIM
#define TIMER1_INTERVAL_MEDIUM 0x7FFF
#define TIMER1_INTERVAL_HIGH 0xF //0x3fff di degistirdim_bora
#define TIMER1_INTERRUPT_NUM 21


void ChangeGraphButtonInit();
//void ChangeGraphButtonRead(float *angels,float *distances,int size);

void ChangeGraphButtonRead();
void OnBoardLedINIT();
void OnBoardLedOpen(int number);	 //0 for red 1 for blue 2 for green
void OnBoardLedClose(int number); //0 for red 1 for blue 2 for green
void Buzzer_INIT();
void Buzzer_Open(float temperature);
void Buzzer_Close();
void DelayMs(uint32_t ms);
void BMP_BUZZER_DRIVE(void);




#endif