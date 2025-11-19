/**
 * @file    shtxx.h
 * @brief   SHTxx Temperature and Humidity Sensor Driver
 * @details Driver for SHT1x, SHT2x, SHT3x series sensors
 */

#ifndef SHTXX_H
#define SHTXX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "CH59x_common.h"

/*********************************************************************
 * CONSTANTS
 */

// I2C Address (SHT2x/SHT3x default)
#define SHT2X_I2C_ADDR          0x40
#define SHT3X_I2C_ADDR          0x44

// SHT2x Commands
#define SHT2X_CMD_TEMP_HOLD     0xE3  // Measure Temperature, Hold Master
#define SHT2X_CMD_HUMID_HOLD    0xE5  // Measure Humidity, Hold Master
#define SHT2X_CMD_TEMP_NO_HOLD  0xF3  // Measure Temperature, No Hold Master
#define SHT2X_CMD_HUMID_NO_HOLD 0xF5  // Measure Humidity, No Hold Master
#define SHT2X_CMD_WRITE_USER    0xE6  // Write User Register
#define SHT2X_CMD_READ_USER     0xE7  // Read User Register
#define SHT2X_CMD_SOFT_RESET    0xFE  // Soft Reset

// SHT3x Commands
#define SHT3X_CMD_MEAS_HIGH_MSB 0x24  // High repeatability measurement
#define SHT3X_CMD_MEAS_HIGH_LSB 0x00
#define SHT3X_CMD_SOFT_RESET_MSB 0x30
#define SHT3X_CMD_SOFT_RESET_LSB 0xA2

// Measurement delays (ms)
#define SHT2X_TEMP_DELAY        85
#define SHT2X_HUMID_DELAY       29
#define SHT3X_MEAS_DELAY        15

/*********************************************************************
 * TYPEDEFS
 */

typedef enum {
    SHT_TYPE_SHT2X = 0,
    SHT_TYPE_SHT3X
} sht_type_t;

typedef struct {
    float temperature;      // Temperature in Celsius
    float humidity;         // Relative humidity in %
    uint8 valid;           // Data valid flag
} sht_data_t;

/*********************************************************************
 * API FUNCTIONS
 */

/**
 * @brief   Initialize SHTxx sensor
 * @param   type - Sensor type (SHT2x or SHT3x)
 * @return  0 = Success, 1 = Failure
 */
uint8 SHT_Init(sht_type_t type);

/**
 * @brief   Read temperature from sensor
 * @param   temp - Pointer to store temperature (Celsius)
 * @return  0 = Success, 1 = Failure
 */
uint8 SHT_ReadTemperature(float *temp);

/**
 * @brief   Read humidity from sensor
 * @param   humid - Pointer to store humidity (%)
 * @return  0 = Success, 1 = Failure
 */
uint8 SHT_ReadHumidity(float *humid);

/**
 * @brief   Read both temperature and humidity
 * @param   data - Pointer to store sensor data
 * @return  0 = Success, 1 = Failure
 */
uint8 SHT_ReadData(sht_data_t *data);

/**
 * @brief   Perform soft reset of sensor
 * @return  0 = Success, 1 = Failure
 */
uint8 SHT_SoftReset(void);

/**
 * @brief   Get last valid sensor data
 * @param   data - Pointer to store sensor data
 * @return  0 = Success, 1 = No valid data
 */
uint8 SHT_GetLastData(sht_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* SHTXX_H */
