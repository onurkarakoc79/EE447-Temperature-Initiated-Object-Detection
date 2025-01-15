#ifndef BMP280_H_
#define BMP280_H_

#include <stdint.h>
#include "KEYPAD.h"
#include "LED_BUZZER.h"
#include "DEEP_SLEEP.h"
/* BMP280 default I2C address is 0x76 or 0x77 (check your module) */
#define BMP280_I2C_ADDR         0x76
//volatile float BMP_DEFAULT_THRESHOLD = 50.2;
#define THRESHOLD 50.2

/* BMP280 Register Addresses */
#define BMP280_REG_ID           0xD0
#define BMP280_REG_RESET        0xE0
#define BMP280_REG_STATUS       0xF3
#define BMP280_REG_CTRL_MEAS    0xF4
#define BMP280_REG_CONFIG       0xF5
#define BMP280_REG_PRESS_MSB    0xF7
#define BMP280_REG_PRESS_LSB    0xF8
#define BMP280_REG_PRESS_XLSB   0xF9
#define BMP280_REG_TEMP_MSB     0xFA
#define BMP280_REG_TEMP_LSB     0xFB
#define BMP280_REG_TEMP_XLSB    0xFC

/* Calibration registers start address */
#define BMP280_REG_CALIB_START  0x88
#define BMP280_REG_CALIB_END    0x9F

/*
 * Data structure to hold BMP280 calibration data
 * Typically 24 bytes for temperature and pressure calibration
 */
typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} bmp280_calib_data_t;

typedef struct {
		volatile float temperatureAvg;
		volatile int temperatureRaw;
		volatile float pressureAvg;
		volatile int pressureRaw;
		volatile float thresholdTemperature;
		void (*init)(void);
		void (*readTemperature)(void);
		void (*readTemperatureAvg)(void);
		void (*readPressure)(void);
		void (*readPressureAvg)(void);
		void (*deinit)(void);
		int (*waitThreshold) (void);
}BMP280_Struct;


extern BMP280_Struct* const bmp280;

#endif /* BMP280_H_ */
