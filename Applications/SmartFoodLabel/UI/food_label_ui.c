/**
 * @file    food_label_ui.c
 * @brief   Food Label UI Implementation
 */

#include "food_label_ui.h"
#include "gdey029t94.h"
#include "CH59x_common.h"
#include <stdio.h>
#include <string.h>

/*********************************************************************
 * LOCAL VARIABLES
 */

static food_label_data_t current_data = {
    .product_type = PRODUCT_FISH,
    .create_date = {1, 1, 2025},
    .expire_date = {1, 1, 2025},
    .initials = "AA",
    .temperature = 4.0f,
    .is_logging = 1
};

static ui_mode_t current_mode = UI_MODE_DISPLAY;
static uint8 current_field = 0;

// Product names
static const char* product_names[PRODUCT_COUNT] = {
    "Fish",
    "Meat",
    "Milk",
    "Vegetables",
    "Fruits",
    "Cheese",
    "Eggs",
    "Other"
};

// Simple 5x7 font (ASCII 32-127)
// For production, use a proper font library
extern const uint8 font_5x7[][5];  // Defined in font file

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief   Draw character at position
 */
static void UI_DrawChar(uint16 x, uint16 y, char c, uint8 size)
{
    uint8 i, j;
    uint8 line;

    if(c < ' ' || c > '~') {
        c = ' ';  // Replace invalid chars with space
    }

    for(i = 0; i < 5; i++) {
        line = font_5x7[c - ' '][i];
        for(j = 0; j < 7; j++) {
            if(line & (1 << j)) {
                if(size == 1) {
                    EPD_SetPixel(x + i, y + j, EPD_COLOR_BLACK);
                } else {
                    EPD_FillRect(x + i * size, y + j * size, size, size, EPD_COLOR_BLACK);
                }
            }
        }
    }
}

/**
 * @brief   Draw string at position
 */
static void UI_DrawString(uint16 x, uint16 y, const char *str, uint8 size)
{
    uint16 pos_x = x;
    uint16 char_width = 6 * size;  // 5 pixels + 1 pixel spacing

    while(*str) {
        UI_DrawChar(pos_x, y, *str, size);
        pos_x += char_width;
        str++;
    }
}

/**
 * @brief   Draw integer at position
 */
static void UI_DrawInt(uint16 x, uint16 y, int32 value, uint8 size)
{
    char buffer[12];
    sprintf(buffer, "%d", (int)value);
    UI_DrawString(x, y, buffer, size);
}

/**
 * @brief   Draw float at position
 */
static void UI_DrawFloat(uint16 x, uint16 y, float value, uint8 decimals, uint8 size)
{
    char buffer[16];

    if(decimals == 1) {
        sprintf(buffer, "%.1f", value);
    } else {
        sprintf(buffer, "%.2f", value);
    }

    UI_DrawString(x, y, buffer, size);
}

/**
 * @brief   Draw display mode screen
 */
static void UI_DrawDisplayMode(food_label_data_t *data)
{
    char buffer[32];
    int16 days_left;

    // Clear display
    EPD_Clear();

    // Draw title
    UI_DrawString(5, 5, "FOOD LABEL", 2);

    // Draw separator line
    EPD_DrawHLine(0, 20, EPD_WIDTH, EPD_COLOR_BLACK);

    // Draw product type
    UI_DrawString(5, 25, "Product:", 1);
    UI_DrawString(50, 25, UI_GetProductName(data->product_type), 2);

    // Draw creation date
    UI_DrawString(5, 45, "Created:", 1);
    sprintf(buffer, "%02d/%02d/%04d", data->create_date.day,
            data->create_date.month, data->create_date.year);
    UI_DrawString(50, 45, buffer, 1);

    // Draw expiration date
    UI_DrawString(5, 60, "Expires:", 1);
    sprintf(buffer, "%02d/%02d/%04d", data->expire_date.day,
            data->expire_date.month, data->expire_date.year);
    UI_DrawString(50, 60, buffer, 1);

    // Calculate and draw days until expiration
    days_left = UI_CalculateDaysUntilExpire(data);
    UI_DrawString(5, 75, "Days Left:", 1);

    if(days_left >= 0) {
        UI_DrawInt(65, 75, days_left, 2);
        UI_DrawString(90, 75, "days", 1);
    } else {
        UI_DrawString(65, 75, "EXPIRED", 2);
    }

    // Draw separator line
    EPD_DrawHLine(0, 95, EPD_WIDTH, EPD_COLOR_BLACK);

    // Draw temperature
    UI_DrawString(5, 100, "Temp:", 1);
    UI_DrawFloat(35, 100, data->temperature, 1, 2);
    UI_DrawString(70, 100, "C", 2);

    // Draw logging status
    UI_DrawString(5, 115, data->is_logging ? "IN FRIDGE" : "REMOVED", 1);

    // Draw editor initials
    UI_DrawString(80, 115, "By:", 1);
    UI_DrawString(100, 115, data->initials, 1);

    // Update display
    EPD_DisplayFull();
}

