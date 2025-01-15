

#include <stdint.h>
#include "TM4C123GH6PM.h"
#include "Nokia5110.h"
#include "stdio.h"
// Define boundaries of the plot
#define X_MIN -100
#define X_MAX 100
#define Y_MIN 0
#define Y_MAX 100
// Define Control Pins for Nokia5110
#define DC                      (*((volatile uint32_t *)0x40004100))  // PA6
#define DC_COMMAND              0
#define DC_DATA                 0x40
#define RESET                   (*((volatile uint32_t *)0x40004200))  // PA7
#define RESET_LOW               0
#define RESET_HIGH              0x80

// Define SSI and GPIO Flags
#define SSI_SR_BSY              0x00000010  // SSI Busy
#define SSI_SR_TNF              0x00000002  // Transmit FIFO Not Full
#define SYSCTL_RCGCSSI_R0       0x00000001  // SSI Module 0 Run Mode Clock Gating Control
#define SYSCTL_RCGCGPIO_R0      0x00000001  // GPIO Port A Run Mode Clock Gating Control
#define SSI_CR0_FRF_MOTO        0x00000000  // Freescale SPI Frame Format
#define SSI_CR0_DSS_8           0x00000007  // 8-bit data size
#define SSI_CR1_SSE             0x00000002  // Synchronous Serial Port Enable

// ASCII character to pixel map
const uint8_t ASCII[96][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, /* Space */
    {0x00, 0x00, 0x5F, 0x00, 0x00}, /* ! */
    {0x00, 0x03, 0x00, 0x03, 0x00}, /* " */
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, /* # */
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, /* $ */
    {0x23, 0x13, 0x08, 0x64, 0x62}, /* % */
    {0x36, 0x49, 0x55, 0x22, 0x50}, /* & */
    {0x00, 0x05, 0x03, 0x00, 0x00}, /* ' */
    {0x00, 0x1C, 0x22, 0x41, 0x00}, /* ( */
    {0x00, 0x41, 0x22, 0x1C, 0x00}, /* ) */
    {0x14, 0x08, 0x3E, 0x08, 0x14}, /* * */
    {0x08, 0x08, 0x3E, 0x08, 0x08}, /* + */
    {0x00, 0x50, 0x30, 0x00, 0x00}, /* , */
    {0x08, 0x08, 0x08, 0x08, 0x08}, /* - */
    {0x00, 0x60, 0x60, 0x00, 0x00}, /* . */
    {0x20, 0x10, 0x08, 0x04, 0x02}, /* / */
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, /* 0 */
    {0x00, 0x42, 0x7F, 0x40, 0x00}, /* 1 */
    {0x42, 0x61, 0x51, 0x49, 0x46}, /* 2 */
    {0x21, 0x41, 0x45, 0x4B, 0x31}, /* 3 */
    {0x18, 0x14, 0x12, 0x7F, 0x10}, /* 4 */
    {0x27, 0x45, 0x45, 0x45, 0x39}, /* 5 */
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, /* 6 */
    {0x01, 0x71, 0x09, 0x05, 0x03}, /* 7 */
    {0x36, 0x49, 0x49, 0x49, 0x36}, /* 8 */
    {0x06, 0x49, 0x49, 0x29, 0x1E}, /* 9 */
    {0x00, 0x36, 0x36, 0x00, 0x00}, /* : */
    {0x00, 0x56, 0x36, 0x00, 0x00}, /* ; */
    {0x08, 0x14, 0x22, 0x41, 0x00}, /* < */
    {0x14, 0x14, 0x14, 0x14, 0x14}, /* = */
    {0x00, 0x41, 0x22, 0x14, 0x08}, /* > */
    {0x02, 0x01, 0x51, 0x09, 0x06}, /* ? */
    {0x3E, 0x41, 0x5D, 0x59, 0x4E}, /* @ */
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, /* A */
    {0x7F, 0x49, 0x49, 0x49, 0x36}, /* B */
    {0x3E, 0x41, 0x41, 0x41, 0x22}, /* C */
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, /* D */
    {0x7F, 0x49, 0x49, 0x49, 0x41}, /* E */
    {0x7F, 0x09, 0x09, 0x09, 0x01}, /* F */
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, /* G */
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, /* H */
    {0x00, 0x41, 0x7F, 0x41, 0x00}, /* I */
    {0x20, 0x40, 0x41, 0x3F, 0x01}, /* J */
    {0x7F, 0x08, 0x14, 0x22, 0x41}, /* K */
    {0x7F, 0x40, 0x40, 0x40, 0x40}, /* L */
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, /* M */
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, /* N */
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, /* O */
    {0x7F, 0x09, 0x09, 0x09, 0x06}, /* P */
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, /* Q */
    {0x7F, 0x09, 0x19, 0x29, 0x46}, /* R */
    {0x26, 0x49, 0x49, 0x49, 0x32}, /* S */
    {0x01, 0x01, 0x7F, 0x01, 0x01}, /* T */
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, /* U */
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, /* V */
    {0x3F, 0x40, 0x30, 0x40, 0x3F}, /* W */
    {0x63, 0x14, 0x08, 0x14, 0x63}, /* X */
    {0x07, 0x08, 0x70, 0x08, 0x07}, /* Y */
    {0x61, 0x51, 0x49, 0x45, 0x43}, /* Z */
    {0x00, 0x7F, 0x41, 0x41, 0x00}, /* [ */
    {0x02, 0x04, 0x08, 0x10, 0x20}, /* \ */
    {0x00, 0x41, 0x41, 0x7F, 0x00}, /* ] */
    {0x04, 0x02, 0x01, 0x02, 0x04}, /* ^ */
    {0x40, 0x40, 0x40, 0x40, 0x40}, /* _ */
    {0x00, 0x01, 0x02, 0x04, 0x00}, /* ` */
    {0x20, 0x54, 0x54, 0x54, 0x78}, /* a */
    {0x7F, 0x48, 0x44, 0x44, 0x38}, /* b */
    {0x38, 0x44, 0x44, 0x44, 0x20}, /* c */
    {0x38, 0x44, 0x44, 0x48, 0x7F}, /* d */
    {0x38, 0x54, 0x54, 0x54, 0x18}, /* e */
    {0x08, 0x7E, 0x09, 0x01, 0x02}, /* f */
    {0x08, 0x14, 0x54, 0x54, 0x3C}, /* g */
    {0x7F, 0x08, 0x04, 0x04, 0x78}, /* h */
    {0x00, 0x44, 0x7D, 0x40, 0x00}, /* i */
    {0x20, 0x40, 0x44, 0x3D, 0x00}, /* j */
    {0x7F, 0x10, 0x28, 0x44, 0x00}, /* k */
    {0x00, 0x41, 0x7F, 0x40, 0x00}, /* l */
    {0x7C, 0x04, 0x18, 0x04, 0x78}, /* m */
    {0x7C, 0x08, 0x04, 0x04, 0x78}, /* n */
    {0x38, 0x44, 0x44, 0x44, 0x38}, /* o */
    {0x7C, 0x14, 0x14, 0x14, 0x08}, /* p */
    {0x08, 0x14, 0x14, 0x18, 0x7C}, /* q */
    {0x7C, 0x08, 0x04, 0x04, 0x08}, /* r */
    {0x48, 0x54, 0x54, 0x54, 0x20}, /* s */
    {0x04, 0x3F, 0x44, 0x40, 0x20}, /* t */
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, /* u */
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, /* v */
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, /* w */
    {0x44, 0x28, 0x10, 0x28, 0x44}, /* x */
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, /* y */
    {0x44, 0x64, 0x54, 0x4C, 0x44}, /* z */
    {0x00, 0x08, 0x36, 0x41, 0x00}, /* { */
    {0x00, 0x00, 0x7F, 0x00, 0x00}, /* | */
    {0x00, 0x41, 0x36, 0x08, 0x00}, /* } */
    {0x08, 0x08, 0x2A, 0x1C, 0x08}, /* ~ */
};

