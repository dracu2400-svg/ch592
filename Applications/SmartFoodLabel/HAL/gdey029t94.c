/**
 * @file    gdey029t94.c
 * @brief   GDEY029T94 2.9" E-Paper Display Driver Implementation
 */

#include "gdey029t94.h"
#include "CH59x_common.h"

/*********************************************************************
 * LOCAL VARIABLES
 */

static uint8 epd_buffer[EPD_BUFFER_SIZE];
static epd_rotation_t current_rotation = EPD_ROTATE_0;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/**
 * @brief   Initialize SPI
 */
static void EPD_SPI_Init(void)
{
    // Configure SPI pins
    GPIOA_ModeCfg(EPD_SPI_SCK | EPD_SPI_MOSI | EPD_CS_PIN, GPIO_ModeOut_PP_5mA);
    GPIOA_ModeCfg(EPD_SPI_MISO, GPIO_ModeIN_PU);

    // Configure control pins
    GPIOA_ModeCfg(EPD_DC_PIN | EPD_RST_PIN, GPIO_ModeOut_PP_5mA);
    GPIOA_ModeCfg(EPD_BUSY_PIN, GPIO_ModeIN_PU);

    // Initialize CS high
    GPIOA_SetBits(EPD_CS_PIN);

    // Configure SPI0 module
    R8_SPI0_CTRL_MOD = RB_SPI_MOSI_OE | RB_SPI_SCK_OE;  // Enable MOSI and SCK
    R8_SPI0_CTRL_CFG = RB_SPI_AUTO_IF;                   // Auto clear interrupt flag
    R16_SPI0_CTRL_CFG |= 0x08;                           // Master mode
    R8_SPI0_CLOCK_DIV = 10;                              // SPI clock divider (6MHz @ 60MHz)
}

/**
 * @brief   SPI transfer byte
 */
static uint8 EPD_SPI_Transfer(uint8 data)
{
    R8_SPI0_BUFFER = data;
    while(!(R8_SPI0_INT_FLAG & RB_SPI_IF_BYTE_END));
    return R8_SPI0_BUFFER;
}

/**
 * @brief   GPIO control macros
 */
#define EPD_CS_LOW()    GPIOA_ResetBits(EPD_CS_PIN)
#define EPD_CS_HIGH()   GPIOA_SetBits(EPD_CS_PIN)
#define EPD_DC_LOW()    GPIOA_ResetBits(EPD_DC_PIN)
#define EPD_DC_HIGH()   GPIOA_SetBits(EPD_DC_PIN)
#define EPD_RST_LOW()   GPIOA_ResetBits(EPD_RST_PIN)
#define EPD_RST_HIGH()  GPIOA_SetBits(EPD_RST_PIN)
#define EPD_IS_BUSY()   (GPIOA_ReadPortPin(EPD_BUSY_PIN))

/**
 * @brief   Send command to display
 */
static void EPD_SendCommand(uint8 cmd)
{
    EPD_DC_LOW();
    EPD_CS_LOW();
    EPD_SPI_Transfer(cmd);
    EPD_CS_HIGH();
}

/**
 * @brief   Send data to display
 */
static void EPD_SendData(uint8 data)
{
    EPD_DC_HIGH();
    EPD_CS_LOW();
    EPD_SPI_Transfer(data);
    EPD_CS_HIGH();
}

/**
 * @brief   Wait until display is not busy
 */
static void EPD_WaitBusy(void)
{
    uint32 timeout = 0;

    while(EPD_IS_BUSY()) {
        DelayMs(10);
        timeout++;
        if(timeout > 400) {  // 4 second timeout
            break;
        }
    }
}

/**
 * @brief   Hardware reset
 */
static void EPD_Reset(void)
{
    EPD_RST_HIGH();
    DelayMs(20);
    EPD_RST_LOW();
    DelayMs(2);
    EPD_RST_HIGH();
    DelayMs(20);
}

/**
 * @brief   Set LUT (Look-Up Table) for full update
 */
