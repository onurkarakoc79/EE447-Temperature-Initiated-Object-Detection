#include "TM4C123GH6PM.h"
#include <stdint.h>
#include <stdio.h>
#include "DEEP_SLEEP.h"
#include "Nokia5110.h"
#include "RADAR.h"
#include "HCSR04.h"
#include "I2C.h"
#include "BMP280.h"
#include "KEYPAD.h"
#include "LED_BUZZER.h"
#include "ADC.h"



void LM35_AC0_Init(void);
void LM35_AC0_DeInit(void);
void WAKE_UP(void);
void DEEP_SLEEP(void);
void POWER_LED_ON(void);
void POWER_LED_OFF(void);
void COMP0_Handler(void);
void BUTTON_DEEP_SLEEP(void);
void DEEP_SLEEP_INIT(void);
void CLASSICAL_OPERATION(void);
	//GPIO_DESTRUCTOR();
	//TIMER_DESTRUCTOR();
extern void SysTick_INIT(void);


static uint8_t LM_35_AboveThreshold = 0;

volatile float LM35_THRESHOLD =30;
char LM35_THRESHOLD_BUFFER[20];



void LM35_AC0_Init(void) {
    SYSCTL->RCGCACMP |= 0x01;          
    while (!(SYSCTL->PRACMP & 0x01));  

    SYSCTL->RCGCGPIO |= (1 << 2);      
    while (!(SYSCTL->PRGPIO & (1 << 2)));

    GPIOC->AFSEL |= (1 << 6) | (1 << 7);
    GPIOC->DEN &= ~((1 << 6) | (1 << 7));
    GPIOC->AMSEL |= (1 << 6) | (1 << 7);

    COMP->ACCTL0 = (1 << 4) |  // Enable output inversion (optional)
                   (1 << 3) |  // Trigger on falling edge
                   (1 << 2);   // Trigger on rising edge

    COMP->ACCTL0 |= (0x0 << 9) | (0x0 << 10);

    COMP->ACINTEN |= 0x01;
    NVIC->ISER[0] |= (1 << 25);
}


void LM35_AC0_DeInit(void) {
    // Step 1: Disable the Comparator
    COMP->ACCTL0 &= ~0x01;      // Disable Comparator 0

    // Step 2: Disable Comparator Interrupts
    COMP->ACINTEN &= ~0x01;     // Disable interrupt for Comparator 0
    NVIC->ICER[0] |= (1 << 25); // Disable AC0 interrupt in NVIC (Interrupt #25)

    // Step 3: Reset GPIO Configuration for PC6 (AIN0) and PC7 (AIN1)
    GPIOC->AFSEL &= ~((1 << 6) | (1 << 7)); // Disable alternate function on PC6 and PC7
    GPIOC->DEN |= ((1 << 6) | (1 << 7));    // Enable digital function for PC6 and PC7
    GPIOC->AMSEL &= ~((1 << 6) | (1 << 7)); // Disable analog function on PC6 and PC7

    // Step 4: Disable Clock for Comparator and GPIO Port C
    SYSCTL->RCGCACMP &= ~0x01;  // Disable clock for Analog Comparator
		__ASM("NOP");
		__ASM("NOP");
		__ASM("NOP");
    
}


void COMP0_Handler(void) {
    if (COMP->ACMIS & 0x01) {   // Check if AC0 interrupt flag is set
        COMP->ACMIS = 0x01;     // Clear the interrupt flag

        if (COMP->ACSTAT0 & 0x02) {  // Check comparator output (COUT)
            LM_35_AboveThreshold = 0;  // Rising edge detected

        } else {
            LM_35_AboveThreshold = 1;  // Falling edge detected
        }
    }

}

void POWER_LED_ON(void){
		SYSCTL->RCGCGPIO |= (1 << 1);  // Enable clock for Port B
    while (!(SYSCTL->PRGPIO & (1 << 1)));  // Wait until Port B is ready

    // Step 2: Configure PB4 as an output
    GPIOB->DIR |= (1 << 4);  // Set PB4 as an output
    GPIOB->DEN |= (1 << 4);  // Enable digital function for PB4

    // Step 3: Set PB4 high
    GPIOB->DATA |= (1 << 4);  // Write 1 to PB4 to set it high
}
void POWER_LED_OFF(void){
		SYSCTL->RCGCGPIO |= (1 << 1);  // Enable clock for Port B
    while (!(SYSCTL->PRGPIO & (1 << 1)));  // Wait until Port B is ready

    // Step 2: Configure PB4 as an output
    GPIOB->DIR |= (1 << 4);  // Set PB4 as an output
    GPIOB->DEN |= (1 << 4);  // Enable digital function for PB4

    GPIOB->DATA &= ~(1 << 4);  // Write 1 to PB4 to set it high
}

