/**
 * @file    main.c
 * @brief   Smart Food Label Application
 * @details Food labeling system with e-paper display, temperature sensor,
 *          and BLE connectivity for monitoring food storage
 */

#include "CH59x_common.h"
#include "shtxx.h"
#include "gdey029t94.h"
#include "button.h"
#include "food_label_ui.h"
#include "data_storage.h"

/*********************************************************************
 * CONSTANTS
 */

// Temperature measurement interval (10 minutes = 600000 ms)
#define TEMP_MEASURE_INTERVAL   600000

// Button polling interval (50 ms)
#define BUTTON_POLL_INTERVAL    50

// Display update interval when in edit mode (immediate)
#define DISPLAY_UPDATE_INTERVAL 1000

/*********************************************************************
 * GLOBAL VARIABLES
 */

static uint32 last_temp_measure_time = 0;
static uint32 last_button_poll_time = 0;
static uint32 last_display_update_time = 0;
static uint8 display_needs_update = 1;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief   Get system time in milliseconds
 */
static uint32 GetTimeMs(void)
{
    // TMOS system clock is in 625us units
    // Convert to milliseconds
    return (TMOS_GetSystemClock() * 5) >> 3;
}

/**
 * @brief   Button event callback
 */
static void Button_EventHandler(uint8 button_id, btn_event_t event)
{
    food_label_data_t *data = UI_GetData();

    switch(button_id) {
        case BUTTON_1:  // Edit/Log button
            if(event == BTN_EVENT_LONG_PRESS) {
                // Toggle edit mode
                if(UI_GetMode() == UI_MODE_DISPLAY) {
                    PRINT("Entering edit mode\n");
                    UI_SetMode(UI_MODE_EDIT);
                } else {
                    PRINT("Exiting edit mode, saving data\n");
                    UI_SetMode(UI_MODE_DISPLAY);

                    // Save data to storage
                    Storage_Save(data);
                }
                display_needs_update = 1;
            }
            else if(event == BTN_EVENT_SHORT_PRESS) {
                if(UI_GetMode() == UI_MODE_EDIT) {
                    // Move to next field in edit mode
                    UI_NextField();
                } else {
                    // Toggle logging state (in fridge / removed)
                    data->is_logging = !data->is_logging;
                    PRINT("Logging %s\n", data->is_logging ? "STARTED" : "STOPPED");
                    display_needs_update = 1;
                }
            }
            break;

        case BUTTON_2:  // Down button
            if(event == BTN_EVENT_SHORT_PRESS) {
                if(UI_GetMode() == UI_MODE_EDIT) {
                    UI_DecrementField();
                }
            }
            break;

        case BUTTON_3:  // Up button
            if(event == BTN_EVENT_SHORT_PRESS) {
                if(UI_GetMode() == UI_MODE_EDIT) {
                    UI_IncrementField();
                }
            }
            break;
    }
}

/**
 * @brief   Read temperature sensor
 */
static void ReadTemperature(void)
{
    sht_data_t sensor_data;
    food_label_data_t *label_data = UI_GetData();

    // Read sensor (only if logging is active)
    if(label_data->is_logging) {
        if(SHT_ReadData(&sensor_data) == 0) {
            // Update temperature
            label_data->temperature = sensor_data.temperature;
            PRINT("Temperature: %.1f C, Humidity: %.1f %%\n",
                   sensor_data.temperature, sensor_data.humidity);

            display_needs_update = 1;
        } else {
            PRINT("Failed to read sensor\n");
        }
    }
}

/**
 * @brief   Initialize system
 */
static void System_Init(void)
{
    // Set system clock to 60MHz
    SetSysClock(CLK_SOURCE_PLL_60MHz);

    // Enable UART for debugging
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();

    PRINT("\n\n========================================\n");
    PRINT("Smart Food Label System\n");
    PRINT("Version: 1.0\n");
    PRINT("========================================\n\n");

    // Initialize storage
    PRINT("Initializing storage...\n");
    Storage_Init();

    // Initialize button handler
    PRINT("Initializing buttons...\n");
    BTN_Init();
    BTN_RegisterCallback(Button_EventHandler);

    // Initialize UI and display
    PRINT("Initializing display...\n");
    UI_Init();

    // Initialize temperature sensor
    PRINT("Initializing SHT sensor...\n");
    if(SHT_Init(SHT_TYPE_SHT2X) == 0) {
        PRINT("SHT sensor initialized successfully\n");
    } else {
        PRINT("Failed to initialize SHT sensor\n");
    }

    // Try to load saved data
    food_label_data_t *data = UI_GetData();
    if(Storage_Load(data) == 0) {
        PRINT("Loaded saved data from storage\n");
    } else {
        PRINT("No saved data found, using defaults\n");
    }

    // Initial temperature reading
    ReadTemperature();

    // Update display
    UI_UpdateDisplay(data);

    PRINT("\nSystem initialized successfully\n");
    PRINT("Press Button 1 for 5 seconds to enter edit mode\n");
    PRINT("Press Button 1 briefly to toggle logging\n\n");
}

/**
 * @brief   Main processing loop
 */
static void System_Process(void)
{
    uint32 current_time = GetTimeMs();
    food_label_data_t *data = UI_GetData();

    // Poll buttons
    if((current_time - last_button_poll_time) >= BUTTON_POLL_INTERVAL) {
        BTN_Poll();
        last_button_poll_time = current_time;
    }

    // Measure temperature periodically
    if((current_time - last_temp_measure_time) >= TEMP_MEASURE_INTERVAL) {
        ReadTemperature();
        last_temp_measure_time = current_time;
    }

    // Update display if needed
    if(display_needs_update) {
        if((current_time - last_display_update_time) >= DISPLAY_UPDATE_INTERVAL) {
            UI_UpdateDisplay(data);
            display_needs_update = 0;
            last_display_update_time = current_time;
        }
    }

    // Enter low power mode when idle
    // (Wake on button press)
    DelayMs(10);
}

/*********************************************************************
 * MAIN FUNCTION
 */

int main(void)
{
    // Initialize system
    System_Init();

    // Main loop
    while(1) {
        System_Process();
    }
}

/*********************************************************************
 * INTERRUPT HANDLERS
 */

__attribute__((interrupt("WCH-Interrupt-fast")))
__attribute__((section(".highcode")))
void HardFault_Handler(void)
{
    PRINT("HardFault!\n");
    while(1);
}
