#include "KEYPAD.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "BMP280.h"
#include "DEEP_SLEEP.h"	


#define MAX_RESULT_LENGTH 20  // Adjust to handle large inputs
#define GPIO_UNLOCK_KEY  0x4C4F434B  // "LOCB"
#define TEXT_OFFSET		

volatile char result[MAX_RESULT_LENGTH] = "";  // Buffer to store the input
extern uint8_t KEYPAD_Read_ASM(void);  // Reads the pressed key (returns 0–12)


static void initGPIO(void);
static void readKEYPAD(void);
static void deinit(void);

static KEYPAD_Struct keypad_instance= {
		.key = -1,
		.number =-1,
		.activate=0,
		.deepSleepActivate=1
};

KEYPAD_Struct* const keypad = &keypad_instance;


/**
 * @brief Initializes keypad rows & columns, plus configures PD3 as an interrupt input.
 */
void initGPIO(void)
{
    // Enable clocks for the relevant Ports (bit 3 => Port D, bit 2 => Port C, etc.)
    SYSCTL->RCGCGPIO |= (1 << KEYPAD_ROW_1_PIN_BASE_NUM) 
                      | (1 << KEYPAD_ROW_2_PIN_BASE_NUM)
                      | (1 << KEYPAD_ROW_3_PIN_BASE_NUM)
                      | (1 << KEYPAD_COL_1_PIN_BASE_NUM) 
                      | (1 << KEYPAD_COL_2_PIN_BASE_NUM) 
                      | (1 << KEYPAD_COL_3_PIN_BASE_NUM) 
                      | (1 << KEYPAD_COL_4_PIN_BASE_NUM)
                      | (1 << KEYPAD_ENTER_PIN_BASE_NUM);  // ensure Port D is powered for PD3

    // Wait until the ports are ready
    while ((SYSCTL->PRGPIO & ((1 << KEYPAD_ROW_1_PIN_BASE_NUM) 
                            | (1 << KEYPAD_ROW_2_PIN_BASE_NUM)
                            | (1 << KEYPAD_ROW_3_PIN_BASE_NUM)
                            | (1 << KEYPAD_COL_1_PIN_BASE_NUM) 
                            | (1 << KEYPAD_COL_2_PIN_BASE_NUM) 
                            | (1 << KEYPAD_COL_3_PIN_BASE_NUM) 
                            | (1 << KEYPAD_COL_4_PIN_BASE_NUM)
                            | (1 << KEYPAD_ENTER_PIN_BASE_NUM))) == 0);
				
		
		// ----------- Unlock PD7 (KEYPAD_ROW_3_PIN) if needed -----------
    // Since we know KEYPAD_ROW_3_PIN = GPIOD and KEYPAD_ROW_3_PIN_NUM = 7:
    GPIOD->LOCK = GPIO_UNLOCK_KEY;        // Write the unlock key
    GPIOD->CR  |= (1 << 7);              // Commit PD7
    // Optionally, re-lock by writing any value != 0x4C4F434B:
    // GPIOD->LOCK = 0;
		
    // Row pins as outputs
    //----------------------------------------------------------------
    // Configure Row pins as outputs
    //----------------------------------------------------------------
    KEYPAD_ROW_1_PIN->DIR |= (1 << KEYPAD_ROW_1_PIN_NUM);
    KEYPAD_ROW_1_PIN->DEN |= (1 << KEYPAD_ROW_1_PIN_NUM);

    KEYPAD_ROW_2_PIN->DIR |= (1 << KEYPAD_ROW_2_PIN_NUM);
    KEYPAD_ROW_2_PIN->DEN |= (1 << KEYPAD_ROW_2_PIN_NUM);

    KEYPAD_ROW_3_PIN->DIR |= (1 << KEYPAD_ROW_3_PIN_NUM);
    KEYPAD_ROW_3_PIN->DEN |= (1 << KEYPAD_ROW_3_PIN_NUM);

    //----------------------------------------------------------------
    // Configure Column pins as inputs with pull-ups
    //----------------------------------------------------------------
    KEYPAD_COL_1_PIN->DIR &= ~(1 << KEYPAD_COL_1_PIN_NUM);
    KEYPAD_COL_1_PIN->DEN |=  (1 << KEYPAD_COL_1_PIN_NUM);
    KEYPAD_COL_1_PIN->PUR |=  (1 << KEYPAD_COL_1_PIN_NUM);

    KEYPAD_COL_2_PIN->DIR &= ~(1 << KEYPAD_COL_2_PIN_NUM);
    KEYPAD_COL_2_PIN->DEN |=  (1 << KEYPAD_COL_2_PIN_NUM);
    KEYPAD_COL_2_PIN->PUR |=  (1 << KEYPAD_COL_2_PIN_NUM);

    KEYPAD_COL_3_PIN->DIR &= ~(1 << KEYPAD_COL_3_PIN_NUM);
    KEYPAD_COL_3_PIN->DEN |=  (1 << KEYPAD_COL_3_PIN_NUM);
    KEYPAD_COL_3_PIN->PUR |=  (1 << KEYPAD_COL_3_PIN_NUM);

    KEYPAD_COL_4_PIN->DIR &= ~(1 << KEYPAD_COL_4_PIN_NUM);
    KEYPAD_COL_4_PIN->DEN |=  (1 << KEYPAD_COL_4_PIN_NUM);
    KEYPAD_COL_4_PIN->PUR |=  (1 << KEYPAD_COL_4_PIN_NUM);

    //----------------------------------------------------------------
    // Configure PD3 (KEYPAD_ENTER_PIN) as an INPUT with interrupt
    //----------------------------------------------------------------
    KEYPAD_ENTER_PIN->DIR &= ~(1 << KEYPAD_ENTER_PIN_NUM);  // input
    KEYPAD_ENTER_PIN->DEN |=  (1 << KEYPAD_ENTER_PIN_NUM);  // digital enable

    // Optional pull-up if you want PD3 normally high
    KEYPAD_ENTER_PIN->PUR |=  (1 << KEYPAD_ENTER_PIN_NUM);

    // Configure interrupt for "falling edge" on PD3
    KEYPAD_ENTER_PIN->IS  &= ~(1 << KEYPAD_ENTER_PIN_NUM); // edge-sensitive (IS=0)
    KEYPAD_ENTER_PIN->IBE &= ~(1 << KEYPAD_ENTER_PIN_NUM); // not both edges (IBE=0)
    KEYPAD_ENTER_PIN->IEV &= ~(1 << KEYPAD_ENTER_PIN_NUM); // falling edge (IEV=0 => falling)
    KEYPAD_ENTER_PIN->ICR  =  (1 << KEYPAD_ENTER_PIN_NUM); // clear any prior interrupt
    KEYPAD_ENTER_PIN->IM  |=  (1 << KEYPAD_ENTER_PIN_NUM); // unmask interrupt for PD3

    // Enable "GPIO Port D" interrupt in NVIC (Port D = IRQ #3 on TM4C123)
    NVIC->ISER[0] = (1 << 3);
}