/**
 * @brief   Draw edit mode screen
 */
static void UI_DrawEditMode(food_label_data_t *data)
{
    char buffer[32];

    // Clear display
    EPD_Clear();

    // Draw title
    UI_DrawString(5, 5, "EDIT MODE", 2);
    EPD_DrawHLine(0, 20, EPD_WIDTH, EPD_COLOR_BLACK);

    // Draw all fields
    uint8 y_pos = 25;
    uint8 line_height = 12;

    // Product
    UI_DrawString(5, y_pos, "Product:", 1);
    UI_DrawString(55, y_pos, UI_GetProductName(data->product_type), 1);
    if(current_field == FIELD_PRODUCT) {
        EPD_DrawHLine(55, y_pos + 8, 60, EPD_COLOR_BLACK);  // Underline
    }
    y_pos += line_height;

    // Create date
    UI_DrawString(5, y_pos, "Create:", 1);
    sprintf(buffer, "%02d/%02d/%04d", data->create_date.day,
            data->create_date.month, data->create_date.year);
    UI_DrawString(55, y_pos, buffer, 1);

    // Underline current field
    if(current_field == FIELD_CREATE_DAY) {
        EPD_DrawHLine(55, y_pos + 8, 12, EPD_COLOR_BLACK);
    } else if(current_field == FIELD_CREATE_MONTH) {
        EPD_DrawHLine(70, y_pos + 8, 12, EPD_COLOR_BLACK);
    } else if(current_field == FIELD_CREATE_YEAR) {
        EPD_DrawHLine(85, y_pos + 8, 24, EPD_COLOR_BLACK);
    }
    y_pos += line_height;

    // Expire date
    UI_DrawString(5, y_pos, "Expire:", 1);
    sprintf(buffer, "%02d/%02d/%04d", data->expire_date.day,
            data->expire_date.month, data->expire_date.year);
    UI_DrawString(55, y_pos, buffer, 1);

    // Underline current field
    if(current_field == FIELD_EXPIRE_DAY) {
        EPD_DrawHLine(55, y_pos + 8, 12, EPD_COLOR_BLACK);
    } else if(current_field == FIELD_EXPIRE_MONTH) {
        EPD_DrawHLine(70, y_pos + 8, 12, EPD_COLOR_BLACK);
    } else if(current_field == FIELD_EXPIRE_YEAR) {
        EPD_DrawHLine(85, y_pos + 8, 24, EPD_COLOR_BLACK);
    }
    y_pos += line_height;

    // Initials
    UI_DrawString(5, y_pos, "Initials:", 1);
    UI_DrawChar(55, y_pos, data->initials[0], 1);
    UI_DrawChar(62, y_pos, data->initials[1], 1);

    if(current_field == FIELD_INITIAL_1) {
        EPD_DrawHLine(55, y_pos + 8, 6, EPD_COLOR_BLACK);
    } else if(current_field == FIELD_INITIAL_2) {
        EPD_DrawHLine(62, y_pos + 8, 6, EPD_COLOR_BLACK);
    }
    y_pos += line_height + 5;

    // Instructions
    EPD_DrawHLine(0, y_pos, EPD_WIDTH, EPD_COLOR_BLACK);
    y_pos += 3;
    UI_DrawString(5, y_pos, "B1:Next B2:Down B3:Up", 1);
    y_pos += 10;
    UI_DrawString(5, y_pos, "Hold B1 5s to save", 1);

    // Update display
    EPD_Display();
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief   Initialize UI system
 */
void UI_Init(void)
{
    // Initialize e-paper display
    EPD_Init();

    // Set default orientation
    EPD_SetRotation(EPD_ROTATE_0);

    // Clear display
    EPD_Clear();
    EPD_Display();
}

/**
 * @brief   Set UI mode
 */
void UI_SetMode(ui_mode_t mode)
{
    current_mode = mode;

    if(mode == UI_MODE_EDIT) {
        current_field = 0;  // Start with first field
    }
}

/**
 * @brief   Get current UI mode
 */
ui_mode_t UI_GetMode(void)
{
    return current_mode;
}

/**
 * @brief   Update display
 */
void UI_UpdateDisplay(food_label_data_t *data)
{
    if(current_mode == UI_MODE_DISPLAY) {
        UI_DrawDisplayMode(data);
    } else {
        UI_DrawEditMode(data);
    }
}

/**
 * @brief   Move to next field
 */
void UI_NextField(void)
{
    if(current_mode == UI_MODE_EDIT) {
        current_field++;
        if(current_field >= FIELD_COUNT) {
            current_field = 0;
        }
        UI_UpdateDisplay(&current_data);
    }
}

/**
 * @brief   Increment current field value
 */
void UI_IncrementField(void)
{
    if(current_mode != UI_MODE_EDIT) return;

    switch(current_field) {
        case FIELD_PRODUCT:
            current_data.product_type++;
            if(current_data.product_type >= PRODUCT_COUNT) {
                current_data.product_type = 0;
            }
            break;

        case FIELD_CREATE_DAY:
            current_data.create_date.day++;
            if(current_data.create_date.day > 31) {
                current_data.create_date.day = 1;
            }
            break;

        case FIELD_CREATE_MONTH:
            current_data.create_date.month++;
            if(current_data.create_date.month > 12) {
                current_data.create_date.month = 1;
            }
            break;

        case FIELD_CREATE_YEAR:
            current_data.create_date.year++;
            if(current_data.create_date.year > 2099) {
                current_data.create_date.year = 2020;
            }
            break;

        case FIELD_EXPIRE_DAY:
            current_data.expire_date.day++;
            if(current_data.expire_date.day > 31) {
                current_data.expire_date.day = 1;
            }
            break;

        case FIELD_EXPIRE_MONTH:
            current_data.expire_date.month++;
            if(current_data.expire_date.month > 12) {
                current_data.expire_date.month = 1;
            }
            break;

        case FIELD_EXPIRE_YEAR:
            current_data.expire_date.year++;
            if(current_data.expire_date.year > 2099) {
                current_data.expire_date.year = 2020;
            }
            break;

        case FIELD_INITIAL_1:
            current_data.initials[0]++;
            if(current_data.initials[0] > 'Z') {
                current_data.initials[0] = 'A';
            }
            break;

        case FIELD_INITIAL_2:
            current_data.initials[1]++;
            if(current_data.initials[1] > 'Z') {
                current_data.initials[1] = 'A';
            }
            break;
    }

    UI_UpdateDisplay(&current_data);
}

/**
 * @brief   Decrement current field value
 */
void UI_DecrementField(void)
{
    if(current_mode != UI_MODE_EDIT) return;

    switch(current_field) {
        case FIELD_PRODUCT:
            if(current_data.product_type == 0) {
                current_data.product_type = PRODUCT_COUNT - 1;
            } else {
                current_data.product_type--;
            }
            break;

        case FIELD_CREATE_DAY:
            if(current_data.create_date.day == 1) {
                current_data.create_date.day = 31;
            } else {
                current_data.create_date.day--;
            }
            break;

        case FIELD_CREATE_MONTH:
            if(current_data.create_date.month == 1) {
                current_data.create_date.month = 12;
            } else {
                current_data.create_date.month--;
            }
            break;

        case FIELD_CREATE_YEAR:
            if(current_data.create_date.year == 2020) {
                current_data.create_date.year = 2099;
            } else {
                current_data.create_date.year--;
            }
            break;

        case FIELD_EXPIRE_DAY:
            if(current_data.expire_date.day == 1) {
                current_data.expire_date.day = 31;
            } else {
                current_data.expire_date.day--;
            }
            break;

        case FIELD_EXPIRE_MONTH:
            if(current_data.expire_date.month == 1) {
                current_data.expire_date.month = 12;
            } else {
                current_data.expire_date.month--;
            }
            break;

        case FIELD_EXPIRE_YEAR:
            if(current_data.expire_date.year == 2020) {
                current_data.expire_date.year = 2099;
            } else {
                current_data.expire_date.year--;
            }
            break;

        case FIELD_INITIAL_1:
            if(current_data.initials[0] == 'A') {
                current_data.initials[0] = 'Z';
            } else {
                current_data.initials[0]--;
            }
            break;

        case FIELD_INITIAL_2:
            if(current_data.initials[1] == 'A') {
                current_data.initials[1] = 'Z';
            } else {
                current_data.initials[1]--;
            }
            break;
    }

    UI_UpdateDisplay(&current_data);
}

/**
 * @brief   Get pointer to current data
 */
food_label_data_t* UI_GetData(void)
{
    return &current_data;
}

/**
 * @brief   Get product name string
 */
const char* UI_GetProductName(uint8 product_type)
{
    if(product_type >= PRODUCT_COUNT) {
        return "Unknown";
    }
    return product_names[product_type];
}

/**
 * @brief   Calculate days until expiration
 */
int16 UI_CalculateDaysUntilExpire(food_label_data_t *data)
{
    // Simplified calculation (assumes current date is create_date + elapsed time)
    // For production, use RTC to get actual current date

    int32 expire_days = data->expire_date.year * 365 +
                        data->expire_date.month * 30 +
                        data->expire_date.day;

    int32 create_days = data->create_date.year * 365 +
                        data->create_date.month * 30 +
                        data->create_date.day;

    // Assume current date = create date for simplicity
    // In production, get from RTC
    int32 current_days = create_days;

    return (int16)(expire_days - current_days);
}

/*********************************************************************
 * SIMPLE FONT DATA
 * 5x7 font for ASCII characters 32-127
 */

const uint8 font_5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // Backslash
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // f
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z
    {0x00, 0x08, 0x36, 0x41, 0x00}, // {
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // |
    {0x00, 0x41, 0x36, 0x08, 0x00}, // }
    {0x08, 0x04, 0x08, 0x10, 0x08}, // ~
};
