#include "I2C.h"
#include <stdint.h>
#include <stdbool.h>
#include "TM4C123GH6PM.h"

/* 
 * Define some helper bitmasks for clarity
 */
#define I2C_MCS_RUN      0x01
#define I2C_MCS_START    0x02
#define I2C_MCS_STOP     0x04
#define I2C_MCS_ACK      0x08
#define I2C_MCS_BUSY     0x01
#define I2C_MCS_ERROR    0x02

/**
 * @brief Initializes I2C0 in Master mode on pins PB2 (SCL) and PB3 (SDA).
 *        Uses 100kHz standard mode (TPR=7) assuming a 16 MHz system clock.
 */
void I2C0_Init(void)
{
    /* 1. Enable clock for I2C0 and for GPIO Port B */
    SYSCTL->RCGCI2C  |= (1U << 0);  // I2C0 clock gate
    SYSCTL->RCGCGPIO |= (1U << 1);  // GPIOB clock gate

    /* Allow time for the clocks to become active */
    volatile int delay;
    for (delay = 0; delay < 1000; delay++) {}

    /* 2. Configure PB2 (SCL) and PB3 (SDA) pins for I2C */
    GPIOB->AFSEL |= (1U << 2) | (1U << 3);  // enable alternate function on PB2, PB3
    GPIOB->DEN   |= (1U << 2) | (1U << 3);  // enable digital on PB2, PB3
    GPIOB->ODR   |= (1U << 3);             // PB3 (SDA) open-drain
    /*
     * For PB2 and PB3, set the port control (PCTL) to I2C (function = 3).
     * PB2 -> I2C0SCL, PB3 -> I2C0SDA
     * Each nibble in PCTL corresponds to a pin:
     *   PB0 -> bits [3:0]
     *   PB1 -> bits [7:4]
     *   PB2 -> bits [11:8]
     *   PB3 -> bits [15:12]
     */
    GPIOB->PCTL &= ~0x0000FF00;  // clear PCTL for PB2, PB3
    GPIOB->PCTL |=  0x00003300;  // set PB2, PB3 to I2C function (3)

    /* 3. Enable I2C0 Master mode */
    I2C0->MCR = 0x10; // set MFE bit (Master Function Enable)

    /*
     * 4. Configure I2C clock speed for standard mode (100kHz)
     *    TPR = (SystemClock/(2*(SCL_LP + SCL_HP)*SCL_CLK)) - 1
     *    For 16 MHz and 100kHz, TPR is typically 7.
     */
    I2C0->MTPR = 7;
}

/**
 * @brief Wait until the I2C Master is no longer busy.
 */
static void I2C0_WaitWhileBusy(void)
{
    while (I2C0->MCS & I2C_MCS_BUSY) {
        // spin until not busy
    }
}

/**
 * @brief Writes one byte (data) to a given register (reg) on a slave device.
 * 
 * @param slaveAddr 7-bit I2C address of the slave
 * @param reg       register/pointer to which we write
 * @param data      the byte to write
 */
void I2C0_WriteByte(uint8_t slaveAddr, uint8_t reg, uint8_t data)
{
    /* 1. Wait if I2C Master is busy */
    I2C0_WaitWhileBusy();

    /* 2. Set slave address & set write mode (R/W bit = 0) */
    I2C0->MSA = (slaveAddr << 1) & 0xFE;

    /* 3. Place the register pointer in MDR */
    I2C0->MDR = reg;

    /* 4. Initiate send of register address (START + RUN) */
    I2C0->MCS = (I2C_MCS_START | I2C_MCS_RUN);

    /* 5. Wait until transfer done, check error */
    I2C0_WaitWhileBusy();
    if (I2C0->MCS & I2C_MCS_ERROR) {
        I2C0->MCS = I2C_MCS_STOP; // STOP on error
        return;
    }

    /* 6. Write the data byte */
    I2C0->MDR = data;
    I2C0->MCS = (I2C_MCS_RUN | I2C_MCS_STOP);

    /* 7. Wait until done */
    I2C0_WaitWhileBusy();
}

/**
 * @brief Reads one byte from a given register of a slave device.
 * 
 * @param slaveAddr 7-bit I2C address
 * @param reg       register/pointer address to read from
 * @return          the byte read (0xFF if error)
 */