/**
 * @brief Handles the GPIO Port D interrupt (e.g., for PD3).
 */

void GPIOD_Handler(void)
{
    if ((GPIOD->MIS & (1 << 3)) != 0) // Check if PD3 triggered
    {
        // 1. Disable interrupts on PD3 to prevent re-entry
        GPIOD->IM &= ~(1 << 3);
        
        // 2. Clear the interrupt flag
        GPIOD->ICR = (1 << 3);
        
        // 3. Perform the main action
        keypad->activate = !keypad->activate;  // Toggle keypad active mode

        // 4. Simple blocking delay for debounce (~50-100 ms)
     
        
        // 5. Wait until the button is released (PD3 goes high)
				while(1){
					
					while ((GPIOD->DATA & (1 << 3)) == 0)
					{
							// Optionally, add a timeout here to prevent an infinite loop
					}
					for (volatile int i = 0; i < 200000; i++)
						{
									// Adjust the loop count based on your system clock to achieve ~50-100 ms delay
						}
					if((GPIOD->DATA & (1<<3))!=0){
							break;
					}
				}
         // 2. Clear the interrupt flag
        GPIOD->ICR = (1 << 3);
        // 6. Re-enable interrupts on PD3
        GPIOD->IM |= (1 << 3);
    }
}

/**
 * @brief Reads and processes keypad inputs when active.
 */
