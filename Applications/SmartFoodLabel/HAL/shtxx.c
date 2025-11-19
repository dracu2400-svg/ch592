/**
 * @file    shtxx.c
 * @brief   SHTxx Temperature and Humidity Sensor Driver Implementation
 */

#include "shtxx.h"
#include "CH59x_common.h"

/*********************************************************************
 * LOCAL VARIABLES
 */

static sht_type_t sensor_type = SHT_TYPE_SHT2X;
static sht_data_t last_data = {0};

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief   Initialize I2C master mode
 */
static void I2C_Master_Init(void)
{
    // Configure I2C pins: PB12=SCL, PB13=SDA
    GPIOB_ModeCfg(GPIO_Pin_12 | GPIO_Pin_13, GPIO_ModeIN_PU);

    // Enable I2C module
    R8_I2C_CTRL_MOD = RB_I2C_MASTER;  // Master mode
    R16_I2C_CLK_DIV = 60;  // 400kHz @ 60MHz system clock
}

/**
 * @brief   I2C Start Condition
 */
static void I2C_Start(void)
{
    R8_I2C_CTRL_MOD |= RB_I2C_START;
    while(!(R8_I2C_INT_FLAG & RB_I2C_IF_START));
    R8_I2C_INT_FLAG = RB_I2C_IF_START;
}

/**
 * @brief   I2C Stop Condition
 */
static void I2C_Stop(void)
{
    R8_I2C_CTRL_MOD |= RB_I2C_STOP;
    while(R8_I2C_INT_FLAG & RB_I2C_IF_STOP);
}

/**
 * @brief   I2C Write Byte
 * @return  0 = ACK, 1 = NACK
 */
static uint8 I2C_WriteByte(uint8 data)
{
    R8_I2C_DATA = data;
    R8_I2C_CTRL_MOD &= ~RB_I2C_START;
    while(!(R8_I2C_INT_FLAG & RB_I2C_IF_TX_END));
    R8_I2C_INT_FLAG = RB_I2C_IF_TX_END;

    return (R8_I2C_CTRL_MOD & RB_I2C_ACK) ? 1 : 0;
}

/**
 * @brief   I2C Read Byte
 * @param   ack - 0 = NACK, 1 = ACK
 * @return  Read data
 */
static uint8 I2C_ReadByte(uint8 ack)
{
    uint8 data;

    if(!ack) {
        R8_I2C_CTRL_MOD |= RB_I2C_ACK;  // Send NACK
    } else {
        R8_I2C_CTRL_MOD &= ~RB_I2C_ACK; // Send ACK
    }

    while(!(R8_I2C_INT_FLAG & RB_I2C_IF_RX_END));
    data = R8_I2C_DATA;
    R8_I2C_INT_FLAG = RB_I2C_IF_RX_END;

    return data;
}

/**
 * @brief   Calculate CRC-8 for SHT2x
 */
static uint8 SHT2x_CalcCRC(uint8 *data, uint8 len)
{
    uint8 crc = 0;
    uint8 i, j;

    for(i = 0; i < len; i++) {
        crc ^= data[i];
        for(j = 0; j < 8; j++) {
            if(crc & 0x80) {
                crc = (crc << 1) ^ 0x131;
            } else {
                crc = crc << 1;
            }
        }
    }

    return crc;
}

/**
 * @brief   Read measurement from SHT2x
 */
static uint8 SHT2x_ReadMeasurement(uint8 cmd, uint16 *raw_value)
{
    uint8 data[3];
    uint8 crc;

    // Send measurement command
    I2C_Start();
    if(I2C_WriteByte(SHT2X_I2C_ADDR << 1) != 0) {
        I2C_Stop();
        return 1;
    }
    if(I2C_WriteByte(cmd) != 0) {
        I2C_Stop();
        return 1;
    }

    // Wait for measurement
    if(cmd == SHT2X_CMD_TEMP_NO_HOLD) {
        DelayMs(SHT2X_TEMP_DELAY);
    } else {
        DelayMs(SHT2X_HUMID_DELAY);
    }

    // Read result
    I2C_Start();
    if(I2C_WriteByte((SHT2X_I2C_ADDR << 1) | 1) != 0) {
        I2C_Stop();
        return 1;
    }

    data[0] = I2C_ReadByte(1);  // MSB
    data[1] = I2C_ReadByte(1);  // LSB
    data[2] = I2C_ReadByte(0);  // CRC
    I2C_Stop();

    // Verify CRC
    crc = SHT2x_CalcCRC(data, 2);
    if(crc != data[2]) {
        return 1;  // CRC error
    }

    // Combine MSB and LSB
    *raw_value = (data[0] << 8) | (data[1] & 0xFC);

    return 0;
}

/**
 * @brief   Read measurement from SHT3x
 */