const float sin_values[181] = {
    0.0000, 0.0175, 0.0349, 0.0523, 0.0698, 0.0872, 0.1045, 0.1219, 0.1392, 0.1564, 
    0.1736, 0.1908, 0.2079, 0.2250, 0.2419, 0.2588, 0.2756, 0.2924, 0.3090, 0.3256, 
    0.3420, 0.3584, 0.3746, 0.3907, 0.4067, 0.4226, 0.4384, 0.4540, 0.4695, 0.4848, 
    0.5000, 0.5150, 0.5299, 0.5446, 0.5592, 0.5736, 0.5878, 0.6018, 0.6157, 0.6293, 
    0.6428, 0.6561, 0.6691, 0.6820, 0.6947, 0.7071, 0.7193, 0.7314, 0.7431, 0.7547, 
    0.7660, 0.7771, 0.7880, 0.7986, 0.8090, 0.8192, 0.8290, 0.8387, 0.8480, 0.8572, 
    0.8660, 0.8746, 0.8829, 0.8910, 0.8988, 0.9063, 0.9135, 0.9205, 0.9272, 0.9336, 
    0.9397, 0.9455, 0.9511, 0.9563, 0.9613, 0.9659, 0.9703, 0.9744, 0.9781, 0.9816, 
    0.9848, 0.9877, 0.9903, 0.9925, 0.9945, 0.9962, 0.9976, 0.9986, 0.9994, 0.9998, 
    1.0000, 0.9998, 0.9994, 0.9986, 0.9976, 0.9962, 0.9945, 0.9925, 0.9903, 0.9877, 
    0.9848, 0.9816, 0.9781, 0.9744, 0.9703, 0.9659, 0.9613, 0.9563, 0.9511, 0.9455, 
    0.9397, 0.9336, 0.9272, 0.9205, 0.9135, 0.9063, 0.8988, 0.8910, 0.8829, 0.8746, 
    0.8660, 0.8572, 0.8480, 0.8387, 0.8290, 0.8192, 0.8090, 0.7986, 0.7880, 0.7771, 
    0.7660, 0.7547, 0.7431, 0.7314, 0.7193, 0.7071, 0.6947, 0.6820, 0.6691, 0.6561, 
    0.6428, 0.6293, 0.6157, 0.6018, 0.5878, 0.5736, 0.5592, 0.5446, 0.5299, 0.5150, 
    0.5000, 0.4848, 0.4695, 0.4540, 0.4384, 0.4226, 0.4067, 0.3907, 0.3746, 0.3584, 
    0.3420, 0.3256, 0.3090, 0.2924, 0.2756, 0.2588, 0.2419, 0.2250, 0.2079, 0.1908, 
    0.1736, 0.1564, 0.1392, 0.1219, 0.1045, 0.0872, 0.0698, 0.0523, 0.0349, 0.0175, 
    0.0000
};

