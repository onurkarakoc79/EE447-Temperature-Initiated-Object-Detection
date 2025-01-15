#include "HCSR04.h"
#include "TM4C123.h"
#include "PINS.h" // Adjust if needed for your pin macros

/*--------------------------------------------------
 *   Configuration Macros
 *--------------------------------------------------*/
#define TIMER_LOW    320
#define TIMER_HIGH   80

#define ONE_METER_TIMEOUT_US 6000  // 6 ms = 6000 µs
/*--------------------------------------------------
 *   Forward Declarations of Internal Helpers
 *--------------------------------------------------*/
static void initTimer(void);
static void initGPIO(void);
static void changeTimerBToNegEdge(void);
static void changeTimerBToPosEdge(void);
static void HCSR04_Deinit_Impl(void);

static int waitForTimerEdge(uint32_t edgeMask, uint32_t maxCount);
// The ISR may need to remain non-static depending on your startup code / vector table.
void TIMER0A_Handler(void);

/*--------------------------------------------------
 *   Single Static Instance of HCSR04_Struct
 *--------------------------------------------------*/
static HCSR04_Struct hcsr04_instance = {
    .timerLow  = TIMER_LOW,
    .timerHigh = TIMER_HIGH,
    .periodInt = 0,
    .distance  = 0,
    .rawDistance = 0,
    .pulseWidth = 0,
    .dutyCycle  = 0,
    .period     = 0
};

/*--------------------------------------------------
 *   Global Pointer to the Single Instance
 *--------------------------------------------------*/
HCSR04_Struct* const hcsr04 = &hcsr04_instance;

/*--------------------------------------------------
 *   Actual Implementation of init and read
 *   (Assigned to hcsr04->init, hcsr04->read)
 *--------------------------------------------------*/
static void HCSR04_Init_Impl(void)
{
    // Re-initialize struct fields as needed
    hcsr04_instance.timerLow    = TIMER_LOW;
    hcsr04_instance.timerHigh   = TIMER_HIGH;
    hcsr04_instance.distance    = 0;
    hcsr04_instance.dutyCycle   = 0;
    hcsr04_instance.periodInt   = 0;
    hcsr04_instance.pulseWidth  = 0;
    hcsr04_instance.rawDistance = 0;
    hcsr04_instance.period      = 0;

    // Hardware inits
    initGPIO();
    initTimer();
}
static void HCSR04_Read_Impl(float temperature)
{

    // 1) Wait for the first edge (rising edge) with timeout
    if (!waitForTimerEdge((1 << 10), ONE_METER_TIMEOUT_US))
    {
        // We timed out => distance > 1m
        // Handle it (e.g. set distance to 999 cm or something)
        hcsr04_instance.distance = 100.0f; 
        return;
    }
    uint32_t risingEdge1 = HCSR04_TIMER->TBR;
    HCSR04_TIMER->ICR = (1 << 10);

    // 2) Wait for the second edge (rising edge) with timeout
    if (!waitForTimerEdge((1 << 10), ONE_METER_TIMEOUT_US))
    {
        // Timed out => distance > 1m
        hcsr04_instance.distance = 100.0f;
        return;
    }
    uint32_t risingEdge2 = HCSR04_TIMER->TBR;
    HCSR04_TIMER->ICR = (1 << 10);

    // Overflow check
    if (risingEdge2 < risingEdge1) {
        risingEdge2 += 0xFFFF;
    }

    // 3) Switch to falling edge
    changeTimerBToNegEdge();

    // 4) Wait for the falling edge with timeout
    if (!waitForTimerEdge((1 << 10), ONE_METER_TIMEOUT_US))
    {
        // Timed out => distance > 1m
        hcsr04_instance.distance = 100.0f;
        // Switch back to rising edge detection before returning
        changeTimerBToPosEdge();
        return;
    }
    uint32_t fallingEdge = HCSR04_TIMER->TBR;
    HCSR04_TIMER->ICR = (1 << 10);

    // Switch back to rising edge detection
    changeTimerBToPosEdge();

    // Overflow check
    if (fallingEdge < risingEdge2) {
        fallingEdge += 0xFFFF;
    }

    // Now compute the pulseWidth and distance
    float soundSpeed = 331.4f + 0.6f * temperature; // m/s
    float soundSpeedCMPerUS = soundSpeed / 10000.0f; // cm/µs

    hcsr04_instance.periodInt  = (float)(risingEdge2 - risingEdge1);
    hcsr04_instance.period     = hcsr04_instance.periodInt / 16.0f;
    hcsr04_instance.pulseWidth = 
         (float)((fallingEdge - risingEdge2) % hcsr04_instance.periodInt) / 16.0f;
    hcsr04_instance.dutyCycle  = (hcsr04_instance.pulseWidth * 100.0f) 
                                 / hcsr04_instance.period;

    // Distance in cm (divide by 2 for one-way)
    hcsr04_instance.distance = (hcsr04_instance.pulseWidth * soundSpeedCMPerUS) 
                               / 2.0f;
}


