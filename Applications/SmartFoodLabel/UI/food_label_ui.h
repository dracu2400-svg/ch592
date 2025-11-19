/**
 * @file    food_label_ui.h
 * @brief   Food Label UI - Display and Edit Modes
 */

#ifndef FOOD_LABEL_UI_H
#define FOOD_LABEL_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "CH59x_common.h"

/*********************************************************************
 * CONSTANTS
 */

// Product types
#define PRODUCT_FISH        0
#define PRODUCT_MEAT        1
#define PRODUCT_MILK        2
#define PRODUCT_VEGETABLES  3
#define PRODUCT_FRUITS      4
#define PRODUCT_CHEESE      5
#define PRODUCT_EGGS        6
#define PRODUCT_OTHER       7
#define PRODUCT_COUNT       8

// Edit fields
#define FIELD_PRODUCT       0
#define FIELD_CREATE_DAY    1
#define FIELD_CREATE_MONTH  2
#define FIELD_CREATE_YEAR   3
#define FIELD_EXPIRE_DAY    4
#define FIELD_EXPIRE_MONTH  5
#define FIELD_EXPIRE_YEAR   6
#define FIELD_INITIAL_1     7
#define FIELD_INITIAL_2     8
#define FIELD_COUNT         9

/*********************************************************************
 * TYPEDEFS
 */

typedef struct {
    uint8 day;
    uint8 month;
    uint16 year;
} date_t;

typedef struct {
    uint8 product_type;       // Product type (PRODUCT_xxx)
    date_t create_date;        // Creation date
    date_t expire_date;        // Expiration date
    char initials[3];          // Editor initials (2 letters + null)
    float temperature;         // Last measured temperature
    uint8 is_logging;          // Logging state (1 = in fridge, 0 = removed)
} food_label_data_t;

typedef enum {
    UI_MODE_DISPLAY = 0,
    UI_MODE_EDIT
} ui_mode_t;

/*********************************************************************
 * API FUNCTIONS
 */

/**
 * @brief   Initialize UI system
 */
void UI_Init(void);

/**
 * @brief   Set UI mode (display or edit)
 * @param   mode - UI mode
 */
void UI_SetMode(ui_mode_t mode);

/**
 * @brief   Get current UI mode
 * @return  Current mode
 */
ui_mode_t UI_GetMode(void);

/**
 * @brief   Update display with current data
 * @param   data - Pointer to food label data
 */
void UI_UpdateDisplay(food_label_data_t *data);

/**
 * @brief   Move to next edit field
 */
void UI_NextField(void);

/**
 * @brief   Increment current field value
 */
void UI_IncrementField(void);

/**
 * @brief   Decrement current field value
 */
void UI_DecrementField(void);

/**
 * @brief   Get pointer to current data
 * @return  Pointer to food label data
 */
food_label_data_t* UI_GetData(void);

/**
 * @brief   Get product name string
 * @param   product_type - Product type
 * @return  Product name
 */
const char* UI_GetProductName(uint8 product_type);

/**
 * @brief   Calculate days until expiration
 * @param   data - Pointer to food label data
 * @return  Days until expiration (negative if expired)
 */
int16 UI_CalculateDaysUntilExpire(food_label_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* FOOD_LABEL_UI_H */