// Precomputed cosine values for angles from 0? to 180?
const float cos_values[181] = {
    1.0000, 0.9998, 0.9994, 0.9986, 0.9976, 0.9962, 0.9945, 0.9925, 0.9903, 0.9877, 
    0.9848, 0.9816, 0.9781, 0.9744, 0.9703, 0.9659, 0.9613, 0.9563, 0.9511, 0.9455, 
    0.9397, 0.9336, 0.9272, 0.9205, 0.9135, 0.9063, 0.8988, 0.8910, 0.8829, 0.8746, 
    0.8660, 0.8572, 0.8480, 0.8387, 0.8290, 0.8192, 0.8090, 0.7986, 0.7880, 0.7771, 
    0.7660, 0.7547, 0.7431, 0.7314, 0.7193, 0.7071, 0.6947, 0.6820, 0.6691, 0.6561, 
    0.6428, 0.6293, 0.6157, 0.6018, 0.5878, 0.5736, 0.5592, 0.5446, 0.5299, 0.5150, 
    0.5000, 0.4848, 0.4695, 0.4540, 0.4384, 0.4226, 0.4067, 0.3907, 0.3746, 0.3584, 
    0.3420, 0.3256, 0.3090, 0.2924, 0.2756, 0.2588, 0.2419, 0.2250, 0.2079, 0.1908, 
    0.1736, 0.1564, 0.1392, 0.1219, 0.1045, 0.0872, 0.0698, 0.0523, 0.0349, 0.0175, 
    0.0000, -0.0175, -0.0349, -0.0523, -0.0698, -0.0872, -0.1045, -0.1219, -0.1392, -0.1564, 
    -0.1736, -0.1908, -0.2079, -0.2250, -0.2419, -0.2588, -0.2756, -0.2924, -0.3090, -0.3256, 
    -0.3420, -0.3584, -0.3746, -0.3907, -0.4067, -0.4226, -0.4384, -0.4540, -0.4695, -0.4848, 
    -0.5000, -0.5150, -0.5299, -0.5446, -0.5592, -0.5736, -0.5878, -0.6018, -0.6157, -0.6293, 
    -0.6428, -0.6561, -0.6691, -0.6820, -0.6947, -0.7071, -0.7193, -0.7314, -0.7431, -0.7547, 
    -0.7660, -0.7771, -0.7880, -0.7986, -0.8090, -0.8192, -0.8290, -0.8387, -0.8480, -0.8572, 
    -0.8660, -0.8746, -0.8829, -0.8910, -0.8988, -0.9063, -0.9135, -0.9205, -0.9272, -0.9336, 
    -0.9397, -0.9455, -0.9511, -0.9563, -0.9613, -0.9659, -0.9703, -0.9744, -0.9781, -0.9816, 
    -0.9848, -0.9877, -0.9903, -0.9925, -0.9945, -0.9962, -0.9976, -0.9986, -0.9994, -0.9998, 
    -1.0000
};

// Enum for Command/Data Types
enum typeOfWrite {
    COMMAND,  // LCD command
    DATA      // LCD data
};

