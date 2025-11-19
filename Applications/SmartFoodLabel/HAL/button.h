/**
 * @file    button.h
 * @brief   Button Handler with Long Press Detection
 */

#ifndef BUTTON_H
#define BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "CH59x_common.h"

/*********************************************************************
 * CONSTANTS
 */

// Button pins (adjust based on hardware)
#define BTN1_PIN        GPIO_Pin_4   // PB4 - Button 1 (Edit/Log)
#define BTN2_PIN        GPIO_Pin_5   // PB5 - Button 2 (Down)
#define BTN3_PIN        GPIO_Pin_6   // PB6 - Button 3 (Up)

// Button IDs
#define BUTTON_1        0
#define BUTTON_2        1
#define BUTTON_3        2
#define BUTTON_COUNT    3

// Button states
#define BTN_RELEASED    0
#define BTN_PRESSED     1

// Press types
#define BTN_PRESS_SHORT 0
#define BTN_PRESS_LONG  1

// Timing (in milliseconds)
#define BTN_DEBOUNCE_TIME   50      // Debounce delay
#define BTN_LONG_PRESS_TIME 5000    // Long press threshold (5 seconds)
#define BTN_POLL_INTERVAL   50      // Polling interval

/*********************************************************************
 * TYPEDEFS
 */

typedef enum {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_SHORT_PRESS,
    BTN_EVENT_LONG_PRESS,
    BTN_EVENT_RELEASED
} btn_event_t;

// Button callback function type
typedef void (*btn_callback_t)(uint8 button_id, btn_event_t event);

/*********************************************************************
 * API FUNCTIONS
 */

/**
 * @brief   Initialize button handler
 */
void BTN_Init(void);

/**
 * @brief   Register button event callback
 * @param   callback - Callback function
 */
void BTN_RegisterCallback(btn_callback_t callback);

/**
 * @brief   Poll button states (call periodically)
 */
void BTN_Poll(void);

/**
 * @brief   Get button state
 * @param   button_id - Button ID (BUTTON_1, BUTTON_2, BUTTON_3)
 * @return  BTN_PRESSED or BTN_RELEASED
 */
uint8 BTN_GetState(uint8 button_id);

/**
 * @brief   Check if button is currently pressed
 * @param   button_id - Button ID
 * @return  TRUE if pressed, FALSE otherwise
 */
uint8 BTN_IsPressed(uint8 button_id);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_H */
