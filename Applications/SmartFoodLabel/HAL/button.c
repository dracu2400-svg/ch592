/**
 * @file    button.c
 * @brief   Button Handler Implementation with Long Press Detection
 */

#include "button.h"
#include "CH59x_common.h"

/*********************************************************************
 * LOCAL TYPEDEFS
 */

typedef struct {
    uint8 current_state;         // Current button state
    uint8 last_state;            // Last stable state
    uint32 press_time;           // Time when button was pressed
    uint8 long_press_triggered;  // Long press event already sent
} button_state_t;

/*********************************************************************
 * LOCAL VARIABLES
 */

static button_state_t button_states[BUTTON_COUNT];
static btn_callback_t event_callback = NULL;
static uint32 last_poll_time = 0;

// Button pin map
static const uint16 button_pins[BUTTON_COUNT] = {
    BTN1_PIN,
    BTN2_PIN,
    BTN3_PIN
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief   Read button GPIO state
 * @param   button_id - Button ID
 * @return  1 if pressed, 0 if released (active low)
 */
static uint8 BTN_ReadPin(uint8 button_id)
{
    if(button_id >= BUTTON_COUNT) {
        return 0;
    }

    // Buttons are active low (pressed = 0)
    return (GPIOB_ReadPortPin(button_pins[button_id]) == 0) ? BTN_PRESSED : BTN_RELEASED;
}

/**
 * @brief   Get system time in milliseconds
 */
static uint32 BTN_GetTimeMs(void)
{
    // TMOS system clock is in 625us units
    // Convert to milliseconds: ticks * 625 / 1000 = ticks * 5 / 8
    return (TMOS_GetSystemClock() * 5) >> 3;
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief   Initialize button handler
 */
void BTN_Init(void)
{
    uint8 i;

    // Configure button pins as input with pull-up
    GPIOB_ModeCfg(BTN1_PIN | BTN2_PIN | BTN3_PIN, GPIO_ModeIN_PU);

    // Initialize button states
    for(i = 0; i < BUTTON_COUNT; i++) {
        button_states[i].current_state = BTN_RELEASED;
        button_states[i].last_state = BTN_RELEASED;
        button_states[i].press_time = 0;
        button_states[i].long_press_triggered = 0;
    }

    last_poll_time = BTN_GetTimeMs();
}

/**
 * @brief   Register button event callback
 */
void BTN_RegisterCallback(btn_callback_t callback)
{
    event_callback = callback;
}

/**
 * @brief   Poll button states
 */
void BTN_Poll(void)
{
    uint8 i;
    uint32 current_time = BTN_GetTimeMs();
    uint32 press_duration;

    // Check if enough time has passed since last poll
    if((current_time - last_poll_time) < BTN_POLL_INTERVAL) {
        return;
    }

    last_poll_time = current_time;

    // Process each button
    for(i = 0; i < BUTTON_COUNT; i++) {
        uint8 pin_state = BTN_ReadPin(i);

        // Button just pressed (transition from released to pressed)
        if(pin_state == BTN_PRESSED && button_states[i].last_state == BTN_RELEASED) {
            button_states[i].current_state = BTN_PRESSED;
            button_states[i].press_time = current_time;
            button_states[i].long_press_triggered = 0;
        }
        // Button currently pressed
        else if(pin_state == BTN_PRESSED && button_states[i].last_state == BTN_PRESSED) {
            press_duration = current_time - button_states[i].press_time;

            // Check for long press
            if(press_duration >= BTN_LONG_PRESS_TIME && !button_states[i].long_press_triggered) {
                button_states[i].long_press_triggered = 1;

                // Trigger long press event
                if(event_callback != NULL) {
                    event_callback(i, BTN_EVENT_LONG_PRESS);
                }
            }
        }
        // Button just released (transition from pressed to released)
        else if(pin_state == BTN_RELEASED && button_states[i].last_state == BTN_PRESSED) {
            button_states[i].current_state = BTN_RELEASED;
            press_duration = current_time - button_states[i].press_time;

            // Trigger short press event (only if long press wasn't triggered)
            if(!button_states[i].long_press_triggered && press_duration >= BTN_DEBOUNCE_TIME) {
                if(event_callback != NULL) {
                    event_callback(i, BTN_EVENT_SHORT_PRESS);
                }
            }

            // Trigger release event
            if(event_callback != NULL) {
                event_callback(i, BTN_EVENT_RELEASED);
            }
        }

        // Update last state
        button_states[i].last_state = pin_state;
    }
}

/**
 * @brief   Get button state
 */
uint8 BTN_GetState(uint8 button_id)
{
    if(button_id >= BUTTON_COUNT) {
        return BTN_RELEASED;
    }

    return button_states[button_id].current_state;
}

/**
 * @brief   Check if button is currently pressed
 */
uint8 BTN_IsPressed(uint8 button_id)
{
    return (BTN_GetState(button_id) == BTN_PRESSED) ? TRUE : FALSE;
}