// Function Prototypes
void Nokia5110_Init(void);
void lcdwrite(enum typeOfWrite type, char message);
void Nokia5110_OutChar(char data);
void Nokia5110_OutString(char *ptr);
void Nokia5110_Clear(void);
void Nokia5110_SetCursor(unsigned char newX, unsigned char newY);
/*
void Nokia5110_Init(void) {
    volatile uint32_t delay;

    // Enable clocks for SSI0 and GPIOA
    SYSCTL->RCGCSSI |= SYSCTL_RCGCSSI_R0;    // Enable SSI0 clock
    SYSCTL->RCGCGPIO |= SYSCTL_RCGCGPIO_R0;  // Enable Port A clock
    delay = SYSCTL->RCGCGPIO;                // Allow time for clock to stabilize

    // Configure GPIOA Pins for SSI0 and Control
    GPIOA->AFSEL |= 0x2C;    // Enable alternate function on PA2, PA3, PA5 (SSI0)
    GPIOA->AFSEL &= ~0xC0;   // Disable alternate function on PA6, PA7 (GPIO control)
    GPIOA->DEN |= 0xEC;      // Enable digital functionality on PA2, PA3, PA5, PA6, PA7
    GPIOA->PCTL = (GPIOA->PCTL & 0xFF0F00FF) | 0x00202200;  // Configure PA2, PA3, PA5 for SSI
    GPIOA->AMSEL &= ~0xEC;   // Disable analog functionality on PA2, PA3, PA5, PA6, PA7
    GPIOA->DIR |= 0xC0;      // Make PA6, PA7 outputs (DC and RESET)

    // Configure SSI0
    SSI0->CR1 = 0;                          // Disable SSI0 and configure as master
    SSI0->CC = 0;                           // Use system clock
    SSI0->CPSR = 16;                        // Set clock prescale to divide by 16 (3.125 MHz)
    SSI0->CR0 = SSI_CR0_FRF_MOTO |          // Freescale SPI frame format
                SSI_CR0_DSS_8;              // 8-bit data size
    SSI0->CR1 |= SSI_CR1_SSE;               // Enable SSI0

    // Reset the Nokia5110
    RESET = RESET_LOW;                       // Pull RESET low
    for (delay = 0; delay < 10; delay++) {}; // Delay for a short time
    RESET = RESET_HIGH;                      // Release RESET

    // Initialize Nokia5110 LCD
		
    lcdwrite(COMMAND, 0x21);                 // Use extended instruction set
   lcdwrite(COMMAND, 0x8F);                 // Bora 8F diye ayarladi Set contrast (adjustable if needed)// ffSI BAYAAAAAAA KRITIK
    lcdwrite(COMMAND, 0x04);                 // Set temperature coefficient
    lcdwrite(COMMAND, 0x14);                 // Set bias mode
    lcdwrite(COMMAND, 0x20);                 // Use basic instruction set
    lcdwrite(COMMAND, 0x0C);                 // Set normal display mode
}

void Nokia5110_DeInit(void) {
    volatile uint32_t delay;

    // Step 1: Ensure clock for SSI0 and GPIOA is enabled
    SYSCTL->RCGCSSI |= SYSCTL_RCGCSSI_R0;    // Enable SSI0 clock
    SYSCTL->RCGCGPIO |= SYSCTL_RCGCGPIO_R0;  // Enable Port A clock
    delay = SYSCTL->RCGCGPIO;                // Allow time for clock to stabilize

    // Step 2: Disable SSI0
    SSI0->CR1 = 0;  // Disable SSI0 by clearing the SSE bit

    // Step 3: Reset GPIOA Pins used for Nokia 5110
    GPIOA->AFSEL &= ~0xEC;   // Disable alternate functions on PA2, PA3, PA5, PA6, PA7
    GPIOA->PCTL &= ~0xFFF0FFF0; // Clear PCTL configuration for PA2, PA3, PA5, PA6, PA7
    GPIOA->AMSEL &= ~0xEC;   // Disable analog functionality on PA2, PA3, PA5, PA6, PA7
    GPIOA->DIR &= ~0xC0;     // Set PA6 and PA7 as inputs (control pins)
    GPIOA->DEN &= ~0xEC;     // Disable digital functionality on PA2, PA3, PA5, PA6, PA7

    // Step 4: Reset the Nokia5110 (optional but safer)
    RESET = RESET_LOW;       // Pull RESET low to put the display in reset state

    // Step 5: Disable clocks for SSI0 and GPIOA (only if not used elsewhere)
    SYSCTL->RCGCSSI &= ~SYSCTL_RCGCSSI_R0;    // Disable SSI0 clock
    SYSCTL->RCGCGPIO &= ~SYSCTL_RCGCGPIO_R0;  // Disable Port A clock
}



void lcdwrite(enum typeOfWrite type, char message) {
    if (type == COMMAND) {
        while ((SSI0->SR & SSI_SR_BSY) == SSI_SR_BSY) {};  // Wait until SSI0 not busy
        DC = DC_COMMAND;                                   // Set DC low for command
        SSI0->DR = message;                                // Send command
        while ((SSI0->SR & SSI_SR_BSY) == SSI_SR_BSY) {};  // Wait until done
    } else {
        while ((SSI0->SR & SSI_SR_TNF) == 0) {};           // Wait until FIFO not full
        DC = DC_DATA;                                      // Set DC high for data
        SSI0->DR = message;                                // Send data
    }
}

void Nokia5110_Clear(void) {
    int i;

    // Set cursor to (0,0) for writing
    Nokia5110_SetCursor(0, 0);

    // Write zeros to the entire screen
    for (i = 0; i < (MAX_X * MAX_Y / 8); i++) {
        lcdwrite(DATA, 0x00);
    }

    // Reset cursor to (0,0)
    Nokia5110_SetCursor(0, 0);
}


void Nokia5110_SetCursor(unsigned char newX, unsigned char newY) {
    if ((newX > 11) || (newY > 5)) return;  // Invalid cursor position
    lcdwrite(COMMAND, 0x80 | (newX * 7));   // Set X-position
    lcdwrite(COMMAND, 0x40 | newY);         // Set Y-position
}

void Nokia5110_OutChar(char data) {
    int i;
    lcdwrite(DATA, 0x00);                 // Blank vertical line padding
    for (i = 0; i < 5; i++) {
        lcdwrite(DATA, ASCII[data - 0x20][i]);
    }
    lcdwrite(DATA, 0x00);                 // Blank vertical line padding
}

void Nokia5110_OutString(char *ptr) {
    unsigned char x = 0, y = 0;  // Current cursor position (x, y)
    while (*ptr) {
        if (*ptr == '\n') {  // Handle newline character
            x = 0;           // Reset x to the beginning of the line
            if (y < 5) {     // Move to the next row if not at the bottom
                y++;
            }
            Nokia5110_SetCursor(x, y);
        } else {  // Regular character
            Nokia5110_OutChar(*ptr);
            x++;  // Move cursor to the next character position
            if (x > 11) {  // Line overflow, wrap to the next line
                x = 0;
                if (y < 5) {
                    y++;
                }
                Nokia5110_SetCursor(x, y);
            }
        }
        ptr++;
    }
}

void IntToStr(int num, char *str) {
    int i = 0, isNegative = 0;

    // Handle 0 explicitly
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';  // Null-terminate the string
        return;
    }

    // Handle negative numbers
    if (num < 0) {
        isNegative = 1;
        num = -num;  // Convert to positive for processing
    }

    // Process individual digits and store them in reverse order
    while (num != 0) {
        str[i++] = (num % 10) + '0';  // Extract the last digit and convert to char
        num /= 10;                    // Remove the last digit
    }

    // Add minus sign if the number is negative
    if (isNegative) {
        str[i++] = '-';
    }

    // Null-terminate the string
    str[i] = '\0';

    // Reverse the string (digits are currently in reverse order)
    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }
}
void Nokia5110_DrawFullImage(const uint8_t *ptr) {
    Nokia5110_SetCursor(0, 0);  // Set the cursor to the top-left corner
    for (int i = 0; i < MAX_X * MAX_Y / 8; i++) {
        lcdwrite(DATA, ptr[i]);  // Write each byte of the image data
    }
}
// Buffer to store the screen content
static uint8_t Screen[504];  // 84x48 pixels = 504 bytes

void Nokia5110_DisplayBuffer(void) {
    Nokia5110_DrawFullImage(Screen);  // Use DrawFullImage to render the buffer
}
void Nokia5110_ClearBuffer(void) {
    for (int i = 0; i < 504; i++) {
        Screen[i] = 0;  // Clear the screen buffer
    }
}
void Nokia5110_SetPxl(uint32_t i, uint32_t j) {
    if (i >= MAX_Y || j >= MAX_X) {
        return;  // Out of bounds
    }
    Screen[(j + (i / 8) * MAX_X)] |= (1 << (i % 8));  // Set the bit
}
void Nokia5110_ClrPxl(uint32_t i, uint32_t j) {
    if (i >= MAX_Y || j >= MAX_X) {
        return;  // Out of bounds
    }
    Screen[(j + (i / 8) * MAX_X)] &= ~(1 << (i % 8));  // Clear the bit
}
// Convert data point to pixel on the Nokia5110 screen
int mapValue(float value, float in_min, float in_max, int out_min, int out_max) {
    return (int)((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

void drawDistanceVsAngle(float *angles, float *distances, int size) {
    Nokia5110_ClearBuffer();  // Clear the buffer

    // Define screen boundaries
    int lcd_x_min = 0, lcd_x_max = 83;
    int lcd_y_min = 47, lcd_y_max = 0;  // Y-axis is inverted on the Nokia 5110

    // Draw axes
    for (int x = lcd_x_min; x <= lcd_x_max; x++) {
        Nokia5110_SetPxl(lcd_y_min, x);  // X-axis
    }
    for (int y = lcd_y_min; y >= lcd_y_max; y--) {
        Nokia5110_SetPxl(y, lcd_x_min);  // Y-axis
    }

    // Add tick marks on axes
    for (int x = lcd_x_min + 10; x <= lcd_x_max; x += 10) {
        Nokia5110_SetPxl(lcd_y_min - 1, x);  // Tick marks on X-axis
        Nokia5110_SetPxl(lcd_y_min - 2, x);
    }
    for (int y = lcd_y_min - 10; y >= lcd_y_max; y -= 10) {
        Nokia5110_SetPxl(y, lcd_x_min + 1);  // Tick marks on Y-axis
        Nokia5110_SetPxl(y, lcd_x_min + 2);
    }

    // Plot data points
    for (int i = 0; i < size; i++) {
        int x = mapValue(angles[i], X_MIN, X_MAX, lcd_x_min, lcd_x_max);
        int y = mapValue(distances[i], Y_MIN, Y_MAX, lcd_y_min, lcd_y_max);
        Nokia5110_SetPxl(y, x);  // Set the corresponding pixel
    }

    // Render the buffer to the screen
    Nokia5110_DisplayBuffer();
}

*/


