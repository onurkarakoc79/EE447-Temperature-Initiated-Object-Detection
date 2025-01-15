#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initializes I2C0 on the Tiva TM4C123GH6PM for master mode.
 *        Uses PortB pins PB2 (SCL) and PB3 (SDA) for I2C0.
 * 
 * @param  none
 * @return none
 */
void I2C0_Init(void);

/**
 * @brief Writes one byte to a slave device over I2C.
 * 
 * @param slaveAddr 7-bit slave address
 * @param reg       register pointer/address to write to
 * @param data      single byte to write
 */
void I2C0_WriteByte(uint8_t slaveAddr, uint8_t reg, uint8_t data);

/**
 * @brief Reads one byte from a slave device over I2C.
 * 
 * @param slaveAddr 7-bit slave address
 * @param reg       register pointer/address to read from
 * @return          the read byte
 */
uint8_t I2C0_ReadByte(uint8_t slaveAddr, uint8_t reg);

/**
 * @brief Reads multiple consecutive bytes from a slave device.
 * 
 * @param slaveAddr 7-bit slave address
 * @param reg       register pointer/address to read from
 * @param count     number of bytes to read
 * @param data      pointer to buffer to store read data
 */
void I2C0_ReadMulti(uint8_t slaveAddr, uint8_t reg, uint8_t count, uint8_t *data);
void I2C0_Deinit(void);
#endif /* I2C_H_ */