volatile float numberReturn = THRESHOLD;
void readKEYPAD(void)
{
    volatile uint8_t counter = 0;
    volatile int dot_pressed = 0;  // Flag to indicate if the dot (key 10) was pressed
    volatile float number = 0;     // Final parsed number

    volatile float fraction_part = 0;  // Fractional part
    volatile int fraction_multiplier = 1;  // Multiplier for fractional part (e.g., 0.1, 0.01)

    char displayBuffer[20];  // Separate buffer for formatted output

    while (keypad->activate)
    {
        uint8_t key = KEYPAD_Read_ASM();  // Read the current key (0–12)

        if (key != 0xFF)  // Check if a valid key was pressed
        {
            if (key >= 0 && key <= 9)  // Numbers 0–9
            {
                if (counter < sizeof(keypad->nokiaOutput) - 1)  // Prevent buffer overflow
                {
                    // Convert numeric key to ASCII and append to nokiaOutput
                    keypad->nokiaOutput[counter] = '0' + key;
                    counter++;
                    keypad->nokiaOutput[counter] = '\0';  // Ensure null-termination
                }

                if (dot_pressed)
                {
                    // Handle the fractional part
                    fraction_part += key * (1.0 / fraction_multiplier);
                    fraction_multiplier *= 10;
                }
                else
                {
                    // Handle the integer part
                    number = (number * 10) + key;
                }

                // Format the output with the label and current number
                snprintf(displayBuffer, sizeof(displayBuffer), "BMP280:%s", keypad->nokiaOutput);

                Nokia5110_Clear();
                Nokia5110_SetCursor(0, 0);
                Nokia5110_OutString(displayBuffer);  // Print formatted string to LCD
								
            }
            else if (key == 10 && !dot_pressed)  // Key 10: Dot (.) pressed
            {
                dot_pressed = 1;  // Mark that the dot has been pressed
                fraction_multiplier = 10;  // Initialize fractional multiplier

                if (counter < sizeof(keypad->nokiaOutput) - 1)
                {
                    keypad->nokiaOutput[counter] = '.';
                    counter++;
                    keypad->nokiaOutput[counter] = '\0';  // Ensure null-termination
                }

                // Format the output with the label and current number
                snprintf(displayBuffer, sizeof(displayBuffer), "BMP280:%s", keypad->nokiaOutput);

                Nokia5110_Clear();
                Nokia5110_SetCursor(0, 0);
                Nokia5110_OutString(displayBuffer);  // Print formatted string to LCD
            }
            else if (key == 11)  // Key 11: Enter deep sleep
            {
                Nokia5110_Clear();
                Nokia5110_SetCursor(0, 0);
                Nokia5110_OutString("Entering deep sleep...");
								//keypad->deepsleepActivate=0;
								keypad->deepSleepActivate = 0;
                return;  // Return immediately as deep sleep is triggered
            }
        }
    }

    // Combine integer and fractional parts and return the result
    number += fraction_part;
		bmp280->init();
		
    if (number == 0)
    {
        keypad->number = numberReturn;
    }
    else
    {
        keypad->number = number;
    }
		bmp280->deinit();
		bmp280->init();
		numberReturn = keypad->number;
		bmp280->thresholdTemperature = keypad->number;
		
		
    return;
}
static void deinit()
{
		
    // Disable GPIO interrupts for the keypad (e.g., PD3)
    GPIOD->IM &= ~(1 << KEYPAD_ENTER_PIN_NUM);  // Mask interrupt for PD3

    // Clear pending interrupts for PD3 (if any)
    GPIOD->ICR = (1 << KEYPAD_ENTER_PIN_NUM);

    // Optionally, reset GPIO pin configurations to their default state
    // Reset row pins (KEYPAD_ROW_1_PIN, KEYPAD_ROW_2_PIN, KEYPAD_ROW_3_PIN) to inputs
    KEYPAD_ROW_1_PIN->DIR &= ~(1 << KEYPAD_ROW_1_PIN_NUM);
    KEYPAD_ROW_1_PIN->DEN &= ~(1 << KEYPAD_ROW_1_PIN_NUM);

    KEYPAD_ROW_2_PIN->DIR &= ~(1 << KEYPAD_ROW_2_PIN_NUM);
    KEYPAD_ROW_2_PIN->DEN &= ~(1 << KEYPAD_ROW_2_PIN_NUM);

    KEYPAD_ROW_3_PIN->DIR &= ~(1 << KEYPAD_ROW_3_PIN_NUM);
    KEYPAD_ROW_3_PIN->DEN &= ~(1 << KEYPAD_ROW_3_PIN_NUM);

    // Reset column pins (KEYPAD_COL_1_PIN, KEYPAD_COL_2_PIN, KEYPAD_COL_3_PIN, KEYPAD_COL_4_PIN) to inputs
    KEYPAD_COL_1_PIN->DIR &= ~(1 << KEYPAD_COL_1_PIN_NUM);
    KEYPAD_COL_1_PIN->DEN &= ~(1 << KEYPAD_COL_1_PIN_NUM);

    KEYPAD_COL_2_PIN->DIR &= ~(1 << KEYPAD_COL_2_PIN_NUM);
    KEYPAD_COL_2_PIN->DEN &= ~(1 << KEYPAD_COL_2_PIN_NUM);

    KEYPAD_COL_3_PIN->DIR &= ~(1 << KEYPAD_COL_3_PIN_NUM);
    KEYPAD_COL_3_PIN->DEN &= ~(1 << KEYPAD_COL_3_PIN_NUM);

    KEYPAD_COL_4_PIN->DIR &= ~(1 << KEYPAD_COL_4_PIN_NUM);
    KEYPAD_COL_4_PIN->DEN &= ~(1 << KEYPAD_COL_4_PIN_NUM);

    // Reset the enter pin (PD3) to input with no interrupt configuration
    KEYPAD_ENTER_PIN->DIR &= ~(1 << KEYPAD_ENTER_PIN_NUM);
    KEYPAD_ENTER_PIN->DEN &= ~(1 << KEYPAD_ENTER_PIN_NUM);
    KEYPAD_ENTER_PIN->PUR &= ~(1 << KEYPAD_ENTER_PIN_NUM); // Disable pull-up resistor

    // Optionally reset keypad active state
    keypad->activate = 0;
		keypad->number=-1;
		keypad->key=0xFF;


    // Clear the result buffer
    memset((void *)result, 0, sizeof(result));

    // Disable NVIC interrupt for GPIO Port D (IRQ #3)
    NVIC->ICER[0] = (1 << 3);

    // Optional: Clear pending interrupts in NVIC for GPIO Port D
    NVIC->ICPR[0] = (1 << 3);
		GPIOD->LOCK=0;
}



__attribute__((constructor))
static void KEYPAD_Setup_Pointers(void){
		keypad_instance.init=initGPIO;
		keypad_instance.read=readKEYPAD;
		keypad_instance.deinit=deinit;
};