void Nokia5110_Init(void) {
    volatile uint32_t delay;

    // Enable clocks for SSI0 and GPIOA
    SYSCTL->RCGCSSI |= SYSCTL_RCGCSSI_R0;    // Enable SSI0 clock
    SYSCTL->RCGCGPIO |= SYSCTL_RCGCGPIO_R0;  // Enable Port A clock
    delay = SYSCTL->RCGCGPIO;                // Allow time for clock to stabilize

    // Configure GPIOA Pins for SSI0 and Control
    GPIOA->AFSEL |= 0x2C;    // Enable alternate function on PA2, PA3, PA5 (SSI0)
    GPIOA->AFSEL &= ~0xC0;   // Disable alternate function on PA6, PA7 (GPIO control)
    GPIOA->DEN |= 0xEC;      // Enable digital functionality on PA2, PA3, PA5, PA6, PA7
    GPIOA->PCTL = (GPIOA->PCTL & 0xFF0F00FF) | 0x00202200;  // Configure PA2, PA3, PA5 for SSI
    GPIOA->AMSEL &= ~0xEC;   // Disable analog functionality on PA2, PA3, PA5, PA6, PA7
    GPIOA->DIR |= 0xC0;      // Make PA6, PA7 outputs (DC and RESET)

    // Configure SSI0
    SSI0->CR1 = 0;                          // Disable SSI0 and configure as master
    SSI0->CC = 0;                           // Use system clock
    SSI0->CPSR = 16;                        // Set clock prescale to divide by 16 (3.125 MHz)
    SSI0->CR0 = SSI_CR0_FRF_MOTO |          // Freescale SPI frame format
                SSI_CR0_DSS_8;              // 8-bit data size
    SSI0->CR1 |= SSI_CR1_SSE;               // Enable SSI0

    // Reset the Nokia5110
    RESET = RESET_LOW;                       // Pull RESET low
    for (delay = 0; delay < 10; delay++) {}; // Delay for a short time
    RESET = RESET_HIGH;                      // Release RESET

    // Initialize Nokia5110 LCD
		
    lcdwrite(COMMAND, 0x21);                 // Use extended instruction set
   lcdwrite(COMMAND, 0x8F);                 // Bora 8F diye ayarladi Set contrast (adjustable if needed)// ffSI BAYAAAAAAA KRITIK
    lcdwrite(COMMAND, 0x04);                 // Set temperature coefficient
    lcdwrite(COMMAND, 0x14);                 // Set bias mode
    lcdwrite(COMMAND, 0x20);                 // Use basic instruction set
    lcdwrite(COMMAND, 0x0C);                 // Set normal display mode
}

