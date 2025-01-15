#include "LED_BUZZER.h"

#define DEBOUNCE_DELAY_MS 1  // Delay to handle button debounce




// Initialize onboard LEDs
void OnBoardLedINIT() {
    SYSCTL->RCGCGPIO |= (1 << ONBOARD_LED_RED_PIN_BASE_NUM);
    while ((SYSCTL->PRGPIO & (1 << ONBOARD_LED_RED_PIN_BASE_NUM)) == 0);

    ONBOARD_LED_RED_PIN->DIR |= (1 << ONBOARD_LED_RED_PIN_NUM);
    ONBOARD_LED_BLUE_PIN->DIR |= (1 << ONBOARD_LED_BLUE_PIN_NUM);
    ONBOARD_LED_GREEN_PIN->DIR |= (1 << ONBOARD_LED_GREEN_PIN_NUM);

    ONBOARD_LED_RED_PIN->DEN |= (1 << ONBOARD_LED_RED_PIN_NUM);
    ONBOARD_LED_BLUE_PIN->DEN |= (1 << ONBOARD_LED_BLUE_PIN_NUM);
    ONBOARD_LED_GREEN_PIN->DEN |= (1 << ONBOARD_LED_GREEN_PIN_NUM);
}

// Open onboard LED
void OnBoardLedOpen(int number) {
    switch (number) {
        case 0: // Red
            ONBOARD_LED_RED_PIN->DATA |= (1 << ONBOARD_LED_RED_PIN_NUM);
            break;
        case 1: // Blue
            ONBOARD_LED_BLUE_PIN->DATA |= (1 << ONBOARD_LED_BLUE_PIN_NUM);
            break;
        case 2: // Green
            ONBOARD_LED_GREEN_PIN->DATA |= (1 << ONBOARD_LED_GREEN_PIN_NUM);
            break;
    }
}

// Close onboard LED
void OnBoardLedClose(int number) {
    switch (number) {
        case 0: // Red
            ONBOARD_LED_RED_PIN->DATA &= ~(1 << ONBOARD_LED_RED_PIN_NUM);
            break;
        case 1: // Blue
            ONBOARD_LED_BLUE_PIN->DATA &= ~(1 << ONBOARD_LED_BLUE_PIN_NUM);
            break;
        case 2: // Green
            ONBOARD_LED_GREEN_PIN->DATA &= ~(1 << ONBOARD_LED_GREEN_PIN_NUM);
            break;
				case 3:
						ONBOARD_LED_RED_PIN->DATA &= ~(1 << ONBOARD_LED_RED_PIN_NUM);
						ONBOARD_LED_BLUE_PIN->DATA &= ~(1 << ONBOARD_LED_BLUE_PIN_NUM);
						ONBOARD_LED_GREEN_PIN->DATA &= ~(1 << ONBOARD_LED_GREEN_PIN_NUM);
    }
}

// Initialize buzzer
void Buzzer_INIT() {
    SYSCTL->RCGCGPIO |= (1 << BUZZER_PIN_BASE_NUM);
    while ((SYSCTL->PRGPIO & (1 << BUZZER_PIN_BASE_NUM)) == 0);

    BUZZER_PIN->DIR |= (1 << BUZZER_PIN_NUM);
    BUZZER_PIN->DEN |= (1 << BUZZER_PIN_NUM);

    // Initialize Timer1
    SYSCTL->RCGCTIMER |= (1 << TIMER1_BASE_NUM); // Enable Timer1
    TIMER1->CTL &= ~(1 << 0);      // Disable Timer A for configuration
    TIMER1->CFG = 0x04;           // Configure as 16-bit timer
    TIMER1->TAMR = 0x02;          // Set periodic mode
    TIMER1->TAILR = TIMER1_INTERVAL_LOW; // Load the initial interval value
    TIMER1->IMR |= (1 << 0);      // Enable Timer A timeout interrupt
    NVIC->ISER[0] |= (1 << TIMER1_INTERRUPT_NUM); // Enable Timer1 interrupt in NVIC
    TIMER1->CTL |= (1 << 0);      // Enable Timer A
}

void BMP_BUZZER_DRIVE(void){
		Buzzer_INIT();
	
		for(int i=0;i<7;i
	++)
		{
			bmp280->readTemperatureAvg();
			Buzzer_Open(bmp280->temperatureAvg);
			DelayMs(10);
		}
		Buzzer_Close();
		
}

// Open buzzer with frequency based on temperature
void Buzzer_Open(float temperature) {
    // mapping between 0-150C
		uint16_t Buzzer_delay = TIMER1_INTERVAL_LOW -((TIMER1_INTERVAL_LOW-TIMER1_INTERVAL_HIGH)*temperature/150);
		TIMER1->TAILR = Buzzer_delay;


    BUZZER_PIN->DATA |= (1 << BUZZER_PIN_NUM); // Enable buzzer
}

