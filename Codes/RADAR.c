
#include "TM4C123GH6PM.h"
#include <stdint.h>
#include "Nokia5110.h"
#include "RADAR.h"
#include "HCSR04.h"
#include "I2C.h"
#include "BMP280.h"

volatile float BMP280_TEMP_DATA = 0;



extern void SysTick_INIT(void);


volatile float distance = 1000;
volatile uint32_t ms_ticks=0;
volatile int step_flag=0;
volatile uint32_t delay = RENDER_RATE*2;//RENDER_RATE * 2ms

uint32_t HCSR_READ(void);//BUNU ONURDAN BEKLIYORUZ

void SysTick_Handler(void){
	ms_ticks++;
	if(ms_ticks % delay ==0){
		step_flag=1;
	}

}


void RADAR_GPIO_ENABLE(){
    // Step 1: Enable clocks for GPIO Port A and Port E
    SYSCTL->RCGCGPIO |=  (1 << 4);  // Enable clock for Port A and Port E
    while (!(SYSCTL->PRGPIO & ( (1 << 4)))); // Wait for Ports A and E to be ready

    // Step 2: Configure PE2 and PE3
    GPIOE->DATA |= 0x3C;     // Set PE2 and PE3 high (0x0C = 0b00001100)
    GPIOE->DIR |= 0x3C;      // Set PE2 and PE3 as outputs
    GPIOE->AFSEL &= ~0x3C;   // Disable alternate functions for PE2 and PE3
    GPIOE->DEN |= 0x3C;      // Enable digital functionality for PE2 and PE3
    GPIOE->AMSEL &= ~0x3C;   // Disable analog functionality for PE2 and PE3


}



void Run_Motor(int step_no){
	
	if(step_no==0){
		GPIOE->DATA &= ~0X38;
		GPIOE->DATA |= 0X04;//0000 0100		
		
	}
	else if(step_no==1){
		GPIOE->DATA &= ~0X34;
		GPIOE->DATA |= 0X08;//0000 1000
	}
	else if(step_no==2){
		GPIOE->DATA &= ~0X2C;
		GPIOE->DATA |= 0X10;
	}
	else if(step_no==3){
		GPIOE->DATA &= ~0X1C;
		GPIOE->DATA |= 0X20;

	}
	else{
		__ASM("NOP");
	}
}
void RADAR_DISABLE(){
		RADAR_GPIO_ENABLE();
		GPIOE->DATA &= ~0X3C;
}



RADAR_Struct radar_instance = {
    .distance = {0},
    .angle = {0},
    .avg_distance = -1,
    .avg_angle = 0
    
};

RADAR_Struct* const radar = &radar_instance;


void RADAR_ENABLE(){
	float total_distance = 0;
  float total_angle = 0;
  int object_count = 0;
	int counter=0;
	RADAR_GPIO_ENABLE();

	for(int step = 0;step<step_motor_angle;step++){
			while(step_flag==0){};
			step_flag = 0;
			Run_Motor(step%step_count);
			

			if(step%RENDER_RATE==0){
					
					hcsr04->read(25.0);
					
					radar->angle[counter] = 180 * step / 1024;
					radar->distance[counter] = hcsr04->distance;
					
					if (radar->distance[counter] < 100) { // Object detected
							total_distance += radar->distance[counter];
							total_angle += radar->angle[counter];
							object_count++;
					}
					else{
						radar->distance[counter]=100.0;
						
						
					}
					counter++;
			}

	}

	for(int step = step_motor_angle;step>0;step--){
		while(step_flag==0);
		step_flag = 0;
		Run_Motor(step%step_count);

	}
		
		radar->avg_distance = (object_count > 0) ? (total_distance / object_count) : -1;
		if(radar->avg_distance>95 || object_count <2){
			radar->avg_distance=-1;
		};
    radar->avg_angle = (object_count > 0) ? (total_angle / object_count) : 0;
		if(radar->avg_distance>75){
			OnBoardLedOpen(2);
			OnBoardLedClose(1);
			OnBoardLedClose(0);
		}
		else if(radar->avg_distance>50){
			OnBoardLedOpen(1);
			OnBoardLedClose(2);
			OnBoardLedClose(0);
		}
		else if(radar->avg_distance>0){
			OnBoardLedOpen(0);
			OnBoardLedClose(1);
			OnBoardLedClose(2);
		}
		else{
			OnBoardLedOpen(2);
			OnBoardLedClose(1);
			OnBoardLedClose(0);
		}
		RADAR_DISABLE();
		keypad->deinit();
		keypad->init();
}