static void EPD_SetLUTFull(void)
{
    // LUT for full update (provided by Good Display)
    static const uint8 lut_vcom[] = {
        0x00, 0x19, 0x01, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00,
    };

    static const uint8 lut_ww[] = {
        0x00, 0x19, 0x01, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    static const uint8 lut_bw[] = {
        0x80, 0x19, 0x01, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    static const uint8 lut_wb[] = {
        0x40, 0x19, 0x01, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    static const uint8 lut_bb[] = {
        0x00, 0x19, 0x01, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    uint8 i;

    EPD_SendCommand(EPD_CMD_VCOM_LUT);
    for(i = 0; i < 44; i++) {
        EPD_SendData(lut_vcom[i]);
    }

    EPD_SendCommand(EPD_CMD_W2W_LUT);
    for(i = 0; i < 42; i++) {
        EPD_SendData(lut_ww[i]);
    }

    EPD_SendCommand(EPD_CMD_B2W_LUT);
    for(i = 0; i < 42; i++) {
        EPD_SendData(lut_bw[i]);
    }

    EPD_SendCommand(EPD_CMD_W2B_LUT);
    for(i = 0; i < 42; i++) {
        EPD_SendData(lut_wb[i]);
    }

    EPD_SendCommand(EPD_CMD_B2B_LUT);
    for(i = 0; i < 42; i++) {
        EPD_SendData(lut_bb[i]);
    }
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief   Initialize E-Paper display
 */
uint8 EPD_Init(void)
{
    // Initialize SPI
    EPD_SPI_Init();

    // Hardware reset
    EPD_Reset();

    // Wait for display ready
    EPD_WaitBusy();

    // Soft reset
    EPD_SendCommand(0x12);
    DelayMs(10);
    EPD_WaitBusy();

    // Driver output control
    EPD_SendCommand(0x01);
    EPD_SendData(0x27);  // 296-1
    EPD_SendData(0x01);
    EPD_SendData(0x00);

    // Data entry mode
    EPD_SendCommand(0x11);
    EPD_SendData(0x03);  // X increment, Y increment

    // Set RAM X address
    EPD_SendCommand(0x44);
    EPD_SendData(0x00);
    EPD_SendData(0x0F);  // 0x0F = 15 (128/8-1)

    // Set RAM Y address
    EPD_SendCommand(0x45);
    EPD_SendData(0x00);
    EPD_SendData(0x00);
    EPD_SendData(0x27);  // 0x127 = 295
    EPD_SendData(0x01);

    // Border waveform
    EPD_SendCommand(0x3C);
    EPD_SendData(0x05);

    // Temperature sensor
    EPD_SendCommand(0x18);
    EPD_SendData(0x80);

    // Set RAM X address counter
    EPD_SendCommand(0x4E);
    EPD_SendData(0x00);

    // Set RAM Y address counter
    EPD_SendCommand(0x4F);
    EPD_SendData(0x00);
    EPD_SendData(0x00);

    EPD_WaitBusy();

    // Clear buffer
    EPD_Clear();

    return 0;
}

/**
 * @brief   Clear display buffer
 */
void EPD_Clear(void)
{
    uint16 i;
    for(i = 0; i < EPD_BUFFER_SIZE; i++) {
        epd_buffer[i] = EPD_COLOR_WHITE;
    }
}

/**
 * @brief   Display buffer content (fast update)
 */
void EPD_Display(void)
{
    uint16 i;

    // Set RAM X address counter
    EPD_SendCommand(0x4E);
    EPD_SendData(0x00);

    // Set RAM Y address counter
    EPD_SendCommand(0x4F);
    EPD_SendData(0x00);
    EPD_SendData(0x00);

    // Write RAM (black/white)
    EPD_SendCommand(0x24);
    for(i = 0; i < EPD_BUFFER_SIZE; i++) {
        EPD_SendData(epd_buffer[i]);
    }

    // Display update
    EPD_SendCommand(0x22);
    EPD_SendData(0xF7);
    EPD_SendCommand(0x20);

    EPD_WaitBusy();
}

/**
 * @brief   Display buffer with full update (better quality, slower)
 */
void EPD_DisplayFull(void)
{
    uint16 i;

    // Set LUT for full update
    EPD_SetLUTFull();

    // Set RAM X address counter
    EPD_SendCommand(0x4E);
    EPD_SendData(0x00);

    // Set RAM Y address counter
    EPD_SendCommand(0x4F);
    EPD_SendData(0x00);
    EPD_SendData(0x00);

    // Write RAM (black/white)
    EPD_SendCommand(0x24);
    for(i = 0; i < EPD_BUFFER_SIZE; i++) {
        EPD_SendData(epd_buffer[i]);
    }

    // Display update
    EPD_SendCommand(0x22);
    EPD_SendData(0xF7);
    EPD_SendCommand(0x20);

    EPD_WaitBusy();
}

/**
 * @brief   Enter deep sleep mode
 */
void EPD_Sleep(void)
{
    EPD_SendCommand(0x10);  // Deep sleep mode
    EPD_SendData(0x01);
    DelayMs(100);
}

/**
 * @brief   Wake up from deep sleep
 */
void EPD_WakeUp(void)
{
    EPD_Reset();
    EPD_Init();
}

/**
 * @brief   Set pixel at coordinates
 */
void EPD_SetPixel(uint16 x, uint16 y, uint8 color)
{
    uint16 index;
    uint8 bit_pos;

    if(x >= EPD_WIDTH || y >= EPD_HEIGHT) {
        return;
    }

    // Handle rotation
    uint16 px = x, py = y;
    switch(current_rotation) {
        case EPD_ROTATE_90:
            px = y;
            py = EPD_WIDTH - 1 - x;
            break;
        case EPD_ROTATE_180:
            px = EPD_WIDTH - 1 - x;
            py = EPD_HEIGHT - 1 - y;
            break;
        case EPD_ROTATE_270:
            px = EPD_HEIGHT - 1 - y;
            py = x;
            break;
        default:
            break;
    }

    // Calculate buffer position
    index = (py * EPD_WIDTH + px) / 8;
    bit_pos = 7 - (px % 8);

    if(color == EPD_COLOR_BLACK) {
        epd_buffer[index] &= ~(1 << bit_pos);
    } else {
        epd_buffer[index] |= (1 << bit_pos);
    }
}

/**
 * @brief   Draw horizontal line
 */
void EPD_DrawHLine(uint16 x, uint16 y, uint16 width, uint8 color)
{
    uint16 i;
    for(i = 0; i < width; i++) {
        EPD_SetPixel(x + i, y, color);
    }
}

/**
 * @brief   Draw vertical line
 */
void EPD_DrawVLine(uint16 x, uint16 y, uint16 height, uint8 color)
{
    uint16 i;
    for(i = 0; i < height; i++) {
        EPD_SetPixel(x, y + i, color);
    }
}

/**
 * @brief   Fill rectangle
 */
void EPD_FillRect(uint16 x, uint16 y, uint16 width, uint16 height, uint8 color)
{
    uint16 i, j;
    for(i = 0; i < height; i++) {
        for(j = 0; j < width; j++) {
            EPD_SetPixel(x + j, y + i, color);
        }
    }
}

/**
 * @brief   Get pointer to display buffer
 */
uint8* EPD_GetBuffer(void)
{
    return epd_buffer;
}

/**
 * @brief   Set rotation
 */
void EPD_SetRotation(epd_rotation_t rotation)
{
    current_rotation = rotation;
}

/**
 * @brief   Flush buffer to display (for LVGL)
 */
void EPD_Flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint8_t *color_p)
{
    // Copy color data to buffer
    int32_t x, y;
    uint32_t index = 0;

    for(y = y1; y <= y2; y++) {
        for(x = x1; x <= x2; x++) {
            EPD_SetPixel(x, y, color_p[index]);
            index++;
        }
    }

    // Update display
    EPD_Display();
}