void WAKE_UP(void) {
    COMP->ACMIS &= ~0x01;       // Clear AC0 interrupt flag
    NVIC->ICPR[0] = (1 << 25); // Clear any pending COMP0 interrupt in NVIC
    COMP->ACRIS &= ~0x01;       // Clear raw interrupt status (RIS)

    LM35_AC0_DeInit();          // De-initialize Comparator

		
    Nokia5110_Init(); 
		Nokia5110_Clear();
    POWER_LED_ON();
		keypad->init();
		ChangeGraphButtonInit();
		SysTick_INIT();
		hcsr04->init();
		I2C0_Init();
		bmp280->init();
		keypad->deepSleepActivate=1;
	
}

void DEEP_SLEEP_INIT(void){
		Nokia5110_Init(); 
		OnBoardLedINIT();
		POWER_LED_ON();
		keypad->init();
		ChangeGraphButtonInit();
		SysTick_INIT();
		hcsr04->init();
		I2C0_Init();
		bmp280->init();
		
	

}

void CLASSICAL_OPERATION(void){
		DEEP_SLEEP();	
		OnBoardLedINIT();
		bmp280->waitThreshold();
		BMP_BUZZER_DRIVE();
		RADAR_ENABLE();
	
		//RADAR ENABLEDAN SONRA BUTONA BASILDIGI ANDA HARD FAULT YIYOR
	    while(1)
    {
				
				//float LM35_THRESHOLD = LM35_THRESHOLD_READ();
				bmp280->readTemperatureAvg();
				ChangeGraphButtonRead();
				keypad->read();
				
    }

}






#include "TM4C123GH6PM.h"

void ResetDevice(void) {
    // Unlock the AIRCR register
    SCB->AIRCR = (0x5FA << 16) |          // Write the key (0x5FA)
                 (1 << 2);                // Set SYSRESETREQ to request a reset

    // Wait for reset to take effect (optional, the MCU resets almost immediately)
    while (1) {
        __NOP();
    }
}
void BUTTON_DEEP_SLEEP(void){
	ResetDevice();
	

}

void DEEP_SLEEP(void) {
		LM35_THRESHOLD = LM35_THRESHOLD_READ();
	
		Nokia5110_Clear();
		IntToStr(LM35_THRESHOLD, LM35_THRESHOLD_BUFFER);
		Nokia5110_OutString("LM35_TH:");
		Nokia5110_OutString(LM35_THRESHOLD_BUFFER);
		//Nokia5110_DeInit();        // De-initialize Nokia LCD
    POWER_LED_OFF();           // Turn off power LED
		keypad->deinit();
    RADAR_DISABLE();           // Disable Radar
		
		hcsr04->deinit();
		
		bmp280->deinit();
		OnBoardLedClose(3);
		
		//
		//OTHER DESTRUCTORS HERE

    // Configure deep sleep
    SCB->SCR |= (1U << 2);     // Enable deep sleep mode

    // Ensure only the Analog Comparator clock is active
	    SYSCTL->RCGCGPIO &= ~0xFB; // Turn off all GPIO ports except Port C
		__ASM("NOP");
		__ASM("NOP");
		__ASM("NOP");
   SYSCTL->RCGCACMP |= 0x01;  // Keep clock for Analog Comparator active
			__ASM("NOP");
		__ASM("NOP");
		__ASM("NOP");


    // Clear Comparator flags

	
		          // Re-initialize Comparator
    // Enter deep sleep mode
			LM35_AC0_Init(); 
    __DSB();                   // Ensure memory operations complete
		
    while (LM_35_AboveThreshold == 0) {
		
      __WFI();               // Wait for interrupt (enters deep sleep)
			
    }
    WAKE_UP();
}


/*
int main(){
	DEEP_SLEEP();
	//POWER_LED_OFF();


}
*/

