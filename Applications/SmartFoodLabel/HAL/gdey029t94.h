/**
 * @file    gdey029t94.h
 * @brief   GDEY029T94 2.9" E-Paper Display Driver (Good Display)
 * @details 296x128 pixels, Black/White, SPI interface
 */

#ifndef GDEY029T94_H
#define GDEY029T94_H

#ifdef __cplusplus
extern "C" {
#endif

#include "CH59x_common.h"

/*********************************************************************
 * CONSTANTS
 */

// Display dimensions
#define EPD_WIDTH       128
#define EPD_HEIGHT      296

// Display buffer size
#define EPD_BUFFER_SIZE ((EPD_WIDTH * EPD_HEIGHT) / 8)

// GPIO Pin definitions (adjust based on hardware connection)
#define EPD_CS_PIN      GPIO_Pin_14   // PA14 - Chip Select
#define EPD_DC_PIN      GPIO_Pin_0    // PA0  - Data/Command
#define EPD_RST_PIN     GPIO_Pin_1    // PA1  - Reset
#define EPD_BUSY_PIN    GPIO_Pin_2    // PA2  - Busy

// SPI pins (PA12=SCK, PA13=MOSI, PA15=MISO)
#define EPD_SPI_SCK     GPIO_Pin_12
#define EPD_SPI_MOSI    GPIO_Pin_13
#define EPD_SPI_MISO    GPIO_Pin_15

// Display commands
#define EPD_CMD_PANEL_SETTING           0x00
#define EPD_CMD_POWER_SETTING           0x01
#define EPD_CMD_POWER_OFF               0x02
#define EPD_CMD_POWER_OFF_SEQUENCE      0x03
#define EPD_CMD_POWER_ON                0x04
#define EPD_CMD_POWER_ON_MEASURE        0x05
#define EPD_CMD_BOOSTER_SOFT_START      0x06
#define EPD_CMD_DEEP_SLEEP              0x07
#define EPD_CMD_DATA_START_TRANSMISSION_1 0x10
#define EPD_CMD_DATA_STOP                0x11
#define EPD_CMD_DISPLAY_REFRESH          0x12
#define EPD_CMD_DATA_START_TRANSMISSION_2 0x13
#define EPD_CMD_VCOM_LUT                 0x20
#define EPD_CMD_W2W_LUT                  0x21
#define EPD_CMD_B2W_LUT                  0x22
#define EPD_CMD_W2B_LUT                  0x23
#define EPD_CMD_B2B_LUT                  0x24
#define EPD_CMD_PLL_CONTROL              0x30
#define EPD_CMD_TEMPERATURE_CALIBRATION  0x41
#define EPD_CMD_VCOM_AND_DATA_SETTING    0x50
#define EPD_CMD_TCON_SETTING             0x60
#define EPD_CMD_RESOLUTION_SETTING       0x61
#define EPD_CMD_GET_STATUS               0x71
#define EPD_CMD_PARTIAL_WINDOW           0x90
#define EPD_CMD_PARTIAL_IN               0x91
#define EPD_CMD_PARTIAL_OUT              0x92
#define EPD_CMD_PROGRAM_MODE             0xA0
#define EPD_CMD_ACTIVE_PROGRAMMING       0xA1
#define EPD_CMD_READ_OTP                 0xA2

// Colors
#define EPD_COLOR_BLACK  0x00
#define EPD_COLOR_WHITE  0xFF

/*********************************************************************
 * TYPEDEFS
 */

typedef enum {
    EPD_ROTATE_0 = 0,
    EPD_ROTATE_90,
    EPD_ROTATE_180,
    EPD_ROTATE_270
} epd_rotation_t;

/*********************************************************************
 * API FUNCTIONS
 */

/**
 * @brief   Initialize E-Paper display
 * @return  0 = Success, 1 = Failure
 */
uint8 EPD_Init(void);

/**
 * @brief   Clear display (fill with white)
 */
void EPD_Clear(void);

/**
 * @brief   Display buffer content
 */
void EPD_Display(void);

/**
 * @brief   Display buffer with full update
 */
void EPD_DisplayFull(void);

/**
 * @brief   Enter deep sleep mode
 */
void EPD_Sleep(void);

/**
 * @brief   Wake up from deep sleep
 */
void EPD_WakeUp(void);

/**
 * @brief   Set pixel at coordinates
 * @param   x - X coordinate
 * @param   y - Y coordinate
 * @param   color - EPD_COLOR_BLACK or EPD_COLOR_WHITE
 */
void EPD_SetPixel(uint16 x, uint16 y, uint8 color);

/**
 * @brief   Draw horizontal line
 * @param   x - Start X coordinate
 * @param   y - Y coordinate
 * @param   width - Line width
 * @param   color - Line color
 */
void EPD_DrawHLine(uint16 x, uint16 y, uint16 width, uint8 color);

/**
 * @brief   Draw vertical line
 * @param   x - X coordinate
 * @param   y - Start Y coordinate
 * @param   height - Line height
 * @param   color - Line color
 */
void EPD_DrawVLine(uint16 x, uint16 y, uint16 height, uint8 color);

/**
 * @brief   Fill rectangle
 * @param   x - Start X coordinate
 * @param   y - Start Y coordinate
 * @param   width - Rectangle width
 * @param   height - Rectangle height
 * @param   color - Fill color
 */
void EPD_FillRect(uint16 x, uint16 y, uint16 width, uint16 height, uint8 color);

/**
 * @brief   Get pointer to display buffer (for LVGL)
 * @return  Pointer to display buffer
 */
uint8* EPD_GetBuffer(void);

/**
 * @brief   Set rotation
 * @param   rotation - Rotation angle
 */
void EPD_SetRotation(epd_rotation_t rotation);

/**
 * @brief   Flush buffer to display (for LVGL integration)
 * @param   x1 - Start X coordinate
 * @param   y1 - Start Y coordinate
 * @param   x2 - End X coordinate
 * @param   y2 - End Y coordinate
 * @param   color_p - Pointer to color data
 */
void EPD_Flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint8_t *color_p);

#ifdef __cplusplus
}
#endif

#endif /* GDEY029T94_H */
