# Smart Food Label

A sophisticated food labeling system with e-paper display, temperature monitoring, and intelligent storage logging for modern refrigerator management.

## Overview

The Smart Food Label is an IoT device designed to help track food products in refrigerators, providing:
- Product type identification (Fish, Meat, Milk, Vegetables, etc.)
- Creation and expiration date tracking
- Days-until-expiration calculation
- Temperature monitoring (every 10 minutes)
- Storage logging (track when food is in/out of fridge)
- Editor initials for accountability
- E-paper display for ultra-low power consumption

## Hardware Requirements

### Main Components
- **MCU**: CH592 BLE RISC-V microcontroller
- **Display**: GDEY029T94 2.9" E-Paper Display (296x128, Black/White)
- **Sensor**: SHTxx Temperature/Humidity Sensor (SHT2x or SHT3x)
- **Buttons**: 3x Push buttons

### Pin Connections

#### SHTxx Sensor (I2C)
- PB12: SCL (I2C Clock)
- PB13: SDA (I2C Data)
- VCC: 3.3V
- GND: Ground

#### GDEY029T94 E-Paper Display (SPI)
- PA12: SCK (SPI Clock)
- PA13: MOSI (SPI Data Out)
- PA15: MISO (SPI Data In)
- PA14: CS (Chip Select)
- PA0: DC (Data/Command)
- PA1: RST (Reset)
- PA2: BUSY (Busy Signal)
- VCC: 3.3V
- GND: Ground

#### Buttons
- PB4: Button 1 (Edit/Log Toggle)
- PB5: Button 2 (Decrease Value)
- PB6: Button 3 (Increase Value)
- All buttons: Active Low with pull-up resistors

#### UART Debug (Optional)
- PA8: RX
- PA9: TX

## Features

### Display Mode
Shows all current food label information:
- Product type
- Creation date
- Expiration date
- Days until expiration
- Current temperature
- Logging status (IN FRIDGE / REMOVED)
- Editor initials

### Edit Mode
Allows editing of all fields:
- Product type: Fish, Meat, Milk, Vegetables, Fruits, Cheese, Eggs, Other
- Creation date: Day, Month, Year
- Expiration date: Day, Month, Year
- Editor initials: Two letters (A-Z)

### Button Functions

#### Button 1 (Edit/Log)
- **Long Press (5 seconds)**: Enter/Exit edit mode
  - Saves data to EEPROM when exiting edit mode
- **Short Press in Display Mode**: Toggle logging state
  - Start logging: Food placed in fridge
  - Stop logging: Food removed from fridge
- **Short Press in Edit Mode**: Move to next field

#### Button 2 (Down)
- **Short Press in Edit Mode**: Decrease current field value
  - Product type: Cycle backward
  - Dates: Decrement day/month/year
  - Initials: Previous letter (Z → A)

#### Button 3 (Up)
- **Short Press in Edit Mode**: Increase current field value
  - Product type: Cycle forward
  - Dates: Increment day/month/year
  - Initials: Next letter (A → Z)

## Usage Workflow

### Initial Setup
1. Power on the device
2. Press Button 1 for 5 seconds to enter edit mode
3. Configure:
   - Product type (use Button 2/3 to select)
   - Creation date (Button 1 to next field, Button 2/3 to adjust)
   - Expiration date
   - Your initials
4. Press Button 1 for 5 seconds to save and exit

### Daily Use
1. **Placing food in fridge**:
   - Press Button 1 briefly to start logging
   - Display shows "IN FRIDGE"
   - Temperature logging begins

2. **Removing food for use**:
   - Press Button 1 briefly to stop logging
   - Display shows "REMOVED"
   - Temperature logging pauses
   - No alarm will be triggered while removed

3. **Returning food to fridge**:
   - Press Button 1 briefly to resume logging
   - Display shows "IN FRIDGE"
   - Temperature logging resumes

### Monitoring
- Display automatically shows:
  - Days until expiration
  - Latest temperature measurement
  - Current logging status
- Temperature is measured every 10 minutes when logging is active
- E-paper display updates only when data changes (ultra-low power)