/*--------------------------------------------------
 *   Constructor-Like Setup for Function Pointers
 *--------------------------------------------------*/
__attribute__((constructor))
static void HCSR04_SetupPointers(void)
{
    hcsr04_instance.init = HCSR04_Init_Impl;
    hcsr04_instance.read = HCSR04_Read_Impl;
		hcsr04_instance.deinit = HCSR04_Deinit_Impl;
}

/*--------------------------------------------------
 *   Internal Helper Functions
 *--------------------------------------------------*/
static void initTimer(void)
{
    // Enable Timer (assumes HCSR04_TIMER_NUM is defined somewhere)
    SYSCTL->RCGCTIMER |= (1 << HCSR04_TIMER_NUM);
    __ASM("NOP"); __ASM("NOP"); __ASM("NOP");
    HCSR04_TIMER->CTL &= ~(1 << 0); // Stop timer A to initialize
    __ASM("NOP"); __ASM("NOP"); __ASM("NOP");

    // Configure timer A for periodic mode
    HCSR04_TIMER->CFG  |= (1 << 2);  // 16-bit
    HCSR04_TIMER->TAMR |= (1 << 1);  // Periodic mode
    HCSR04_TIMER->TAILR = hcsr04_instance.timerLow;
    HCSR04_TIMER->TAPR  = 15;        // Prescaler
    HCSR04_TIMER->IMR  |= (1 << 0);  // Interrupt on timeout

    NVIC->IPR[19]  = (1 << 6);
    NVIC->ISER[0]  = (1 << 19);

    // Configure timer B for edge-time capture (echo pin)
    HCSR04_TIMER->CTL &= ~(1 << 8);  // Stop timer B
    __ASM("NOP"); __ASM("NOP"); __ASM("NOP");
    HCSR04_TIMER->CFG  |= (1 << 2);
    HCSR04_TIMER->TBMR |= (7 << 0) | (1 << 4); // Edge-time, count up
    HCSR04_TIMER->CTL  &= ~(3 << 10);          // capture positive edges
    HCSR04_TIMER->CTL  |= (1 << 0) | (1 << 8); // Re-enable timers
    __ASM("NOP"); __ASM("NOP"); __ASM("NOP");
}

static void initGPIO(void)
{
    // Enable GPIO clock for trig pin
    SYSCTL->RCGCGPIO |= (1 << HCSR04_TRIG_PIN_BASE_NUM);
    while ((SYSCTL->PRGPIO & (1 << HCSR04_TRIG_PIN_BASE_NUM)) == 0);

    // Configure trig pin as GPIO output
    HCSR04_TRIG_PIN->DIR   |=  (1 << HCSR04_TRIG_PIN_NUM);
    HCSR04_TRIG_PIN->AFSEL &= ~(1 << HCSR04_TRIG_PIN_NUM);
    HCSR04_TRIG_PIN->PCTL  &=  (0 << (HCSR04_TRIG_PIN_NUM*4));
    HCSR04_TRIG_PIN->AMSEL  =  0;
    HCSR04_TRIG_PIN->DEN   |=  (1 << HCSR04_TRIG_PIN_NUM);
	
		SYSCTL->RCGCGPIO |= (1<<HCSR04_ECHO_PIN_BASE_NUM);
		while ((SYSCTL->PRGPIO & (1<< HCSR04_ECHO_PIN_BASE_NUM))==0);
		
		HCSR04_ECHO_PIN->DIR &= ~(1<<HCSR04_ECHO_PIN_NUM);
		HCSR04_ECHO_PIN->AFSEL |=(1<<HCSR04_ECHO_PIN_NUM);
		HCSR04_ECHO_PIN->AMSEL &= ~(1<<HCSR04_ECHO_PIN_NUM);
		HCSR04_ECHO_PIN->DEN |= (1<<HCSR04_ECHO_PIN_NUM);
		HCSR04_ECHO_PIN->PCTL &= ~(0xF<<HCSR04_ECHO_PIN_NUM*4);
		HCSR04_ECHO_PIN->PCTL |= ((0x7 & 0xF) <<HCSR04_ECHO_PIN_NUM*4); // enable gpio as timer pin 0x07 is timer mode 
	
}

