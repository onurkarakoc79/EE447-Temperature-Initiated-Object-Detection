#include <stdio.h>
#include "DEEP_SLEEP.h"
#include "Nokia5110.h"
#include "RADAR.h"
#include <stdint.h>
#include "I2C.h"
#include "BMP280.h"
#include "HCSR04.h"
#include "KEYPAD.h"
#include "LED_BUZZER.h"
#include "ADC.h"



 
extern void SysTick_INIT(void);


int main(void)
{
		
		DEEP_SLEEP_INIT();
		//CLASSICAL_OPERATION();
		while(1){
			
			DEEP_SLEEP();	
			OnBoardLedINIT();
			if(bmp280->waitThreshold()==1){
				BMP_BUZZER_DRIVE();
				RADAR_ENABLE();
			}				
			
		
			//RADAR ENABLEDAN SONRA BUTONA BASILDIGI ANDA HARD FAULT YIYOR
			while(keypad->deepSleepActivate ==1)
			{
					
					//float LM35_THRESHOLD = LM35_THRESHOLD_READ();
					bmp280->readTemperatureAvg();
					ChangeGraphButtonRead();
					keypad->read();
					
			}
		
	}
            
		
    return 0;
}