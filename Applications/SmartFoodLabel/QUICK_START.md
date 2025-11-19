# Smart Food Label - Quick Start Guide

## Hardware Setup

### Required Components
1. CH592 development board
2. GDEY029T94 2.9" E-Paper display
3. SHT2x or SHT3x temperature/humidity sensor
4. 3x push buttons
5. Breadboard and jumper wires
6. 3.3V power supply

### Wiring Diagram

```
CH592          GDEY029T94
============================
PA12 --------- SCK
PA13 --------- MOSI
PA15 --------- MISO
PA14 --------- CS
PA0  --------- DC
PA1  --------- RST
PA2  --------- BUSY
3.3V --------- VCC
GND  --------- GND

CH592          SHTxx
============================
PB12 --------- SCL
PB13 --------- SDA
3.3V --------- VDD
GND  --------- GND

CH592          Buttons
============================
PB4  --------- Button 1 (+ pull-up to 3.3V)
PB5  --------- Button 2 (+ pull-up to 3.3V)
PB6  --------- Button 3 (+ pull-up to 3.3V)
GND  --------- Button Common
```

### Pin Summary Table

| Function | CH592 Pin | Component Pin |
|----------|-----------|---------------|
| SPI SCK  | PA12      | EPD SCK       |
| SPI MOSI | PA13      | EPD MOSI      |
| SPI MISO | PA15      | EPD MISO      |
| EPD CS   | PA14      | EPD CS        |
| EPD DC   | PA0       | EPD DC        |
| EPD RST  | PA1       | EPD RST       |
| EPD BUSY | PA2       | EPD BUSY      |
| I2C SCL  | PB12      | SHT SCL       |
| I2C SDA  | PB13      | SHT SDA       |
| Button 1 | PB4       | Edit/Log BTN  |
| Button 2 | PB5       | Down BTN      |
| Button 3 | PB6       | Up BTN        |
| UART TX  | PA9       | Debug TX      |
| UART RX  | PA8       | Debug RX      |

## Software Setup

