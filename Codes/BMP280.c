#include "BMP280.h"
#include "I2C.h"

#define SAMPLING_NUMBER 130


static bmp280_calib_data_t bmp280_calib;
static int32_t t_fine;




static BMP280_Struct bmp280_instance = {
		.temperatureAvg=0,
		.temperatureRaw=0,
		.pressureAvg=0,
		.pressureRaw=0,
		.thresholdTemperature=THRESHOLD
};

BMP280_Struct* const bmp280 = &bmp280_instance;

int BMP280_WAIT_OVERSHOOT(void){
		bmp280->readTemperatureAvg();
		while(bmp280->temperatureAvg < bmp280->thresholdTemperature){
			bmp280->readTemperatureAvg();
			
			ChangeGraphButtonRead();
			keypad->read();
			if(keypad->deepSleepActivate==0){
				return -1;
			}
			bmp280->thresholdTemperature=keypad->number;
			
		}
		return 1;

}

/* 
 * Helper function to read the 24 bytes of calibration data 
 * from registers 0x88 to 0x9F 
 */
static void BMP280_ReadCalibration(void)
{
    uint8_t calib[24];
    I2C0_ReadMulti(BMP280_I2C_ADDR, BMP280_REG_CALIB_START, 24, calib);

    bmp280_calib.dig_T1 = (uint16_t)((calib[1] << 8) | calib[0]);
    bmp280_calib.dig_T2 = (int16_t)((calib[3] << 8) | calib[2]);
    bmp280_calib.dig_T3 = (int16_t)((calib[5] << 8) | calib[4]);
    bmp280_calib.dig_P1 = (uint16_t)((calib[7] << 8) | calib[6]);
    bmp280_calib.dig_P2 = (int16_t)((calib[9] << 8) | calib[8]);
    bmp280_calib.dig_P3 = (int16_t)((calib[11] << 8) | calib[10]);
    bmp280_calib.dig_P4 = (int16_t)((calib[13] << 8) | calib[12]);
    bmp280_calib.dig_P5 = (int16_t)((calib[15] << 8) | calib[14]);
    bmp280_calib.dig_P6 = (int16_t)((calib[17] << 8) | calib[16]);
    bmp280_calib.dig_P7 = (int16_t)((calib[19] << 8) | calib[18]);
    bmp280_calib.dig_P8 = (int16_t)((calib[21] << 8) | calib[20]);
    bmp280_calib.dig_P9 = (int16_t)((calib[23] << 8) | calib[22]);
}

void BMP280_Init_Impl(void)
{
		I2C0_Init();
    /* Read the chip ID to verify communication */
    //uint8_t chipID = I2C0_ReadByte(BMP280_I2C_ADDR, BMP280_REG_ID);
    // Ideally check if chipID == 0x58 or 0x60 for BMP280, BME280, etc.

    /* Soft reset the sensor (optional) */
    I2C0_WriteByte(BMP280_I2C_ADDR, BMP280_REG_RESET, 0xB6);

    /* Wait a bit for reset to complete (per datasheet ~2ms) */
    for(volatile int i = 0; i < 80000; i++) {}

    /* Read calibration data */
    BMP280_ReadCalibration();

    /*
     * Set CTRL_MEAS and CONFIG registers:
     *   - Oversampling for temperature = x1
     *   - Oversampling for pressure = x1
     *   - Normal mode
     *   - Filter off, standby time = 0.5ms
     */
    I2C0_WriteByte(BMP280_I2C_ADDR, BMP280_REG_CTRL_MEAS, 0x27);  // 00100111
    I2C0_WriteByte(BMP280_I2C_ADDR, BMP280_REG_CONFIG,    0x00);  // 00000000
}

void BMP280_ReadTemperature_Impl(void)
{
    /* Read raw temperature data (20-bit, but stored in 3 bytes) */
    uint8_t raw[3];
    I2C0_ReadMulti(BMP280_I2C_ADDR, BMP280_REG_TEMP_MSB, 3, raw);
    int32_t adc_T = ((int32_t)raw[0] << 12) | ((int32_t)raw[1] << 4) | ((int32_t)(raw[2] >> 4));

    /*
     * Temperature compensation formula from the BMP280 datasheet
     */
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)bmp280_calib.dig_T1 << 1))) *
            ((int32_t)bmp280_calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)bmp280_calib.dig_T1)) *
               ((adc_T >> 4) - ((int32_t)bmp280_calib.dig_T1))) >> 12) *
             ((int32_t)bmp280_calib.dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;  // temperature in °C * 100
		bmp280_instance.temperatureRaw=T;
    
}

void BMP280_ReadPressure_Impl(void)
{
    /* Must read temperature first to update t_fine */
    BMP280_ReadTemperature_Impl();

    /* Read raw pressure data (20-bit) */
    uint8_t raw[3];
    I2C0_ReadMulti(BMP280_I2C_ADDR, BMP280_REG_PRESS_MSB, 3, raw);
    int32_t adc_P = ((int32_t)raw[0] << 12) | ((int32_t)raw[1] << 4) | ((int32_t)(raw[2] >> 4));

    /*
     * Pressure compensation formula from BMP280 datasheet
     */
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)bmp280_calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)bmp280_calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)bmp280_calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)bmp280_calib.dig_P3) >> 8) +
           ((var1 * (int64_t)bmp280_calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1) * ((int64_t)bmp280_calib.dig_P1)) >> 33;

    if(var1 == 0) {
        bmp280_instance.readPressure = 0; // avoid exception caused by division by zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)bmp280_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)bmp280_calib.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)bmp280_calib.dig_P7) << 4);
    
    /* p is in Q4.12 format => Pa. If you want hPa, do (p / 25600). */
    bmp280_instance.pressureRaw=(uint32_t)p;  // pressure in Pa
}


static void BMP280_ReadTemperatureAvg_Impl(){
		for(int i=0;i<SAMPLING_NUMBER;i++){
				bmp280_instance.readTemperature();
				bmp280_instance.temperatureAvg +=bmp280_instance.temperatureRaw;
				for(int i=0;i<1000;i++){
				};
		};
		bmp280_instance.temperatureAvg/=(float)SAMPLING_NUMBER;
		bmp280_instance.temperatureAvg/=100.0;
};


static void BMP280_Deinit_Impl(void)
{
    /*
     * 1) Put the BMP280 in sleep mode or minimal power mode 
     *    (writing 0x00 to CTRL_MEAS, for example)
     *    This ensures the BMP280 is not performing any measurements.
     */
    //I2C0_WriteByte(BMP280_I2C_ADDR, BMP280_REG_CTRL_MEAS, 0x00);

		I2C0_Deinit();

    /*
     * 3) Optionally set function pointers to NULL to prevent accidental usage
     */
}
__attribute__((constructor))
static void BMP280_SetupPointers(void)
{
    bmp280_instance.init = BMP280_Init_Impl;
    bmp280_instance.readTemperature = BMP280_ReadTemperature_Impl;
    bmp280_instance.readPressure = BMP280_ReadPressure_Impl;
    bmp280_instance.readTemperatureAvg = BMP280_ReadTemperatureAvg_Impl;
    bmp280_instance.deinit = BMP280_Deinit_Impl;
		bmp280_instance.waitThreshold = BMP280_WAIT_OVERSHOOT;
		
}
