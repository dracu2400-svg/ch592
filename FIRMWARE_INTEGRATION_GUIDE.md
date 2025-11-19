# CH592 Firmware Integration Guide

## Table of Contents
1. [Introduction](#introduction)
2. [Understanding the Firmware Architecture](#understanding-the-firmware-architecture)
3. [How to Add a New BLE Service](#how-to-add-a-new-ble-service)
4. [How to Integrate a New Sensor](#how-to-integrate-a-new-sensor)
5. [How to Modify Existing Services](#how-to-modify-existing-services)
6. [How to Remove Services](#how-to-remove-services)
7. [Advanced Integration Patterns](#advanced-integration-patterns)
8. [Best Practices and Tips](#best-practices-and-tips)

---

## Introduction

This guide provides comprehensive instructions for developers who want to extend the CH592 firmware with:
- **Custom BLE Services**: Add new Bluetooth Low Energy services with custom characteristics
- **Additional Sensors**: Integrate I2C, SPI, or ADC-based sensors
- **Service Manipulation**: Modify, add, or remove existing services

The CH592 is a BLE-enabled RISC-V microcontroller with rich peripheral support. This guide assumes you have basic knowledge of embedded C programming and BLE concepts.

---

## Understanding the Firmware Architecture

### Directory Structure

```
ch592/
├── EVT/
│   └── EXAM/                    # Example projects (learning resources)
│       ├── BLE/                 # BLE examples
│       │   ├── Peripheral/      # Basic peripheral example
│       │   ├── BLE_UART/        # UART over BLE (custom service example)
│       │   ├── HeartRate/       # Heart rate service
│       │   ├── HID_Mouse/       # HID mouse service
│       │   └── ...              # 23+ service examples
│       ├── ADC/                 # ADC peripheral examples
│       ├── I2C/                 # I2C peripheral examples
│       ├── SPI/                 # SPI peripheral examples
│       └── ...                  # 35+ peripheral categories
└── Applications/                # Production applications
    ├── 8K_PollingRateWirelessMouse/
    └── PDF_DataLogger/
```

### Key Components

1. **BLE Stack**: Provided by WCH, handles BLE protocol implementation
2. **TMOS**: Task Management Operating System (event-driven scheduler)
3. **HAL**: Hardware Abstraction Layer for peripherals
4. **Profiles/Services**: BLE GATT services (Heart Rate, HID, Custom, etc.)
5. **Application Layer**: Your custom application logic

### Application Initialization Flow

```c
int main(void)
{
    SetSysClock(CLK_SOURCE_PLL_60MHz);      // 1. Clock setup

    CH59x_BLEInit();                         // 2. BLE stack init
    HAL_Init();                               // 3. HAL init (LED, KEY, RTC)
    GAPRole_PeripheralInit();                 // 4. GAP role setup
    Peripheral_Init();                        // 5. Application init

    while(1) {
        TMOS_SystemProcess();                 // 6. Event loop
    }
}
```

---

## How to Add a New BLE Service

### Overview

Adding a BLE service involves creating:
1. **Service Header** (`.h`): UUIDs, parameters, callbacks
2. **Service Implementation** (`.c`): GATT attribute table, read/write handlers
3. **Integration**: Register service in your application

### Step-by-Step Guide

#### Step 1: Create Service Header File

Create `custom_service.h`:

```c
#ifndef CUSTOM_SERVICE_H
#define CUSTOM_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

// Service UUID (use uuidgen or online generator for your own)
#define CUSTOMSERVICE_SERV_UUID           0x1234

// Characteristic UUIDs
#define CUSTOMSERVICE_CHAR1_UUID          0x1235  // Read characteristic
#define CUSTOMSERVICE_CHAR2_UUID          0x1236  // Write characteristic
#define CUSTOMSERVICE_CHAR3_UUID          0x1237  // Notify characteristic

// Service profile indexes
#define CUSTOMSERVICE_CHAR1_INDEX         0
#define CUSTOMSERVICE_CHAR2_INDEX         1
#define CUSTOMSERVICE_CHAR3_INDEX         2

// Callback events
#define CUSTOMSERVICE_CHAR1_READ_EVENT    0x01
#define CUSTOMSERVICE_CHAR2_WRITE_EVENT   0x02

/*********************************************************************
 * TYPEDEFS
 */

// Callback function type
typedef void (*customServiceCB_t)(uint8 event, uint8 *data, uint16 len);

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * Profile Callbacks
 */

/*********************************************************************
 * API FUNCTIONS
 */

/*
 * CustomService_AddService - Initialize the service
 */
extern bStatus_t CustomService_AddService(void);

/*
 * CustomService_RegisterAppCBs - Register application callbacks
 */
extern bStatus_t CustomService_RegisterAppCBs(customServiceCB_t appCallbacks);

/*
 * CustomService_SetParameter - Set a service parameter
 */
extern bStatus_t CustomService_SetParameter(uint8 param, uint16 len, void *value);

/*
 * CustomService_GetParameter - Get a service parameter
 */
extern bStatus_t CustomService_GetParameter(uint8 param, uint16 *len, void *value);

/*
 * CustomService_Notify - Send notification to client
 */
extern bStatus_t CustomService_Notify(uint8 param, uint16 len, uint8 *value);

#ifdef __cplusplus
}
#endif

#endif /* CUSTOMSERVICE_H */
```

#### Step 2: Create Service Implementation

Create `custom_service.c`:

```c
#include "CONFIG.h"
#include "custom_service.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define SERVAPP_NUM_ATTR_SUPPORTED    11  // Number of attributes

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Service UUID
static CONST uint8 customServiceUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(CUSTOMSERVICE_SERV_UUID), HI_UINT16(CUSTOMSERVICE_SERV_UUID)
};

// Characteristic 1 UUID (Read)
static CONST uint8 customServiceChar1UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(CUSTOMSERVICE_CHAR1_UUID), HI_UINT16(CUSTOMSERVICE_CHAR1_UUID)
};

// Characteristic 2 UUID (Write)
static CONST uint8 customServiceChar2UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(CUSTOMSERVICE_CHAR2_UUID), HI_UINT16(CUSTOMSERVICE_CHAR2_UUID)
};

// Characteristic 3 UUID (Notify)
static CONST uint8 customServiceChar3UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(CUSTOMSERVICE_CHAR3_UUID), HI_UINT16(CUSTOMSERVICE_CHAR3_UUID)
};

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static customServiceCB_t customService_AppCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// Service attribute
static CONST gattAttrType_t customService = {ATT_BT_UUID_SIZE, customServiceUUID};

// Characteristic 1 Properties
static uint8 customServiceChar1Props = GATT_PROP_READ;

// Characteristic 1 Value
static uint8 customServiceChar1[20] = {0};

// Characteristic 1 User Description
static uint8 customServiceChar1UserDesc[] = "Sensor Data";

// Characteristic 2 Properties
static uint8 customServiceChar2Props = GATT_PROP_WRITE;

// Characteristic 2 Value
static uint8 customServiceChar2[20] = {0};

// Characteristic 2 User Description
static uint8 customServiceChar2UserDesc[] = "Control Command";

// Characteristic 3 Properties
static uint8 customServiceChar3Props = GATT_PROP_NOTIFY;

// Characteristic 3 Value
static uint8 customServiceChar3[20] = {0};

// Characteristic 3 CCCD (Client Characteristic Configuration Descriptor)
static gattCharCfg_t customServiceChar3Config[PERIPHERAL_MAX_CONNECTION];

// Characteristic 3 User Description
static uint8 customServiceChar3UserDesc[] = "Sensor Notifications";

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t customServiceAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] = {
    // Service Declaration
    {
        {ATT_BT_UUID_SIZE, primaryServiceUUID}, /* type */
        GATT_PERMIT_READ,                        /* permissions */
        0,                                       /* handle */
        (uint8 *)&customService                  /* pValue */
    },

    // Characteristic 1 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &customServiceChar1Props
    },
    // Characteristic 1 Value
    {
        {ATT_BT_UUID_SIZE, customServiceChar1UUID},
        GATT_PERMIT_READ,
        0,
        customServiceChar1
    },
    // Characteristic 1 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        customServiceChar1UserDesc
    },

    // Characteristic 2 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &customServiceChar2Props
    },
    // Characteristic 2 Value
    {
        {ATT_BT_UUID_SIZE, customServiceChar2UUID},
        GATT_PERMIT_WRITE,
        0,
        customServiceChar2
    },
    // Characteristic 2 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        customServiceChar2UserDesc
    },

    // Characteristic 3 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &customServiceChar3Props
    },
    // Characteristic 3 Value
    {
        {ATT_BT_UUID_SIZE, customServiceChar3UUID},
        0,  // No direct read/write permissions for notify characteristic
        0,
        customServiceChar3
    },
    // Characteristic 3 CCCD
    {
        {ATT_BT_UUID_SIZE, clientCharCfgUUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *)customServiceChar3Config
    },
    // Characteristic 3 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        customServiceChar3UserDesc
    },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t customService_ReadAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                           uint8 *pValue, uint16 *pLen, uint16 offset,
                                           uint16 maxLen, uint8 method);
static bStatus_t customService_WriteAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                            uint8 *pValue, uint16 len, uint16 offset,
                                            uint8 method);

/*********************************************************************
 * PROFILE CALLBACKS
 */

// Service Callbacks
CONST gattServiceCBs_t customServiceCBs = {
    customService_ReadAttrCB,  // Read callback function pointer
    customService_WriteAttrCB, // Write callback function pointer
    NULL                       // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      CustomService_AddService
 *
 * @brief   Initialize the Custom Service by registering GATT attributes
 *
 * @return  Success or Failure
 */
bStatus_t CustomService_AddService(void)
{
    uint8 status = SUCCESS;

    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, customServiceChar3Config);

    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService(customServiceAttrTbl,
                                          GATT_NUM_ATTRS(customServiceAttrTbl),
                                          GATT_MAX_ENCRYPT_KEY_SIZE,
                                          &customServiceCBs);

    return (status);
}

/*********************************************************************
 * @fn      CustomService_RegisterAppCBs
 *
 * @brief   Register application callbacks
 *
 * @param   appCallbacks - pointer to application callbacks
 *
 * @return  SUCCESS or INVALIDPARAMETER
 */
bStatus_t CustomService_RegisterAppCBs(customServiceCB_t appCallbacks)
{
    if(appCallbacks)
    {
        customService_AppCBs = appCallbacks;
        return (SUCCESS);
    }
    else
    {
        return (bleInvalidParameter);
    }
}

/*********************************************************************
 * @fn      CustomService_SetParameter
 *
 * @brief   Set a Custom Service parameter
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to write
 * @param   value - pointer to data to write
 *
 * @return  bStatus_t
 */
bStatus_t CustomService_SetParameter(uint8 param, uint16 len, void *value)
{
    bStatus_t ret = SUCCESS;

    switch(param)
    {
        case CUSTOMSERVICE_CHAR1_INDEX:
            if(len <= 20)
            {
                tmos_memcpy(customServiceChar1, value, len);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        case CUSTOMSERVICE_CHAR3_INDEX:
            if(len <= 20)
            {
                tmos_memcpy(customServiceChar3, value, len);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}

/*********************************************************************
 * @fn      CustomService_GetParameter
 *
 * @brief   Get a Custom Service parameter
 *
 * @param   param - Profile parameter ID
 * @param   len - pointer to a variable that contains the maximum length
 *                that can be written. Updated with actual written length.
 * @param   value - pointer to data to get
 *
 * @return  bStatus_t
 */
bStatus_t CustomService_GetParameter(uint8 param, uint16 *len, void *value)
{
    bStatus_t ret = SUCCESS;

    switch(param)
    {
        case CUSTOMSERVICE_CHAR1_INDEX:
            tmos_memcpy(value, customServiceChar1, 20);
            *len = 20;
            break;

        case CUSTOMSERVICE_CHAR2_INDEX:
            tmos_memcpy(value, customServiceChar2, 20);
            *len = 20;
            break;

        case CUSTOMSERVICE_CHAR3_INDEX:
            tmos_memcpy(value, customServiceChar3, 20);
            *len = 20;
            break;

        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}

/*********************************************************************
 * @fn      CustomService_Notify
 *
 * @brief   Send a notification containing a characteristic value
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to notify
 * @param   value - pointer to data to notify
 *
 * @return  bStatus_t
 */
bStatus_t CustomService_Notify(uint8 param, uint16 len, uint8 *value)
{
    attHandleValueNoti_t noti;

    if(param == CUSTOMSERVICE_CHAR3_INDEX)
    {
        // Set the handle
        noti.handle = customServiceAttrTbl[8].handle;

        // Set the length and value
        noti.len = len;
        tmos_memcpy(noti.pValue, value, len);

        // Send notification
        return GATT_Notification(0, &noti, FALSE);
    }

    return bleInvalidParameter;
}

/*********************************************************************
 * @fn          customService_ReadAttrCB
 *
 * @brief       Read an attribute
 *
 * @return      Success or Failure
 */
static bStatus_t customService_ReadAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                           uint8 *pValue, uint16 *pLen, uint16 offset,
                                           uint16 maxLen, uint8 method)
{
    bStatus_t status = SUCCESS;

    // Make sure it's not a blob operation (no attributes in the profile are long)
    if(offset > 0)
    {
        return (ATT_ERR_ATTR_NOT_LONG);
    }

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

        switch(uuid)
        {
            case CUSTOMSERVICE_CHAR1_UUID:
                *pLen = 20;
                tmos_memcpy(pValue, pAttr->pValue, 20);

                // Notify application
                if(customService_AppCBs)
                {
                    customService_AppCBs(CUSTOMSERVICE_CHAR1_READ_EVENT, NULL, 0);
                }
                break;

            default:
                *pLen = 0;
                status = ATT_ERR_ATTR_NOT_FOUND;
                break;
        }
    }
    else
    {
        // 128-bit UUID (not used in this example)
        *pLen = 0;
        status = ATT_ERR_INVALID_HANDLE;
    }

    return (status);
}

/*********************************************************************
 * @fn      customService_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @return  Success or Failure
 */
static bStatus_t customService_WriteAttrCB(uint16 connHandle, gattAttribute_t *pAttr,
                                            uint8 *pValue, uint16 len, uint16 offset,
                                            uint8 method)
{
    bStatus_t status = SUCCESS;

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

        switch(uuid)
        {
            case CUSTOMSERVICE_CHAR2_UUID:
                if(len <= 20)
                {
                    tmos_memcpy(pAttr->pValue, pValue, len);

                    // Notify application
                    if(customService_AppCBs)
                    {
                        customService_AppCBs(CUSTOMSERVICE_CHAR2_WRITE_EVENT, pValue, len);
                    }
                }
                else
                {
                    status = ATT_ERR_INVALID_VALUE_SIZE;
                }
                break;

            case GATT_CLIENT_CHAR_CFG_UUID:
                status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len,
                                                         offset, GATT_CLIENT_CFG_NOTIFY);
                break;

            default:
                status = ATT_ERR_ATTR_NOT_FOUND;
                break;
        }
    }
    else
    {
        // 128-bit UUID
        status = ATT_ERR_INVALID_HANDLE;
    }

    return (status);
}
```

#### Step 3: Integrate Service into Application

In your main application file (e.g., `peripheral.c`):

```c
#include "custom_service.h"

// Add service callback handler
static void customServiceCB(uint8 event, uint8 *data, uint16 len)
{
    switch(event)
    {
        case CUSTOMSERVICE_CHAR1_READ_EVENT:
            PRINT("Characteristic 1 read\n");
            // Update sensor data before read
            break;

        case CUSTOMSERVICE_CHAR2_WRITE_EVENT:
            PRINT("Characteristic 2 written, len=%d\n", len);
            // Process received command
            if(len > 0)
            {
                PRINT("Command: 0x%02X\n", data[0]);
                // Execute command
            }
            break;
    }
}

// In your initialization function
void Peripheral_Init()
{
    // ... existing initialization code ...

    // Add custom service
    CustomService_AddService();
    CustomService_RegisterAppCBs(customServiceCB);

    // ... rest of initialization ...
}

// Example: Sending notification
void sendSensorData(uint8 *data, uint16 len)
{
    // Update characteristic value
    CustomService_SetParameter(CUSTOMSERVICE_CHAR3_INDEX, len, data);

    // Send notification
    CustomService_Notify(CUSTOMSERVICE_CHAR3_INDEX, len, data);
}
```

#### Step 4: Update Build Configuration

Add your new files to the project:
- In your IDE (MounRiver Studio), add `custom_service.c` and `custom_service.h` to the project
- Or update your Makefile to include the new source files

---

## How to Integrate a New Sensor

### Overview

Sensor integration typically involves:
1. **Hardware Setup**: Configure GPIO, I2C, SPI, or ADC
2. **Driver Implementation**: Read sensor data
3. **Data Processing**: Convert raw data to meaningful values
4. **BLE Integration**: Send sensor data via BLE notifications

### Example 1: ADC-Based Sensor (Analog)

Let's integrate a temperature sensor using ADC.

#### Hardware Configuration

```c
#include "CH59x_common.h"

void ADC_Init_Temperature(void)
{
    // Enable ADC power
    R8_ADC_CFG = RB_ADC_POWER_ON;

    // Configure ADC: 12-bit, internal temperature sensor
    R8_ADC_CHANNEL = 0;  // Internal temperature channel
    R8_ADC_CFG |= RB_ADC_OFS_TEST;  // Use internal reference

    // Delay for ADC stabilization
    DelayUs(10);
}

uint16 ADC_Read_Temperature_Raw(void)
{
    uint16 result;

    // Start conversion
    R8_ADC_CONVERT = RB_ADC_START;

    // Wait for conversion complete
    while(R8_ADC_CONVERT & RB_ADC_START);

    // Read result
    result = R16_ADC_DATA;

    return result;
}

// Convert ADC value to temperature (Celsius)
int16 ADC_Convert_Temperature(uint16 rawValue)
{
    // CH592 internal temperature formula
    // T = (rawValue - calibration_value) * scale_factor
    // Use factory calibration values or empirical calibration

    int16 temperature;

    // Example conversion (adjust based on datasheet)
    temperature = (int16)((rawValue * 330 / 4096) - 50);

    return temperature;
}
```

#### Integration with BLE Service

```c
// In your application file

#define TEMP_SENSOR_PERIOD          1000  // Read every 1 second

static uint16 Peripheral_ProcessEvent(uint8 task_id, uint16 events)
{
    // ... existing event handlers ...

    if(events & TEMP_SENSOR_EVENT)
    {
        // Read temperature
        uint16 raw = ADC_Read_Temperature_Raw();
        int16 temp = ADC_Convert_Temperature(raw);

        PRINT("Temperature: %d C\n", temp);

        // Send via BLE (using custom service from previous example)
        uint8 data[2];
        data[0] = (uint8)(temp & 0xFF);
        data[1] = (uint8)((temp >> 8) & 0xFF);

        CustomService_SetParameter(CUSTOMSERVICE_CHAR1_INDEX, 2, data);
        CustomService_Notify(CUSTOMSERVICE_CHAR3_INDEX, 2, data);

        // Schedule next reading
        tmos_start_task(Peripheral_TaskID, TEMP_SENSOR_EVENT, TEMP_SENSOR_PERIOD);

        return (events ^ TEMP_SENSOR_EVENT);
    }

    return 0;
}

void Peripheral_Init()
{
    // ... existing initialization ...

    ADC_Init_Temperature();

    // Start periodic temperature readings
    tmos_start_task(Peripheral_TaskID, TEMP_SENSOR_EVENT, TEMP_SENSOR_PERIOD);
}
```

### Example 2: I2C-Based Sensor

Let's integrate a BME280 environmental sensor (I2C).

#### I2C Driver Implementation

```c
#include "CH59x_common.h"

#define BME280_I2C_ADDR     0x76  // Default I2C address

// I2C initialization
void I2C_Init_Master(void)
{
    // Configure I2C pins
    GPIOB_ModeCfg(GPIO_Pin_12 | GPIO_Pin_13, GPIO_ModeIN_PU);  // SCL, SDA

    // Enable I2C clock
    R8_I2C_CTRL_MOD = RB_I2C_MASTER;  // Master mode
    R16_I2C_CLK_DIV = 60;  // Set I2C clock (400kHz @ 60MHz system clock)
}

// I2C write byte
uint8 I2C_Write_Byte(uint8 dev_addr, uint8 reg_addr, uint8 data)
{
    uint8 i;

    // Start condition
    R8_I2C_CTRL_MOD |= RB_I2C_START;
    while(!(R8_I2C_INT_FLAG & RB_I2C_IF_START));
    R8_I2C_INT_FLAG = RB_I2C_IF_START;

    // Send device address + write
    R8_I2C_DATA = (dev_addr << 1) | 0;
    R8_I2C_CTRL_MOD &= ~RB_I2C_START;
    while(!(R8_I2C_INT_FLAG & RB_I2C_IF_TX_END));
    R8_I2C_INT_FLAG = RB_I2C_IF_TX_END;

    // Check ACK
    if(R8_I2C_CTRL_MOD & RB_I2C_ACK)
        return 1;  // NACK received

    // Send register address
    R8_I2C_DATA = reg_addr;
    while(!(R8_I2C_INT_FLAG & RB_I2C_IF_TX_END));
    R8_I2C_INT_FLAG = RB_I2C_IF_TX_END;

    if(R8_I2C_CTRL_MOD & RB_I2C_ACK)
        return 1;

    // Send data
    R8_I2C_DATA = data;
    while(!(R8_I2C_INT_FLAG & RB_I2C_IF_TX_END));
    R8_I2C_INT_FLAG = RB_I2C_IF_TX_END;

    // Stop condition
    R8_I2C_CTRL_MOD |= RB_I2C_STOP;
    while(R8_I2C_INT_FLAG & RB_I2C_IF_STOP);

    return 0;  // Success
}

// I2C read bytes
uint8 I2C_Read_Bytes(uint8 dev_addr, uint8 reg_addr, uint8 *buffer, uint16 len)
{
    uint16 i;

    // Start condition
    R8_I2C_CTRL_MOD |= RB_I2C_START;
    while(!(R8_I2C_INT_FLAG & RB_I2C_IF_START));
    R8_I2C_INT_FLAG = RB_I2C_IF_START;

    // Send device address + write
    R8_I2C_DATA = (dev_addr << 1) | 0;
    R8_I2C_CTRL_MOD &= ~RB_I2C_START;
    while(!(R8_I2C_INT_FLAG & RB_I2C_IF_TX_END));
    R8_I2C_INT_FLAG = RB_I2C_IF_TX_END;

    if(R8_I2C_CTRL_MOD & RB_I2C_ACK)
        return 1;

    // Send register address
    R8_I2C_DATA = reg_addr;
    while(!(R8_I2C_INT_FLAG & RB_I2C_IF_TX_END));
    R8_I2C_INT_FLAG = RB_I2C_IF_TX_END;

    // Repeated start
    R8_I2C_CTRL_MOD |= RB_I2C_START;
    while(!(R8_I2C_INT_FLAG & RB_I2C_IF_START));
    R8_I2C_INT_FLAG = RB_I2C_IF_START;

    // Send device address + read
    R8_I2C_DATA = (dev_addr << 1) | 1;
    R8_I2C_CTRL_MOD &= ~RB_I2C_START;
    while(!(R8_I2C_INT_FLAG & RB_I2C_IF_TX_END));
    R8_I2C_INT_FLAG = RB_I2C_IF_TX_END;

    // Read data bytes
    for(i = 0; i < len; i++)
    {
        if(i == len - 1)
        {
            // Last byte - send NACK
            R8_I2C_CTRL_MOD |= RB_I2C_ACK;
        }

        while(!(R8_I2C_INT_FLAG & RB_I2C_IF_RX_END));
        buffer[i] = R8_I2C_DATA;
        R8_I2C_INT_FLAG = RB_I2C_IF_RX_END;
    }

    // Stop condition
    R8_I2C_CTRL_MOD |= RB_I2C_STOP;
    while(R8_I2C_INT_FLAG & RB_I2C_IF_STOP);

    return 0;
}

// BME280 initialization
uint8 BME280_Init(void)
{
    uint8 chip_id;

    // Read chip ID (should be 0x60)
    if(I2C_Read_Bytes(BME280_I2C_ADDR, 0xD0, &chip_id, 1) != 0)
        return 1;

    if(chip_id != 0x60)
        return 2;  // Wrong chip ID

    // Soft reset
    I2C_Write_Byte(BME280_I2C_ADDR, 0xE0, 0xB6);
    DelayMs(10);

    // Configure sensor (example: normal mode, oversampling x1)
    I2C_Write_Byte(BME280_I2C_ADDR, 0xF2, 0x01);  // Humidity oversampling
    I2C_Write_Byte(BME280_I2C_ADDR, 0xF4, 0x27);  // Temp/pressure oversampling, normal mode
    I2C_Write_Byte(BME280_I2C_ADDR, 0xF5, 0xA0);  // Config: standby 1s, filter off

    return 0;
}

// Read BME280 sensor data
uint8 BME280_Read_Data(int32 *temperature, uint32 *pressure, uint32 *humidity)
{
    uint8 data[8];
    int32 adc_T, adc_P, adc_H;

    // Read all sensor data (burst read from 0xF7)
    if(I2C_Read_Bytes(BME280_I2C_ADDR, 0xF7, data, 8) != 0)
        return 1;

    // Parse raw ADC values
    adc_P = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    adc_T = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
    adc_H = (data[6] << 8) | data[7];

    // Compensate values (simplified - use full compensation from BME280 datasheet)
    *temperature = adc_T / 100;  // Simplified conversion
    *pressure = adc_P / 256;     // Simplified conversion
    *humidity = adc_H / 1024;    // Simplified conversion

    return 0;
}
```

#### Integration with BLE

```c
#define BME280_SENSOR_PERIOD     2000  // Read every 2 seconds

static uint16 Peripheral_ProcessEvent(uint8 task_id, uint16 events)
{
    if(events & BME280_SENSOR_EVENT)
    {
        int32 temp;
        uint32 pressure, humidity;

        // Read sensor
        if(BME280_Read_Data(&temp, &pressure, &humidity) == 0)
        {
            PRINT("Temp: %d C, Pressure: %d Pa, Humidity: %d%%\n",
                   temp, pressure, humidity);

            // Pack data for BLE transmission
            uint8 data[12];
            data[0] = (uint8)(temp & 0xFF);
            data[1] = (uint8)((temp >> 8) & 0xFF);
            data[2] = (uint8)((temp >> 16) & 0xFF);
            data[3] = (uint8)((temp >> 24) & 0xFF);
            data[4] = (uint8)(pressure & 0xFF);
            data[5] = (uint8)((pressure >> 8) & 0xFF);
            data[6] = (uint8)((pressure >> 16) & 0xFF);
            data[7] = (uint8)((pressure >> 24) & 0xFF);
            data[8] = (uint8)(humidity & 0xFF);
            data[9] = (uint8)((humidity >> 8) & 0xFF);
            data[10] = (uint8)((humidity >> 16) & 0xFF);
            data[11] = (uint8)((humidity >> 24) & 0xFF);

            // Send via BLE
            CustomService_Notify(CUSTOMSERVICE_CHAR3_INDEX, 12, data);
        }

        // Schedule next reading
        tmos_start_task(Peripheral_TaskID, BME280_SENSOR_EVENT, BME280_SENSOR_PERIOD);

        return (events ^ BME280_SENSOR_EVENT);
    }

    return 0;
}

void Peripheral_Init()
{
    // Initialize I2C
    I2C_Init_Master();

    // Initialize BME280
    if(BME280_Init() == 0)
    {
        PRINT("BME280 initialized successfully\n");

        // Start periodic readings
        tmos_start_task(Peripheral_TaskID, BME280_SENSOR_EVENT, BME280_SENSOR_PERIOD);
    }
    else
    {
        PRINT("BME280 initialization failed\n");
    }
}
```

### Example 3: SPI-Based Sensor

For SPI sensors (e.g., accelerometer, gyroscope):

```c
void SPI_Init_Master(void)
{
    // Configure SPI pins
    GPIOA_ModeCfg(GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14, GPIO_ModeOut_PP_5mA);  // SCK, MOSI, CS
    GPIOA_ModeCfg(GPIO_Pin_15, GPIO_ModeIN_PU);  // MISO

    // Configure SPI module
    R8_SPI0_CTRL_MOD = RB_SPI_MOSI_OE | RB_SPI_SCK_OE;  // Enable MOSI and SCK output
    R8_SPI0_CTRL_CFG = RB_SPI_AUTO_IF;  // Auto clear IF flag
    R16_SPI0_CTRL_CFG |= 0x08;  // Master mode
    R8_SPI0_CLOCK_DIV = 20;  // SPI clock divider (3MHz @ 60MHz system clock)
}

uint8 SPI_ReadWrite_Byte(uint8 data)
{
    R8_SPI0_BUFFER = data;
    while(!(R8_SPI0_INT_FLAG & RB_SPI_IF_BYTE_END));  // Wait for transfer complete
    return R8_SPI0_BUFFER;
}

// Example: Read accelerometer register
uint8 ACCEL_Read_Register(uint8 reg_addr)
{
    uint8 value;

    // CS low
    GPIOA_ResetBits(GPIO_Pin_14);

    // Send read command (register address with read bit)
    SPI_ReadWrite_Byte(reg_addr | 0x80);

    // Read data
    value = SPI_ReadWrite_Byte(0xFF);

    // CS high
    GPIOA_SetBits(GPIO_Pin_14);

    return value;
}
```

---

## How to Modify Existing Services

### Modifying Service Characteristics

To modify an existing service (e.g., add a characteristic to Heart Rate Service):

#### Step 1: Locate Service Files

Find the service implementation:
- Service header: `EVT/EXAM/BLE/HeartRate/Profile/heartrate.h`
- Service implementation: `EVT/EXAM/BLE/HeartRate/Profile/heartrate.c`

#### Step 2: Add New Characteristic UUID

In `heartrate.h`:

```c
// Add new characteristic UUID
#define HEARTRATE_ENERGY_UUID           0x2A08  // Example: Energy Expended

// Add new characteristic index
#define HEARTRATE_ENERGY_CHAR           3
```

#### Step 3: Update Attribute Table

In `heartrate.c`:

```c
// Add characteristic properties
static uint8 heartRateEnergyProps = GATT_PROP_READ | GATT_PROP_NOTIFY;

// Add characteristic value storage
static uint16 heartRateEnergy = 0;

// Add CCCD storage
static gattCharCfg_t heartRateEnergyConfig[PERIPHERAL_MAX_CONNECTION];

// Update attribute table
static gattAttribute_t heartRateAttrTbl[] = {
    // ... existing attributes ...

    // New characteristic declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &heartRateEnergyProps
    },

    // New characteristic value
    {
        {ATT_BT_UUID_SIZE, heartRateEnergyUUID},
        GATT_PERMIT_READ,
        0,
        (uint8 *)&heartRateEnergy
    },

    // New CCCD
    {
        {ATT_BT_UUID_SIZE, clientCharCfgUUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *)heartRateEnergyConfig
    },
};
```

#### Step 4: Update Service Functions

Add getter/setter functions:

```c
bStatus_t HeartRate_SetParameter(uint8 param, uint8 len, void *value)
{
    bStatus_t ret = SUCCESS;

    switch(param)
    {
        // ... existing cases ...

        case HEARTRATE_ENERGY_CHAR:
            if(len == sizeof(uint16))
            {
                heartRateEnergy = *((uint16 *)value);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return ret;
}
```

### Changing Characteristic Properties

To change a characteristic from read-only to read/write:

```c
// Change properties
static uint8 myCharProps = GATT_PROP_READ | GATT_PROP_WRITE;

// Update permissions in attribute table
{
    {ATT_BT_UUID_SIZE, myCharUUID},
    GATT_PERMIT_READ | GATT_PERMIT_WRITE,  // Add write permission
    0,
    myCharValue
}

// Add write handler in WriteAttrCB
static bStatus_t myService_WriteAttrCB(...)
{
    switch(uuid)
    {
        case MY_CHAR_UUID:
            // Handle write
            tmos_memcpy(pAttr->pValue, pValue, len);
            // Notify application
            if(myService_AppCBs)
            {
                myService_AppCBs(MY_CHAR_WRITE_EVENT, pValue, len);
            }
            break;
    }
}
```

### Changing Data Format

To change the data format of a characteristic:

```c
// Old format: single uint8
static uint8 oldValue;

// New format: array of values
static uint8 newValue[10];

// Update in SetParameter
case MY_CHAR_INDEX:
    if(len <= 10)  // Changed from 1 to 10
    {
        tmos_memcpy(newValue, value, len);
    }
    break;
```

---

## How to Remove Services

### Removing a BLE Service

To remove a service you don't need:

#### Step 1: Comment Out Service Registration

In your application file:

```c
void Peripheral_Init()
{
    // ... existing code ...

    // Comment out unwanted service
    // BattService_AddService();
    // DevInfo_AddService();

    // Keep only needed services
    HeartRate_AddService();
    CustomService_AddService();
}
```

#### Step 2: Remove Service Files (Optional)

If you want to reduce code size:
1. Remove service `.c` and `.h` files from project
2. Remove `#include` statements
3. Update build configuration

#### Step 3: Reduce Memory Usage

Update `CONFIG.h` to reduce BLE heap if fewer services:

```c
// Reduce heap size when using fewer services
#define BLE_MEMHEAP_SIZE    (1024 * 6)  // Reduced from default
```

### Disabling Peripheral Features

To disable unused peripherals and save power:

```c
// In your initialization
void System_Init()
{
    // Disable unused peripherals
    R8_SAFE_ACCESS_SIG = 0x57;
    R8_SAFE_ACCESS_SIG = 0xA8;

    // Disable ADC if not used
    R8_ADC_CFG &= ~RB_ADC_POWER_ON;

    // Disable SPI if not used
    R8_SPI0_CTRL_MOD = 0;

    // Disable UART if not used
    R8_UART1_IER = 0;

    R8_SAFE_ACCESS_SIG = 0;
}
```

---

## Advanced Integration Patterns

### Multi-Sensor Data Fusion

Combining multiple sensors:

```c
typedef struct {
    int16 temperature;
    uint32 pressure;
    uint32 humidity;
    int16 accel_x;
    int16 accel_y;
    int16 accel_z;
    uint8 battery_level;
} sensor_data_t;

sensor_data_t g_sensor_data;

void update_all_sensors(void)
{
    // Read temperature sensor
    g_sensor_data.temperature = read_temperature();

    // Read BME280
    BME280_Read_Data(NULL, &g_sensor_data.pressure, &g_sensor_data.humidity);

    // Read accelerometer
    read_accelerometer(&g_sensor_data.accel_x,
                       &g_sensor_data.accel_y,
                       &g_sensor_data.accel_z);

    // Read battery
    g_sensor_data.battery_level = read_battery_level();

    // Send combined data via BLE
    CustomService_Notify(SENSOR_DATA_CHAR, sizeof(sensor_data_t),
                         (uint8 *)&g_sensor_data);
}
```

### Power-Efficient Sensor Reading

Using sleep modes between readings:

```c
#define SENSOR_FAST_PERIOD      100   // 100ms when active
#define SENSOR_SLOW_PERIOD      5000  // 5s when idle

static uint8 sensor_mode = SENSOR_MODE_IDLE;

static uint16 Peripheral_ProcessEvent(uint8 task_id, uint16 events)
{
    if(events & SENSOR_READ_EVENT)
    {
        read_sensors();
        send_sensor_data();

        // Dynamic period based on activity
        uint16 next_period;
        if(sensor_mode == SENSOR_MODE_ACTIVE)
        {
            next_period = SENSOR_FAST_PERIOD;
        }
        else
        {
            next_period = SENSOR_SLOW_PERIOD;

            // Enter sleep mode
            LowPower_Sleep(RB_PWR_RAM14K | RB_PWR_RAM2K);
        }

        tmos_start_task(task_id, SENSOR_READ_EVENT, next_period);

        return (events ^ SENSOR_READ_EVENT);
    }
}

// Switch to active mode on BLE connection
static void peripheralStateNotificationCB(gapRole_States_t newState)
{
    if(newState == GAPROLE_CONNECTED)
    {
        sensor_mode = SENSOR_MODE_ACTIVE;
    }
    else if(newState == GAPROLE_WAITING)
    {
        sensor_mode = SENSOR_MODE_IDLE;
    }
}
```

### Error Handling and Recovery

Robust sensor error handling:

```c
#define SENSOR_MAX_RETRIES      3
#define SENSOR_ERROR_THRESHOLD  5

static uint8 sensor_error_count = 0;

uint8 read_sensor_with_retry(void)
{
    uint8 retry_count = 0;
    uint8 result;

    while(retry_count < SENSOR_MAX_RETRIES)
    {
        result = read_sensor();

        if(result == SUCCESS)
        {
            sensor_error_count = 0;  // Reset error counter
            return SUCCESS;
        }

        retry_count++;
        DelayMs(10);  // Wait before retry
    }

    // All retries failed
    sensor_error_count++;

    if(sensor_error_count >= SENSOR_ERROR_THRESHOLD)
    {
        // Reinitialize sensor
        PRINT("Sensor error threshold reached, reinitializing...\n");
        reinit_sensor();
        sensor_error_count = 0;
    }

    return FAILURE;
}

void reinit_sensor(void)
{
    // Power cycle sensor
    sensor_power_off();
    DelayMs(100);
    sensor_power_on();
    DelayMs(10);

    // Reinitialize
    sensor_init();
}
```

### Data Buffering and Batching

Efficient data transmission:

```c
#define BUFFER_SIZE     10

typedef struct {
    uint32 timestamp;
    int16 value;
} sensor_sample_t;

static sensor_sample_t sample_buffer[BUFFER_SIZE];
static uint8 buffer_index = 0;

void buffer_sensor_sample(int16 value)
{
    sample_buffer[buffer_index].timestamp = TMOS_GetSystemClock();
    sample_buffer[buffer_index].value = value;
    buffer_index++;

    // Send batch when buffer is full
    if(buffer_index >= BUFFER_SIZE)
    {
        send_buffer();
        buffer_index = 0;
    }
}

void send_buffer(void)
{
    // Send entire buffer at once (more efficient than individual samples)
    CustomService_Notify(SENSOR_DATA_CHAR,
                         sizeof(sensor_sample_t) * buffer_index,
                         (uint8 *)sample_buffer);
}
```

### Multi-Connection Service Management

Handling multiple BLE connections:

```c
#define MAX_CONNECTIONS     4

typedef struct {
    uint16 conn_handle;
    uint8 notification_enabled;
    uint8 data_rate;
} connection_info_t;

static connection_info_t connections[MAX_CONNECTIONS];

void handle_connection_event(uint16 conn_handle, uint8 event)
{
    uint8 i;

    if(event == CONNECTION_ESTABLISHED)
    {
        // Find free slot
        for(i = 0; i < MAX_CONNECTIONS; i++)
        {
            if(connections[i].conn_handle == INVALID_CONNHANDLE)
            {
                connections[i].conn_handle = conn_handle;
                connections[i].notification_enabled = FALSE;
                connections[i].data_rate = DATA_RATE_SLOW;
                break;
            }
        }
    }
    else if(event == CONNECTION_TERMINATED)
    {
        // Clear slot
        for(i = 0; i < MAX_CONNECTIONS; i++)
        {
            if(connections[i].conn_handle == conn_handle)
            {
                connections[i].conn_handle = INVALID_CONNHANDLE;
                break;
            }
        }
    }
}

void send_to_all_connections(uint8 *data, uint16 len)
{
    uint8 i;

    for(i = 0; i < MAX_CONNECTIONS; i++)
    {
        if(connections[i].conn_handle != INVALID_CONNHANDLE &&
           connections[i].notification_enabled)
        {
            // Send to this connection
            attHandleValueNoti_t noti;
            noti.handle = myCharHandle;
            noti.len = len;
            tmos_memcpy(noti.pValue, data, len);

            GATT_Notification(connections[i].conn_handle, &noti, FALSE);
        }
    }
}
```

---

## Best Practices and Tips

### 1. Memory Management

**Heap Usage:**
```c
// Monitor heap usage
uint32 free_heap = tmos_msg_get_free_mem();
PRINT("Free heap: %d bytes\n", free_heap);

// Allocate memory safely
uint8 *buffer = tmos_msg_allocate(size);
if(buffer == NULL)
{
    PRINT("Memory allocation failed!\n");
    // Handle error
}

// Always free allocated memory
tmos_msg_deallocate(buffer);
```

**Stack Usage:**
- Keep local variables small
- Avoid deep recursion
- Use static buffers for large data

### 2. Power Optimization

**Sleep Modes:**
```c
// Enter sleep mode when idle
void enter_sleep_mode(void)
{
    // Configure wake sources
    GPIOA_ITModeCfg(GPIO_Pin_0, GPIO_ITMode_FallEdge);  // Wake on button press
    PFIC_EnableIRQ(GPIO_A_IRQn);

    // Enter sleep (retains RAM)
    LowPower_Sleep(RB_PWR_RAM14K | RB_PWR_RAM2K);
}

// Low-frequency sensor reading
// Use longer intervals when battery is low
if(battery_level < 20)
{
    sensor_period = 10000;  // 10s
}
else
{
    sensor_period = 1000;   // 1s
}
```

**Peripheral Power Management:**
```c
// Disable peripherals when not in use
void disable_unused_peripherals(void)
{
    // Disable ADC
    R8_ADC_CFG &= ~RB_ADC_POWER_ON;

    // Disable unused GPIO
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_Floating);

    // Reduce system clock when idle
    SetSysClock(CLK_SOURCE_PLL_32MHz);  // Reduce from 60MHz
}
```

### 3. BLE Optimization

**Connection Parameters:**
```c
// Set optimal connection parameters
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL   20  // 25ms (units of 1.25ms)
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL   40  // 50ms
#define DEFAULT_DESIRED_SLAVE_LATENCY       0
#define DEFAULT_DESIRED_CONN_TIMEOUT        200 // 2s (units of 10ms)

// Update connection parameters
GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL,
                     sizeof(uint16),
                     &minInterval);
```

**Notification Frequency:**
```c
// Limit notification rate to avoid congestion
#define MIN_NOTIFICATION_INTERVAL   20  // 20ms

static uint32 last_notification_time = 0;

void send_notification_throttled(uint8 *data, uint16 len)
{
    uint32 current_time = TMOS_GetSystemClock();

    if((current_time - last_notification_time) >= MIN_NOTIFICATION_INTERVAL)
    {
        CustomService_Notify(CHAR_INDEX, len, data);
        last_notification_time = current_time;
    }
}
```

### 4. Debugging

**UART Debug Output:**
```c
// Enable UART debug in CONFIG.h
#define DEBUG_LEVEL     DEBUG_LEVEL_ALL

// Use debug macros
PRINT("Sensor value: %d\n", value);
PRINT("BLE connected, handle: %d\n", conn_handle);

// Debug with hex dump
void print_hex(uint8 *data, uint16 len)
{
    uint16 i;
    for(i = 0; i < len; i++)
    {
        PRINT("%02X ", data[i]);
        if((i + 1) % 16 == 0)
            PRINT("\n");
    }
    PRINT("\n");
}
```

**GPIO Debug Pins:**
```c
// Toggle GPIO for timing measurements
#define DEBUG_PIN   GPIO_Pin_0

void debug_pin_init(void)
{
    GPIOA_ModeCfg(DEBUG_PIN, GPIO_ModeOut_PP_5mA);
}

// Use with oscilloscope to measure execution time
GPIOA_SetBits(DEBUG_PIN);   // Start
function_to_measure();
GPIOA_ResetBits(DEBUG_PIN); // End
```

### 5. Code Organization

**Modular Design:**
```
MyProject/
├── APP/
│   ├── main.c
│   ├── peripheral.c
│   └── peripheral.h
├── Profile/
│   ├── custom_service.c
│   └── custom_service.h
├── HAL/
│   ├── sensor_driver.c
│   └── sensor_driver.h
└── CONFIG/
    ├── CONFIG.h
    └── MCU.c
```

**Header Guards:**
```c
#ifndef MY_HEADER_H
#define MY_HEADER_H

// Header content

#endif /* MY_HEADER_H */
```

### 6. Testing

**Unit Testing:**
```c
void test_sensor_reading(void)
{
    uint16 value;
    uint8 result;

    result = read_sensor(&value);

    if(result == SUCCESS)
    {
        if(value >= MIN_VALUE && value <= MAX_VALUE)
        {
            PRINT("Sensor test PASSED\n");
        }
        else
        {
            PRINT("Sensor test FAILED: value out of range\n");
        }
    }
    else
    {
        PRINT("Sensor test FAILED: read error\n");
    }
}
```

**BLE Testing Tools:**
- **nRF Connect** (Mobile app): Connect and test BLE services
- **LightBlue** (iOS): Explore services and characteristics
- **Bluetooth SIG tools**: Validate service UUIDs

### 7. Performance Considerations

**Efficient Data Structures:**
```c
// Use packed structures to save memory
typedef struct __attribute__((packed)) {
    uint16 temperature;
    uint16 humidity;
    uint8 battery;
} sensor_packet_t;

// Use bit fields for flags
typedef struct {
    uint8 sensor_enabled : 1;
    uint8 notification_enabled : 1;
    uint8 low_power_mode : 1;
    uint8 reserved : 5;
} flags_t;
```

**Fast Math:**
```c
// Use integer math instead of floating point
// Convert: temp_celsius = (adc_value * 330 / 4096) - 50
int16 temp = ((int32)adc_value * 330) >> 12;  // Divide by 4096 using shift
temp -= 50;

// Use lookup tables for complex calculations
static const uint16 temp_lookup[256] = { /* precalculated values */ };
```

### 8. Security

**Pairing and Bonding:**
```c
// Enable pairing in CONFIG.h
#define DEFAULT_PAIRING_MODE            GAPBOND_PAIRING_MODE_WAIT_FOR_REQ
#define DEFAULT_MITM_MODE               TRUE
#define DEFAULT_BONDING_MODE            GAPBOND_BONDING_ENABLED

// Set passkey
uint32 passkey = 123456;
GAPBondMgr_SetParameter(GAPBOND_DEFAULT_PASSCODE, sizeof(uint32), &passkey);
```

**Data Encryption:**
```c
// Simple XOR encryption (for demonstration)
void encrypt_data(uint8 *data, uint16 len, uint8 key)
{
    uint16 i;
    for(i = 0; i < len; i++)
    {
        data[i] ^= key;
    }
}
```

---

## Summary

This guide covered:

1. **Adding BLE Services**: Creating custom GATT services with characteristics
2. **Integrating Sensors**: ADC, I2C, and SPI sensor integration
3. **Modifying Services**: Changing existing service characteristics
4. **Removing Services**: Cleaning up unused code
5. **Advanced Patterns**: Multi-sensor fusion, power management, error handling
6. **Best Practices**: Memory management, debugging, optimization

### Reference Examples in Repository

- **Basic Peripheral**: `EVT/EXAM/BLE/Peripheral/`
- **Custom Service**: `EVT/EXAM/BLE/BLE_UART/`
- **ADC Examples**: `EVT/EXAM/ADC/`
- **I2C Examples**: `EVT/EXAM/I2C/`
- **SPI Examples**: `EVT/EXAM/SPI/`
- **Production App**: `Applications/8K_PollingRateWirelessMouse/`
- **Complete Integration Example**: `Applications/SmartFoodLabel/` - Full application demonstrating:
  - SHTxx I2C sensor integration
  - GDEY029T94 e-paper SPI display
  - Button handling with long-press detection
  - Custom UI system with font rendering
  - EEPROM data persistence
  - Event-driven architecture

### Next Steps

1. Start with example projects in `EVT/EXAM/`
2. Modify an existing example to add your sensor
3. Create custom service following the patterns in this guide
4. Test with BLE debugging tools (nRF Connect, LightBlue)
5. Optimize for power consumption
6. Deploy to production hardware

### Additional Resources

- **CH592 Datasheet**: `/Datasheet/CH592DS1.PDF`
- **EVT Documentation**: `/EVT/` directory
- **WCH Official Website**: www.wch.cn
- **Community Forums**: Search for CH592 development communities

---

*This guide is maintained as part of the CH592 firmware repository. For questions or contributions, please refer to the repository documentation.*