static uint8 SHT3x_ReadMeasurement(uint16 *temp_raw, uint16 *humid_raw)
{
    uint8 data[6];
    uint8 i;

    // Send measurement command
    I2C_Start();
    if(I2C_WriteByte(SHT3X_I2C_ADDR << 1) != 0) {
        I2C_Stop();
        return 1;
    }
    if(I2C_WriteByte(SHT3X_CMD_MEAS_HIGH_MSB) != 0) {
        I2C_Stop();
        return 1;
    }
    if(I2C_WriteByte(SHT3X_CMD_MEAS_HIGH_LSB) != 0) {
        I2C_Stop();
        return 1;
    }

    // Wait for measurement
    DelayMs(SHT3X_MEAS_DELAY);

    // Read result
    I2C_Start();
    if(I2C_WriteByte((SHT3X_I2C_ADDR << 1) | 1) != 0) {
        I2C_Stop();
        return 1;
    }

    for(i = 0; i < 6; i++) {
        data[i] = I2C_ReadByte(i < 5 ? 1 : 0);
    }
    I2C_Stop();

    // Extract temperature and humidity (skip CRC for simplicity)
    *temp_raw = (data[0] << 8) | data[1];
    *humid_raw = (data[3] << 8) | data[4];

    return 0;
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief   Initialize SHTxx sensor
 */
uint8 SHT_Init(sht_type_t type)
{
    sensor_type = type;

    // Initialize I2C
    I2C_Master_Init();

    // Perform soft reset
    DelayMs(10);
    if(SHT_SoftReset() != 0) {
        return 1;
    }

    DelayMs(15);

    // Initialize data structure
    last_data.temperature = 0.0f;
    last_data.humidity = 0.0f;
    last_data.valid = 0;

    return 0;
}

/**
 * @brief   Read temperature from sensor
 */
uint8 SHT_ReadTemperature(float *temp)
{
    uint16 raw_value;

    if(sensor_type == SHT_TYPE_SHT2X) {
        if(SHT2x_ReadMeasurement(SHT2X_CMD_TEMP_NO_HOLD, &raw_value) != 0) {
            return 1;
        }

        // Convert to Celsius: T = -46.85 + 175.72 * (raw / 2^16)
        *temp = -46.85f + 175.72f * ((float)raw_value / 65536.0f);
    } else {
        uint16 temp_raw, humid_raw;
        if(SHT3x_ReadMeasurement(&temp_raw, &humid_raw) != 0) {
            return 1;
        }

        // Convert to Celsius: T = -45 + 175 * (raw / 2^16)
        *temp = -45.0f + 175.0f * ((float)temp_raw / 65535.0f);
    }

    return 0;
}

/**
 * @brief   Read humidity from sensor
 */
uint8 SHT_ReadHumidity(float *humid)
{
    uint16 raw_value;

    if(sensor_type == SHT_TYPE_SHT2X) {
        if(SHT2x_ReadMeasurement(SHT2X_CMD_HUMID_NO_HOLD, &raw_value) != 0) {
            return 1;
        }

        // Convert to %RH: RH = -6 + 125 * (raw / 2^16)
        *humid = -6.0f + 125.0f * ((float)raw_value / 65536.0f);
    } else {
        uint16 temp_raw, humid_raw;
        if(SHT3x_ReadMeasurement(&temp_raw, &humid_raw) != 0) {
            return 1;
        }

        // Convert to %RH: RH = 100 * (raw / 2^16)
        *humid = 100.0f * ((float)humid_raw / 65535.0f);
    }

    // Limit to valid range
    if(*humid < 0.0f) *humid = 0.0f;
    if(*humid > 100.0f) *humid = 100.0f;

    return 0;
}

/**
 * @brief   Read both temperature and humidity
 */
uint8 SHT_ReadData(sht_data_t *data)
{
    if(sensor_type == SHT_TYPE_SHT2X) {
        if(SHT_ReadTemperature(&data->temperature) != 0) {
            data->valid = 0;
            return 1;
        }
        if(SHT_ReadHumidity(&data->humidity) != 0) {
            data->valid = 0;
            return 1;
        }
    } else {
        uint16 temp_raw, humid_raw;
        if(SHT3x_ReadMeasurement(&temp_raw, &humid_raw) != 0) {
            data->valid = 0;
            return 1;
        }

        // Convert temperature
        data->temperature = -45.0f + 175.0f * ((float)temp_raw / 65535.0f);

        // Convert humidity
        data->humidity = 100.0f * ((float)humid_raw / 65535.0f);
        if(data->humidity < 0.0f) data->humidity = 0.0f;
        if(data->humidity > 100.0f) data->humidity = 100.0f;
    }

    data->valid = 1;

    // Store last valid data
    last_data = *data;

    return 0;
}

/**
 * @brief   Perform soft reset of sensor
 */
uint8 SHT_SoftReset(void)
{
    if(sensor_type == SHT_TYPE_SHT2X) {
        I2C_Start();
        if(I2C_WriteByte(SHT2X_I2C_ADDR << 1) != 0) {
            I2C_Stop();
            return 1;
        }
        if(I2C_WriteByte(SHT2X_CMD_SOFT_RESET) != 0) {
            I2C_Stop();
            return 1;
        }
        I2C_Stop();
    } else {
        I2C_Start();
        if(I2C_WriteByte(SHT3X_I2C_ADDR << 1) != 0) {
            I2C_Stop();
            return 1;
        }
        if(I2C_WriteByte(SHT3X_CMD_SOFT_RESET_MSB) != 0) {
            I2C_Stop();
            return 1;
        }
        if(I2C_WriteByte(SHT3X_CMD_SOFT_RESET_LSB) != 0) {
            I2C_Stop();
            return 1;
        }
        I2C_Stop();
    }

    return 0;
}

/**
 * @brief   Get last valid sensor data
 */
uint8 SHT_GetLastData(sht_data_t *data)
{
    if(!last_data.valid) {
        return 1;
    }

    *data = last_data;
    return 0;
}
