/**
 * @file    app_config.h
 * @brief   Application Configuration
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/*********************************************************************
 * SYSTEM CONFIGURATION
 */

// System clock frequency (Hz)
#define SYS_CLOCK_HZ            60000000

// UART configuration
#define UART_BAUD_RATE          115200

// Debug output enable
#define DEBUG_ENABLED           1

/*********************************************************************
 * SENSOR CONFIGURATION
 */

// SHTxx sensor type selection
// 0 = SHT2X, 1 = SHT3X
#define SHT_SENSOR_TYPE         0  // SHT2X

// Temperature measurement interval (milliseconds)
// Default: 600000 ms = 10 minutes
#define TEMP_MEASURE_INTERVAL   600000

/*********************************************************************
 * BUTTON CONFIGURATION
 */

// Button debounce time (milliseconds)
#define BUTTON_DEBOUNCE_MS      50

// Long press threshold (milliseconds)
#define BUTTON_LONG_PRESS_MS    5000

// Button poll interval (milliseconds)
#define BUTTON_POLL_INTERVAL_MS 50

/*********************************************************************
 * DISPLAY CONFIGURATION
 */

// Display update interval in edit mode (milliseconds)
#define DISPLAY_UPDATE_INTERVAL_MS  1000

// Display rotation
// 0 = 0°, 1 = 90°, 2 = 180°, 3 = 270°
#define DISPLAY_ROTATION        0

// Display refresh type
// 0 = Fast (partial update), 1 = Full (better quality)
#define DISPLAY_FULL_REFRESH    1

/*********************************************************************
 * STORAGE CONFIGURATION
 */

// Storage base address in EEPROM/Flash
#define STORAGE_BASE_ADDRESS    0x00000000

// Enable automatic save on edit mode exit
#define AUTO_SAVE_ENABLED       1

/*********************************************************************
 * POWER MANAGEMENT
 */

// Enable low power mode when idle
#define LOW_POWER_MODE_ENABLED  0

// Idle timeout before sleep (milliseconds)
#define IDLE_TIMEOUT_MS         30000

/*********************************************************************
 * PRODUCT TYPES
 */

// Default product types (can be customized)
#define PRODUCT_TYPE_COUNT      8

/*********************************************************************
 * DATE/TIME CONFIGURATION
 */

// Year range for date selection
#define MIN_YEAR                2020
#define MAX_YEAR                2099

// Default date if no RTC available
#define DEFAULT_DAY             1
#define DEFAULT_MONTH           1
#define DEFAULT_YEAR            2025

/*********************************************************************
 * TEMPERATURE THRESHOLDS
 */

// Temperature warning thresholds (Celsius)
#define TEMP_WARNING_LOW        0.0f   // Below 0°C warning
#define TEMP_WARNING_HIGH       8.0f   // Above 8°C warning

// Humidity thresholds (%)
#define HUMIDITY_WARNING_LOW    20.0f
#define HUMIDITY_WARNING_HIGH   90.0f

/*********************************************************************
 * PIN DEFINITIONS (can be overridden in HAL files)
 */

// E-Paper Display Pins
#define EPD_SPI_SCK_PIN         GPIO_Pin_12
#define EPD_SPI_MOSI_PIN        GPIO_Pin_13
#define EPD_SPI_MISO_PIN        GPIO_Pin_15
#define EPD_CS_PIN              GPIO_Pin_14
#define EPD_DC_PIN              GPIO_Pin_0
#define EPD_RST_PIN             GPIO_Pin_1
#define EPD_BUSY_PIN            GPIO_Pin_2

// SHTxx Sensor Pins
#define SHT_I2C_SCL_PIN         GPIO_Pin_12
#define SHT_I2C_SDA_PIN         GPIO_Pin_13

// Button Pins
#define BTN1_PIN                GPIO_Pin_4
#define BTN2_PIN                GPIO_Pin_5
#define BTN3_PIN                GPIO_Pin_6

// UART Debug Pins
#define UART_TX_PIN             GPIO_Pin_9
#define UART_RX_PIN             GPIO_Pin_8

/*********************************************************************
 * FEATURE FLAGS
 */

// Enable BLE connectivity (future feature)
#define BLE_ENABLED             0

// Enable alarm buzzer (future feature)
#define BUZZER_ENABLED          0

// Enable LED indicator (future feature)
#define LED_ENABLED             0

// Enable RTC module (future feature)
#define RTC_ENABLED             0

/*********************************************************************
 * DEBUG MACROS
 */

#if DEBUG_ENABLED
    #include <stdio.h>
    #define PRINT(...)          printf(__VA_ARGS__)
    #define DEBUG_PRINT(...)    printf(__VA_ARGS__)
#else
    #define PRINT(...)
    #define DEBUG_PRINT(...)
#endif

#endif /* APP_CONFIG_H */