## Software Architecture

### Project Structure
```
SmartFoodLabel/
├── APP/
│   └── main.c                 # Main application logic
├── HAL/
│   ├── shtxx.c/h             # Temperature sensor driver
│   ├── gdey029t94.c/h        # E-paper display driver
│   ├── button.c/h            # Button handler with long press
│   └── data_storage.c/h      # EEPROM data storage
├── UI/
│   └── food_label_ui.c/h     # User interface & display logic
└── README.md
```

### Key Components

#### SHTxx Driver
- Supports SHT2x and SHT3x series sensors
- I2C communication with CRC checking
- Temperature accuracy: ±0.3°C
- Humidity accuracy: ±2% RH

#### E-Paper Display Driver
- 296x128 pixel monochrome display
- Full/partial refresh support
- Deep sleep mode for power saving
- Custom 5x7 font rendering
- Drawing primitives (lines, rectangles, text)

#### Button Handler
- Debouncing (50ms)
- Long press detection (5 seconds)
- Short press detection
- Event-based callback system

#### UI System
- Display mode: Show all information
- Edit mode: Field-by-field editing with visual feedback
- Automatic days-until-expiration calculation
- Temperature display with formatting

#### Data Storage
- EEPROM-based persistent storage
- Magic number validation
- Checksum verification
- Automatic save on edit mode exit

## Building and Flashing

### Prerequisites
- MounRiver Studio IDE
- WCH-Link programmer
- CH592 SDK

### Build Instructions
1. Open MounRiver Studio
2. Import the SmartFoodLabel project
3. Build the project (Ctrl+B)
4. Connect WCH-Link programmer
5. Flash to CH592 (F8)

### Debugging
- UART output at 115200 baud (PA9-TX, PA8-RX)
- Connect USB-TTL adapter to view debug messages
- Monitor temperature readings, button events, and system status

## Power Consumption

### Typical Operating Modes
- **Active (measuring + BLE)**: ~5-10mA
- **Display update**: ~15mA for ~3 seconds (e-paper refresh)
- **Deep sleep**: <10µA
- **Temperature logging**: Average <100µA (with 10-minute intervals)

### Battery Life Estimation
With CR2032 (220mAh):
- Continuous monitoring: ~1-2 months
- Deep sleep mode: >1 year

With 2x AA batteries (2000mAh):
- Continuous monitoring: ~6-12 months
- Optimized usage: >2 years

## Future Enhancements

### Planned Features
- [ ] BLE notifications for expiration warnings
- [ ] Mobile app for remote monitoring
- [ ] Temperature alarm thresholds
- [ ] Historical temperature data logging
- [ ] Multi-language support
- [ ] Barcode/QR code scanning
- [ ] NFC tag integration

### Possible Additions
- RTC module for accurate current date
- Buzzer for expiration alarms
- RGB LED status indicator
- Battery level monitoring
- OTA firmware updates via BLE

## Troubleshooting

### Display Not Working
- Check SPI connections (SCK, MOSI, CS, DC, RST, BUSY)
- Verify 3.3V power supply
- Ensure display is not in deep sleep (reset required)

### Temperature Sensor Not Responding
- Check I2C connections (SCL, SDA)
- Verify sensor I2C address (0x40 for SHT2x, 0x44 for SHT3x)
- Try soft reset command
- Check pull-up resistors on I2C lines

### Buttons Not Responding
- Verify button connections (active low)
- Check internal pull-ups are enabled
- Test with multimeter for continuity

### Data Not Saving
- EEPROM may be full or corrupted
- Try erasing storage (call Storage_Erase)
- Check power supply during write operations

## License

This project is part of the CH592 firmware examples.

## Authors

- Firmware implementation: Smart Food Label Team
- E-paper driver: Based on Good Display specifications
- SHTxx driver: Based on Sensirion datasheets

## Acknowledgments

- WCH for CH592 SDK and documentation
- Good Display for GDEY029T94 specifications
- Sensirion for SHTxx sensor documentation

---

**Version**: 1.0
**Date**: 2025
**Platform**: CH592 BLE RISC-V MCU
