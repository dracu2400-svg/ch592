/**
 * @file    data_storage.h
 * @brief   Data Storage for Food Label (EEPROM/Flash)
 */

#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "CH59x_common.h"
#include "food_label_ui.h"

/*********************************************************************
 * CONSTANTS
 */

// Storage address in Data Flash
#define STORAGE_BASE_ADDR   0x00000000  // Start of data flash

/*********************************************************************
 * API FUNCTIONS
 */

/**
 * @brief   Initialize data storage
 * @return  0 = Success, 1 = Failure
 */
uint8 Storage_Init(void);

/**
 * @brief   Save food label data to storage
 * @param   data - Pointer to food label data
 * @return  0 = Success, 1 = Failure
 */
uint8 Storage_Save(food_label_data_t *data);

/**
 * @brief   Load food label data from storage
 * @param   data - Pointer to store loaded data
 * @return  0 = Success, 1 = Failure or no data
 */
uint8 Storage_Load(food_label_data_t *data);

/**
 * @brief   Erase stored data
 * @return  0 = Success, 1 = Failure
 */
uint8 Storage_Erase(void);

#ifdef __cplusplus
}
#endif

#endif /* DATA_STORAGE_H */