static void changeTimerBToNegEdge(void)
{
    HCSR04_TIMER->CTL &= ~(1 << 8);
    __ASM("NOP"); __ASM("NOP"); __ASM("NOP");
    HCSR04_TIMER->CTL &= ~(3 << 10); // Clear event mode
    HCSR04_TIMER->CTL |=  (1 << 10); // Set to falling-edge capture
    HCSR04_TIMER->CTL |=  (1 << 8);  // Re-enable timer B
}

static void changeTimerBToPosEdge(void)
{
    HCSR04_TIMER->CTL &= ~(1 << 8);
    __ASM("NOP"); __ASM("NOP"); __ASM("NOP");
    HCSR04_TIMER->CTL &= ~(3 << 10);
    HCSR04_TIMER->CTL |=  (1 << 8);  // Re-enable timer B (pos edge by default)
}

/*--------------------------------------------------
 *   Interrupt Service Routine
 *   Adjust name if your startup code expects a different name
 *--------------------------------------------------*/
void TIMER0A_Handler(void)
{
    // Handle timer A timeout toggling the trig pin (per your original logic)
    //HCSR04_TIMER->CTL &= ~(1 << 0);
    HCSR04_TIMER->ICR = 0x01;

    if (HCSR04_TRIG_PIN->DATA & (1 << HCSR04_TRIG_PIN_NUM)) {
        // Turn trig pin off
        HCSR04_TRIG_PIN->DATA &= ~(1 << HCSR04_TRIG_PIN_NUM);
        HCSR04_TIMER->TAILR = hcsr04_instance.timerLow;
    } else {
        // Turn trig pin on
        HCSR04_TRIG_PIN->DATA |= (1 << HCSR04_TRIG_PIN_NUM);
        HCSR04_TIMER->TAILR = hcsr04_instance.timerHigh;
    }

    //HCSR04_TIMER->CTL |= (1 << 0);
    return;
}

static int waitForTimerEdge(uint32_t edgeMask, uint32_t maxCount)
{
    uint32_t count = 0;
    while ((HCSR04_TIMER->RIS & edgeMask) == 0)
    {
        // ~ Wait a bit
        count++;
        if (count > maxCount)
        {
            return 0; // timed out
        }
    }
    return 1;
}

static void HCSR04_Deinit_Impl(void)
{
    //
    // 1) Disable interrupts for Timer 0 (or the relevant timer)
    //
    HCSR04_TIMER->IMR  &= ~((1 << 0));   // Disable TIMER A interrupt
    // If you prefer, also disable capture interrupt for Timer B
    // HCSR04_TIMER->IMR &= ~(1 << something_for_timerB_capture);

    // Disable the NVIC vector if you want
    NVIC->ICER[0] = (1 << 19);  // Timer0A interrupt vector is #19 on TM4C123
    
    //
    // 2) Disable Timer
    //
    HCSR04_TIMER->CTL &= ~((1 << 0) | (1 << 8));  // Stop Timer A and Timer B

    //
    // 3) Optionally set the timer “disable” (if you want to remove power to Timer 0)
    //
    // Be cautious: If anything else in your system still needs Timer0,
    // you shouldn't disable it entirely. But, if safe:
    SYSCTL->RCGCTIMER &= ~(1 << HCSR04_TIMER_NUM);

    //
    // 4) Reconfigure GPIO as needed (optional)
    //
    // For instance, set trig pin to input, or to a low-power state:
    HCSR04_TRIG_PIN->DIR &= ~(1 << HCSR04_TRIG_PIN_NUM);
    HCSR04_TRIG_PIN->DATA &= ~(1 << HCSR04_TRIG_PIN_NUM);
    // Possibly disable digital enable or reassign AFSEL, etc.

    // For Echo pin, revert to a default safe state, e.g. GPIO input disabled or input:
    HCSR04_ECHO_PIN->DIR &= ~(1 << HCSR04_ECHO_PIN_NUM);
    HCSR04_ECHO_PIN->AFSEL &= ~(1 << HCSR04_ECHO_PIN_NUM);
    HCSR04_ECHO_PIN->DEN &= ~(1 << HCSR04_ECHO_PIN_NUM);
		hcsr04_instance.distance=0.0;
    // Or your desired “safe” state…

    //
    // 5) Optionally clear function pointers to prevent further usage
    //
  
}