void Nokia5110_DeInit(void) {
    volatile uint32_t delay;

    // Step 1: Ensure clock for SSI0 and GPIOA is enabled
    SYSCTL->RCGCSSI |= SYSCTL_RCGCSSI_R0;    // Enable SSI0 clock
    SYSCTL->RCGCGPIO |= SYSCTL_RCGCGPIO_R0;  // Enable Port A clock
    delay = SYSCTL->RCGCGPIO;                // Allow time for clock to stabilize

    // Step 2: Disable SSI0
    SSI0->CR1 = 0;  // Disable SSI0 by clearing the SSE bit

    // Step 3: Reset GPIOA Pins used for Nokia 5110
    GPIOA->AFSEL &= ~0xEC;   // Disable alternate functions on PA2, PA3, PA5, PA6, PA7
    GPIOA->PCTL &= ~0xFFF0FFF0; // Clear PCTL configuration for PA2, PA3, PA5, PA6, PA7
    GPIOA->AMSEL &= ~0xEC;   // Disable analog functionality on PA2, PA3, PA5, PA6, PA7
    GPIOA->DIR &= ~0xC0;     // Set PA6 and PA7 as inputs (control pins)
    GPIOA->DEN &= ~0xEC;     // Disable digital functionality on PA2, PA3, PA5, PA6, PA7

    // Step 4: Reset the Nokia5110 (optional but safer)
    RESET = RESET_LOW;       // Pull RESET low to put the display in reset state

    // Step 5: Disable clocks for SSI0 and GPIOA (only if not used elsewhere)
    SYSCTL->RCGCSSI &= ~SYSCTL_RCGCSSI_R0;    // Disable SSI0 clock
    SYSCTL->RCGCGPIO &= ~SYSCTL_RCGCGPIO_R0;  // Disable Port A clock
}