uint8_t I2C0_ReadByte(uint8_t slaveAddr, uint8_t reg)
{
    uint8_t data = 0xFF;

    /* 1. Wait if I2C Master is busy */
    I2C0_WaitWhileBusy();

    /* 2. Set slave address & write mode */
    I2C0->MSA = (slaveAddr << 1) & 0xFE;

    /* 3. Send the register pointer */
    I2C0->MDR = reg;
    I2C0->MCS = (I2C_MCS_START | I2C_MCS_RUN);

    /* 4. Wait & check error */
    I2C0_WaitWhileBusy();
    if (I2C0->MCS & I2C_MCS_ERROR) {
        I2C0->MCS = I2C_MCS_STOP;
        return 0xFF;
    }

    /* 5. Repeated START, switch to read mode */
    I2C0->MSA = (slaveAddr << 1) | 0x01; // R/W bit = 1
    I2C0->MCS = (I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP);

    /* 6. Wait until transfer is complete, check error */
    I2C0_WaitWhileBusy();
    if (I2C0->MCS & I2C_MCS_ERROR) {
        I2C0->MCS = I2C_MCS_STOP;
        return 0xFF;
    }

    /* 7. Read the data from MDR */
    data = (uint8_t)(I2C0->MDR & 0xFF);
    return data;
}

/**
 * @brief Reads multiple consecutive bytes from a given register.
 * 
 * @param slaveAddr 7-bit I2C address
 * @param reg       starting register/pointer address
 * @param count     number of bytes to read
 * @param data      pointer to buffer to store the bytes
 */
void I2C0_ReadMulti(uint8_t slaveAddr, uint8_t reg, uint8_t count, uint8_t *data)
{
    if (count == 0) return;

    /* 1. Wait if I2C Master is busy */
    I2C0_WaitWhileBusy();

    /* 2. Send slave address (write mode) */
    I2C0->MSA = (slaveAddr << 1) & 0xFE;

    /* 3. Send the register pointer */
    I2C0->MDR = reg;
    I2C0->MCS = (I2C_MCS_START | I2C_MCS_RUN);

    /* 4. Wait & check error */
    I2C0_WaitWhileBusy();
    if (I2C0->MCS & I2C_MCS_ERROR) {
        I2C0->MCS = I2C_MCS_STOP;
        return;
    }

    /* 5. Repeated START, switch to read mode */
    I2C0->MSA = (slaveAddr << 1) | 0x01; // R/W = 1
    I2C0->MCS = (I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_ACK);
    I2C0_WaitWhileBusy();
    if (I2C0->MCS & I2C_MCS_ERROR) {
        I2C0->MCS = I2C_MCS_STOP;
        return;
    }

    /* 6. Read count-1 bytes with ACK each time */
    for(uint8_t i = 0; i < (count - 1); i++)
    {
        data[i] = (uint8_t)(I2C0->MDR & 0xFF);
        I2C0->MCS = (I2C_MCS_RUN | I2C_MCS_ACK);
        I2C0_WaitWhileBusy();
        if (I2C0->MCS & I2C_MCS_ERROR) {
            I2C0->MCS = I2C_MCS_STOP;
            return;
        }
    }

    /* 7. Read final byte with NACK + STOP */
    data[count - 1] = (uint8_t)(I2C0->MDR & 0xFF);
    I2C0->MCS = (I2C_MCS_RUN | I2C_MCS_STOP);
    I2C0_WaitWhileBusy();
}
void I2C0_Deinit(void)
{
    // 1) Wait if busy (defensive)
    while (I2C0->MCS & I2C_MCS_BUSY) { /* spin */ }

    // 2) Disable the I2C0 Master function
    I2C0->MCR &= ~0x10;  // Clear the MFE bit

    // 3) Disable the clock to I2C0 if absolutely sure not used elsewhere
    SYSCTL->RCGCI2C &= ~(1U << 0);

    // 4) Optionally reconfigure PB2, PB3 as GPIO inputs or whatever safe state you want:
    GPIOB->AFSEL &= ~((1U << 2) | (1U << 3));
    GPIOB->DEN   &= ~((1U << 2) | (1U << 3));
    GPIOB->ODR   &= ~((1U << 3));
    GPIOB->PCTL  &= ~0x0000FF00;  // Reset PCTL for PB2, PB3

    // 5) Also, if desired, kill clock gating for GPIOB
    //    SYSCTL->RCGCGPIO &= ~(1U << 1);
}