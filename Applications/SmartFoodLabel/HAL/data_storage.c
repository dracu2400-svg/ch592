/**
 * @file    data_storage.c
 * @brief   Data Storage Implementation using EEPROM
 */

#include "data_storage.h"
#include "CH59x_common.h"
#include <string.h>

/*********************************************************************
 * CONSTANTS
 */

#define STORAGE_MAGIC       0x464C4232  // "FLB2" - Food Label v2
#define STORAGE_VERSION     0x01

/*********************************************************************
 * TYPEDEFS
 */

typedef struct {
    uint32 magic;                   // Magic number for validation
    uint8 version;                  // Data format version
    food_label_data_t label_data;  // Food label data
    uint16 checksum;                // Simple checksum
} storage_record_t;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief   Calculate simple checksum
 */
static uint16 calc_checksum(uint8 *data, uint16 len)
{
    uint16 sum = 0;
    uint16 i;

    for(i = 0; i < len; i++) {
        sum += data[i];
    }

    return sum;
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief   Initialize data storage
 */
uint8 Storage_Init(void)
{
    // EEPROM is automatically initialized by the system
    // Nothing specific needed here
    return 0;
}

/**
 * @brief   Save food label data
 */
uint8 Storage_Save(food_label_data_t *data)
{
    storage_record_t record;
    uint8 *ptr = (uint8 *)&record;
    uint32 addr = STORAGE_BASE_ADDR;

    // Prepare storage record
    record.magic = STORAGE_MAGIC;
    record.version = STORAGE_VERSION;
    memcpy(&record.label_data, data, sizeof(food_label_data_t));

    // Calculate checksum (exclude magic, version, and checksum fields)
    record.checksum = calc_checksum((uint8 *)&record.label_data,
                                    sizeof(food_label_data_t));

    // Erase data flash block
    EEPROM_ERASE(addr, sizeof(storage_record_t));

    // Write data to EEPROM
    EEPROM_WRITE(addr, ptr, sizeof(storage_record_t));

    return 0;
}

/**
 * @brief   Load food label data
 */
uint8 Storage_Load(food_label_data_t *data)
{
    storage_record_t record;
    uint32 addr = STORAGE_BASE_ADDR;
    uint16 checksum;

    // Read data from EEPROM
    EEPROM_READ(addr, (uint8 *)&record, sizeof(storage_record_t));

    // Verify magic number
    if(record.magic != STORAGE_MAGIC) {
        return 1;  // No valid data
    }

    // Verify version
    if(record.version != STORAGE_VERSION) {
        return 1;  // Version mismatch
    }

    // Verify checksum
    checksum = calc_checksum((uint8 *)&record.label_data,
                             sizeof(food_label_data_t));
    if(checksum != record.checksum) {
        return 1;  // Checksum error
    }

    // Copy data
    memcpy(data, &record.label_data, sizeof(food_label_data_t));

    return 0;
}

/**
 * @brief   Erase stored data
 */
uint8 Storage_Erase(void)
{
    uint32 addr = STORAGE_BASE_ADDR;

    // Erase data flash block
    EEPROM_ERASE(addr, sizeof(storage_record_t));

    return 0;
}