void lcdwrite(enum typeOfWrite type, char message) {
    if (type == COMMAND) {
        while ((SSI0->SR & SSI_SR_BSY) == SSI_SR_BSY) {};  // Wait until SSI0 not busy
        DC = DC_COMMAND;                                   // Set DC low for command
        SSI0->DR = message;                                // Send command
        while ((SSI0->SR & SSI_SR_BSY) == SSI_SR_BSY) {};  // Wait until done
    } else {
        while ((SSI0->SR & SSI_SR_TNF) == 0) {};           // Wait until FIFO not full
        DC = DC_DATA;                                      // Set DC high for data
        SSI0->DR = message;                                // Send data
    }
}

void Nokia5110_Clear(void) {
    int i;

    // Set cursor to (0,0) for writing
    Nokia5110_SetCursor(0, 0);

    // Write zeros to the entire screen
    for (i = 0; i < (MAX_X * MAX_Y / 8); i++) {
        lcdwrite(DATA, 0x00);
    }

    // Reset cursor to (0,0)
    Nokia5110_SetCursor(0, 0);
}


void Nokia5110_SetCursor(unsigned char newX, unsigned char newY) {
    if ((newX > 11) || (newY > 5)) return;  // Invalid cursor position
    lcdwrite(COMMAND, 0x80 | (newX * 7));   // Set X-position
    lcdwrite(COMMAND, 0x40 | newY);         // Set Y-position
}

void Nokia5110_OutChar(char data) {
    int i;
    lcdwrite(DATA, 0x00);                 // Blank vertical line padding
    for (i = 0; i < 5; i++) {
        lcdwrite(DATA, ASCII[data - 0x20][i]);
    }
    lcdwrite(DATA, 0x00);                 // Blank vertical line padding
}

void Nokia5110_OutString(char *ptr) {
    unsigned char x = 0, y = 0;  // Current cursor position (x, y)
    while (*ptr) {
        if (*ptr == '\n') {  // Handle newline character
            x = 0;           // Reset x to the beginning of the line
            if (y < 5) {     // Move to the next row if not at the bottom
                y++;
            }
            Nokia5110_SetCursor(x, y);
        } else {  // Regular character
            Nokia5110_OutChar(*ptr);
            x++;  // Move cursor to the next character position
            if (x > 11) {  // Line overflow, wrap to the next line
                x = 0;
                if (y < 5) {
                    y++;
                }
                Nokia5110_SetCursor(x, y);
            }
        }
        ptr++;
    }
}

void IntToStr(int num, char *str) {
    int i = 0, isNegative = 0;

    // Handle 0 explicitly
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';  // Null-terminate the string
        return;
    }

    // Handle negative numbers
    if (num < 0) {
        isNegative = 1;
        num = -num;  // Convert to positive for processing
    }

    // Process individual digits and store them in reverse order
    while (num != 0) {
        str[i++] = (num % 10) + '0';  // Extract the last digit and convert to char
        num /= 10;                    // Remove the last digit
    }

    // Add minus sign if the number is negative
    if (isNegative) {
        str[i++] = '-';
    }

    // Null-terminate the string
    str[i] = '\0';

    // Reverse the string (digits are currently in reverse order)
    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }
}
void Nokia5110_DrawFullImage(const uint8_t *ptr) {
    Nokia5110_SetCursor(0, 0);  // Set the cursor to the top-left corner
    for (int i = 0; i < MAX_X * MAX_Y / 8; i++) {
        lcdwrite(DATA, ptr[i]);  // Write each byte of the image data
    }
}
// Buffer to store the screen content
static uint8_t Screen[504];  // 84x48 pixels = 504 bytes