// Close buzzer
void Buzzer_Close() {
    BUZZER_PIN->DATA &= ~(1 << BUZZER_PIN_NUM); // Disable buzzer
	
		

		NVIC->ISER[0] &= ~(1 << TIMER1_INTERRUPT_NUM); //DISABLE INTERRUPT
		TIMER1->CTL &= ~(1 << 0);      // Disable Timer A 
		SYSCTL->RCGCTIMER &= ~(1 << TIMER1_BASE_NUM); // DISABLE Timer1
    
}

// Timer1 interrupt handler
void TIMER1A_Handler(void) {
    if (TIMER1->MIS & (1 << 0)) { // Check if timeout interrupt occurred
        TIMER1->ICR |= (1 << 0);  // Clear the interrupt

        // Toggle buzzer pin
        BUZZER_PIN->DATA ^= (1 << BUZZER_PIN_NUM);

    }
}

void ChangeGraphButtonInit(){
		SYSCTL->RCGCGPIO |= (1 << CHANGE_GRAPH_PIN_BASE_NUM);
		while((SYSCTL->PRGPIO & (1 << CHANGE_GRAPH_PIN_BASE_NUM)) ==0);
	
		CHANGE_GRAPH_PIN->DIR &= ~(1 << CHANGE_GRAPH_PIN_NUM);
		CHANGE_GRAPH_PIN->DEN |= (1 << CHANGE_GRAPH_PIN_NUM);
		CHANGE_GRAPH_PIN->PUR |= (1 << CHANGE_GRAPH_PIN_NUM);

	
};
void DelayMs(uint32_t ms) {
    volatile uint32_t count;
    while (ms > 0) {
        count = 16000;  // Approximate count for 1ms delay on 16 MHz clock
        while (count > 0) {
            count--;
        }
        ms--;
    }
}
void ChangeGraphButtonRead() {
    static uint8_t previousState = 1;  // Store the previous state of the button (default high due to pull-up)
    static uint8_t displayState = 0;  // State variable to track current display mode
    uint8_t currentState;

    // Read the current state of the button
    currentState = (CHANGE_GRAPH_PIN->DATA & (1 << CHANGE_GRAPH_PIN_NUM)) >> CHANGE_GRAPH_PIN_NUM;

    // Check for a falling edge (button press)
    if (previousState == 1 && currentState == 0) {
        // Button pressed, move to the next display state in a loop
			  DelayMs(DEBOUNCE_DELAY_MS);  // Debounce delay
        currentState = (CHANGE_GRAPH_PIN->DATA & (1 << CHANGE_GRAPH_PIN_NUM)) >> CHANGE_GRAPH_PIN_NUM;  // Re-read button state
        if (currentState == 0) {
					displayState = (displayState + 1) % 4;  // Cycle through 0, 1, 2, 3
				
				}
    }
		switch (displayState) {
						case 0:
								// Show thresholds
								char displayBuffer[20];
								snprintf(displayBuffer, sizeof(displayBuffer), "BMP280:%s", keypad->nokiaOutput);
								Nokia5110_Clear();
								Nokia5110_SetCursor(0, 0);
								Nokia5110_OutString(displayBuffer);
								break;

						case 1:
								// Show detected temperatures

								char bmpBuffer[20];
								char bmpThresholdBuffer[20];
								char lm35ThresholdBuffer[20];
								
								snprintf(bmpBuffer, sizeof(bmpBuffer), "BMP280:%2.f", bmp280->temperatureAvg);
								snprintf(bmpThresholdBuffer, sizeof(bmpThresholdBuffer), "BTH:%2.f", bmp280->thresholdTemperature);
								snprintf(lm35ThresholdBuffer,sizeof(lm35ThresholdBuffer),"LMTH:%2.f",LM35_THRESHOLD_READ());
								Nokia5110_Clear();
								Nokia5110_SetCursor(0, 0);
								Nokia5110_OutString(bmpBuffer);
								Nokia5110_SetCursor(0, 1);
								Nokia5110_OutString(bmpThresholdBuffer);
								Nokia5110_SetCursor(0,2);
								Nokia5110_OutString(lm35ThresholdBuffer);
								break;

						case 2:
								// Show graph 1
								drawDistanceVsAngle(radar->angle, radar->distance, 180);
								break;

						case 3:
								if(radar->avg_distance!=-1){
										char angleBuffer[20];
										char distanceBuffer[20];
										snprintf(angleBuffer,sizeof(angleBuffer),"Angle:%2.f",radar->avg_angle-90);
										snprintf(distanceBuffer,sizeof(distanceBuffer),"Distance:%2.f",radar->avg_distance);
										// Show graph 2
										
										Nokia5110_Clear();
										Nokia5110_SetCursor(0, 0);
										Nokia5110_OutString(angleBuffer);
										Nokia5110_SetCursor(0,1);
										Nokia5110_OutString(distanceBuffer);
										
								}
								else{
									Nokia5110_Clear();
									Nokia5110_SetCursor(0,0);
									Nokia5110_OutString("No object...");
								}
								break;
				}

    // Update the previous state
		bmp280->init();
    previousState = currentState;
		bmp280->deinit();
		bmp280->init();
}