### 1. Install Development Tools
- Download and install MounRiver Studio from [WCH website](http://www.wch.cn)
- Install WCH-Link USB drivers

### 2. Import Project
1. Open MounRiver Studio
2. File → Import → Existing Projects into Workspace
3. Select `SmartFoodLabel` directory
4. Click Finish

### 3. Configure Build
1. Right-click project → Properties
2. C/C++ Build → Settings
3. Verify toolchain: RISC-V Cross GCC
4. Check optimization level: -O2 or -Os

### 4. Build Project
1. Select project in Project Explorer
2. Press Ctrl+B or click Build icon
3. Verify build succeeds without errors

### 5. Flash to Hardware
1. Connect WCH-Link programmer to CH592
2. Connect WCH-Link to PC via USB
3. Press F8 or click Debug icon
4. Firmware will be flashed automatically

## First Boot

### Initial Configuration
1. Power on the device
2. E-paper display will show default values
3. Press and hold Button 1 for 5 seconds
4. Display enters EDIT MODE

### Configure Your First Label
1. **Select Product Type**:
   - Current field shows selected product
   - Press Button 3 (Up) or Button 2 (Down) to cycle through:
     - Fish
     - Meat
     - Milk
     - Vegetables
     - Fruits
     - Cheese
     - Eggs
     - Other

2. **Set Creation Date**:
   - Press Button 1 to move to day field
   - Use Button 2/3 to set day (1-31)
   - Press Button 1 to move to month field
   - Use Button 2/3 to set month (1-12)
   - Press Button 1 to move to year field
   - Use Button 2/3 to set year (2020-2099)

3. **Set Expiration Date**:
   - Press Button 1 to move to day field
   - Use Button 2/3 to set day
   - Continue for month and year

4. **Set Your Initials**:
   - Press Button 1 to move to first initial
   - Use Button 2/3 to select letter (A-Z)
   - Press Button 1 to move to second initial
   - Use Button 2/3 to select letter

5. **Save Configuration**:
   - Press and hold Button 1 for 5 seconds
   - Display exits edit mode and saves to EEPROM
   - Configuration is now persistent

## Daily Usage

### Scenario 1: Storing New Food
1. Configure label as described above
2. In display mode, press Button 1 briefly
3. Display shows "IN FRIDGE" - logging starts
4. Temperature measurements begin (every 10 minutes)

### Scenario 2: Removing Food for Use
1. Before removing food, press Button 1 briefly
2. Display shows "REMOVED" - logging pauses
3. No temperature alarms while removed
4. Use food as needed

### Scenario 3: Returning Food to Fridge
1. After using, press Button 1 briefly
2. Display shows "IN FRIDGE" - logging resumes
3. Temperature monitoring continues

### Scenario 4: Checking Food Status
- Simply view the e-paper display:
  - Product name
  - Days until expiration
  - Current temperature
  - Logging status
  - Editor initials

## Understanding the Display

### Display Mode Layout
```
┌─────────────────────────────┐
│ FOOD LABEL                  │
├─────────────────────────────┤
│ Product:  FISH              │
│                             │
│ Created:  19/11/2025        │
│ Expires:  26/11/2025        │
│ Days Left:  7 days          │
├─────────────────────────────┤
│ Temp:  4.2 C                │
│ IN FRIDGE          By: AB   │
└─────────────────────────────┘
```

### Edit Mode Layout
```
┌─────────────────────────────┐
│ EDIT MODE                   │
├─────────────────────────────┤
│ Product: Fish               │
│          ─────              │ ← Underline shows active field
│ Create:  19/11/2025         │
│ Expire:  26/11/2025         │
│ Initials: AB                │
├─────────────────────────────┤
│ B1:Next B2:Down B3:Up       │
│ Hold B1 5s to save          │
└─────────────────────────────┘
```

## Button Operation Summary

| Mode | Button 1 | Button 2 | Button 3 |
|------|----------|----------|----------|
| **Display** | Short: Toggle logging<br>Long: Enter edit | - | - |
| **Edit** | Short: Next field<br>Long: Save & exit | Decrease value | Increase value |

## Debugging

### Enable UART Debug Output
1. Connect USB-TTL adapter:
   - TX → PA9 (CH592 RX)
   - RX → PA8 (CH592 TX)
   - GND → GND

2. Open serial terminal:
   - Baud rate: 115200
   - Data bits: 8
   - Stop bits: 1
   - Parity: None

3. Reset CH592 to view boot messages

### Debug Output Examples
```
========================================
Smart Food Label System
Version: 1.0
========================================

Initializing storage...
Initializing buttons...
Initializing display...
Initializing SHT sensor...
SHT sensor initialized successfully
Loaded saved data from storage

System initialized successfully
Press Button 1 for 5 seconds to enter edit mode
Press Button 1 briefly to toggle logging

Temperature: 4.2 C, Humidity: 65.3 %
Logging STARTED
```

## Common Issues

### Issue: Display Shows Nothing
**Solution**:
- Check all SPI connections
- Verify 3.3V power is stable
- Try power cycle (reset)
- Check BUSY pin connection

### Issue: Buttons Don't Work
**Solution**:
- Verify button connections (active low)
- Check pull-up resistors (internal or external)
- Test continuity with multimeter
- Review button polling in debug output

### Issue: Temperature Always 0°C
**Solution**:
- Check I2C connections (SCL, SDA)
- Verify sensor address (0x40 or 0x44)
- Enable UART debug to see error messages
- Try different SHTxx sensor variant

### Issue: Data Not Saved After Edit
**Solution**:
- Hold Button 1 for full 5 seconds when exiting
- Check UART debug for save confirmation
- Verify EEPROM is not write-protected
- Power should be stable during save

### Issue: Display Update is Slow
**Expected behavior**:
- E-paper displays inherently refresh slowly (2-4 seconds)
- This is normal and reduces power consumption
- Full updates are slower than partial updates

## Tips & Best Practices

1. **Battery Life**:
   - Use logging mode only when needed
   - Longer measurement intervals save power
   - E-paper display uses no power when static

2. **Accuracy**:
   - Keep sensor away from heat sources
   - Allow sensor to stabilize after power-on
   - Calibrate temperature if needed

3. **Reliability**:
   - Always exit edit mode properly (save data)
   - Avoid interrupting power during EEPROM write
   - Regular temperature sensor cleaning

4. **Customization**:
   - Adjust measurement interval in `main.c`
   - Modify product types in `food_label_ui.h`
   - Change button pins in `button.h`

## Next Steps

- Read the full [README.md](README.md) for detailed information
- Review the [Firmware Integration Guide](../../FIRMWARE_INTEGRATION_GUIDE.md)
- Experiment with custom product types
- Add BLE connectivity for remote monitoring
- Implement RTC for accurate date/time

## Support

For issues or questions:
1. Check debug output via UART
2. Review the troubleshooting section
3. Consult CH592 datasheet and SDK documentation
4. Refer to component datasheets (GDEY029T94, SHTxx)

---

**Happy Food Tracking!** 🥘🥩🥛🧀