void Nokia5110_DisplayBuffer(void) {
    Nokia5110_DrawFullImage(Screen);  // Use DrawFullImage to render the buffer
}
void Nokia5110_ClearBuffer(void) {
    for (int i = 0; i < 504; i++) {
        Screen[i] = 0;  // Clear the screen buffer
    }
}
void Nokia5110_SetPxl(uint32_t i, uint32_t j) {
    if (i >= MAX_Y || j >= MAX_X) {
        return;  // Out of bounds
    }
    Screen[(j + (i / 8) * MAX_X)] |= (1 << (i % 8));  // Set the bit
}
void Nokia5110_ClrPxl(uint32_t i, uint32_t j) {
    if (i >= MAX_Y || j >= MAX_X) {
        return;  // Out of bounds
    }
    Screen[(j + (i / 8) * MAX_X)] &= ~(1 << (i % 8));  // Clear the bit
}
// Convert data point to pixel on the Nokia5110 screen
int mapValue(float value, float in_min, float in_max, int out_min, int out_max) {
    return (int)((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

/*
void drawDistanceVsAngle(float *angles, float *distances, int size) {
>>>>>>> Stashed changes
    Nokia5110_ClearBuffer();  // Clear the buffer

    // Define screen boundaries
    int lcd_x_min = 0, lcd_x_max = 83;
    int lcd_y_min = 47, lcd_y_max = 0;  // Y-axis is inverted on the Nokia 5110

    // Draw axes
    for (int x = lcd_x_min; x <= lcd_x_max; x++) {
        Nokia5110_SetPxl(lcd_y_min, x);  // X-axis
    }
    for (int y = lcd_y_min; y >= lcd_y_max; y--) {
        Nokia5110_SetPxl(y, lcd_x_min);  // Y-axis
    }

    // Add tick marks on axes
    for (int x = lcd_x_min + 10; x <= lcd_x_max; x += 10) {
        Nokia5110_SetPxl(lcd_y_min - 1, x);  // Tick marks on X-axis
        Nokia5110_SetPxl(lcd_y_min - 2, x);
    }
    for (int y = lcd_y_min - 10; y >= lcd_y_max; y -= 10) {
        Nokia5110_SetPxl(y, lcd_x_min + 1);  // Tick marks on Y-axis
        Nokia5110_SetPxl(y, lcd_x_min + 2);
    }

    // Plot data points
    for (int i = 0; i < size; i++) {
        int x = mapValue(angles[i]-90.0, X_MIN, X_MAX, lcd_x_min, lcd_x_max);
        int y = mapValue(distances[i], Y_MIN, Y_MAX, lcd_y_min, lcd_y_max);
        Nokia5110_SetPxl(y, x);  // Set the corresponding pixel
    }

    // Render the buffer to the screen
    Nokia5110_DisplayBuffer();
}*/

void drawDistanceVsAngle(float *angles, float *distances, int size) {
    Nokia5110_ClearBuffer();  // Clear the buffer

    // Define screen boundaries
    int lcd_x_min = 0, lcd_x_max = 83;
    int lcd_y_min = 47, lcd_y_max = 0;  // Y-axis is inverted on the Nokia 5110

    // Draw axes
    for (int x = lcd_x_min; x <= lcd_x_max; x++) {
        Nokia5110_SetPxl(lcd_y_min, x);  // X-axis
    }
    for (int y = lcd_y_min; y >= lcd_y_max; y--) {
        Nokia5110_SetPxl(y, lcd_x_min);  // Y-axis
    }

    // Add tick marks on axes
    for (int x = lcd_x_min + 10; x <= lcd_x_max; x += 10) {
        Nokia5110_SetPxl(lcd_y_min - 1, x);  // Tick marks on X-axis
        Nokia5110_SetPxl(lcd_y_min - 2, x);
    }
    for (int y = lcd_y_min - 10; y >= lcd_y_max; y -= 10) {
        Nokia5110_SetPxl(y, lcd_x_min + 1);  // Tick marks on Y-axis
        Nokia5110_SetPxl(y, lcd_x_min + 2);
    }

    for (int i = 0; i < size; i++) {
        // Convert polar coordinates to Cartesian
        float x = distances[i] * cos_values[i];  // Convert angle to radians
        float y = distances[i] * sin_values[i];

        // Map Cartesian coordinates to screen pixels
        int screen_x = mapValue(x, X_MIN, X_MAX, lcd_x_min, lcd_x_max);  // Screen width is 84 pixels
        int screen_y = mapValue(y, Y_MIN, Y_MAX, lcd_y_min, lcd_y_max);  // Screen height is 48 pixels (inverted)

        Nokia5110_SetPxl(screen_y, screen_x);  // Plot the point
    }

    // Render the buffer to the screen
    Nokia5110_DisplayBuffer();
}









