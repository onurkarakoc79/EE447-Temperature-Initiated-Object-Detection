#include "TM4C123GH6PM.h"
#include <stdio.h>

void ADC_INIT(void);
uint16_t ADC_READ(void);
float ADC_TO_TEMP(uint16_t ADC_VALUE);
float LM35_THRESHOLD_READ(void);

void ADC_INIT(void){
    SYSCTL->RCGCGPIO |= (1 << 1);
    SYSCTL->RCGCADC |= (1 << 0);
    while ((SYSCTL->PRGPIO & (1 << 1)) == 0);

    GPIOB->AFSEL |= (1 << 5);
    GPIOB->DEN &= ~(1 << 5);
    GPIOB->AMSEL |= (1 << 5);

    ADC0->ACTSS &= ~(1 << 3);
    ADC0->EMUX &= ~0xF000;
    ADC0->SSMUX3 = 11;
    ADC0->SSCTL3 = 0x06;
    ADC0->ACTSS |= (1 << 3);

}

void ADC_DEINIT(void) {
    

    // Reset the ADC trigger to default (processor trigger)
    ADC0->EMUX &= ~0xF000;

    // Clear the sequencer multiplexer configuration
    ADC0->SSMUX3 = 0;

    // Clear the sequencer control configuration
    ADC0->SSCTL3 = 0;
	
	// Disable the ADC0 sequencer 3
    ADC0->ACTSS &= ~(1 << 3); 
    // Disable clock to ADC0 to save power
    SYSCTL->RCGCADC &= ~(1 << 0);

    
}



uint16_t ADC_READ(void){
		ADC_INIT();//(1,4) for pb4
    ADC0->PSSI |= (1 << 3);     // Start ADC conversion on SS3
    while ((ADC0->RIS & (1 << 3)) == 0);  // Wait for conversion to complete
    uint16_t ADC_VALUE = ADC0->SSFIFO3 & 0xFFF;     // Read 12-bit ADC result
    ADC0->ISC = (1 << 3);       // Clear completion flag
		return ADC_VALUE*3.3/5;
}

float ADC_TO_TEMP(uint16_t ADC_VALUE){
		ADC_DEINIT();
		return ADC_VALUE*330/4095;
}

float LM35_THRESHOLD_READ(void){
		ADC_INIT();
		return ADC_TO_TEMP(ADC_READ());
}







