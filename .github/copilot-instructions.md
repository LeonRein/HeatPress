# HeatPress ESP32 Project - AI Agent Instructions

## Project Overview
This is a PlatformIO-based ESP32 project for a heat press pressure monitor with TFT display. The application reads pressure from an HX711 load cell amplifier and displays it on a Cheap Yellow Display (CYD). Key features include pressure detection, countdown timer, user-configurable timer duration via touch buttons, tare functionality, and visual alerts.

## Build & Development Workflow

### PlatformIO Commands
- **Build**: `platformio run --environment cyd` (default ILI9341 display)
- **Upload**: `platformio run --target upload --environment cyd`
- **Monitor**: `platformio device monitor` (115200 baud, esp32_exception_decoder filter)
- **Clean**: `platformio run --target clean`

### Environment-Specific Builds
- `cyd`: ILI9341 display driver, landscape orientation
- `cyd2usb`: ST7789 display driver, TFT_BGR color order

## Hardware Configuration

### TFT Display (TFT_eSPI)
```cpp
// Key pins (defined in build_flags)
TFT_MISO=12, TFT_MOSI=13, TFT_SCLK=14, TFT_CS=15, TFT_DC=2, TFT_RST=-1, TFT_BL=21
```
- Always call `tft.init()` and `tft.setRotation(1)` in setup()
- Use `tft.fillScreen(TFT_BLACK)` to clear display
- Resistive touch: `tft.getTouch(&x, &y)` for button presses
- Font 4 (`tft.drawCentreString(text, x, y, 4)`) for large centered text

### HX711 Load Cell
```cpp
const int HX711_DT = 27;
const int HX711_CLK = 22;
HX711_ADC LoadCell(HX711_DT, HX711_CLK);
```
- **Async Pattern** (non-blocking):
  ```cpp
  if (LoadCell.dataWaitingAsync()) {
    if (LoadCell.updateAsync()) {
      float weight = LoadCell.getData();
      // Use weight value
    }
  }
  ```
- Calibration: `LoadCell.setCalFactor(200.0)` (adjust with known weights)
- Tare: `LoadCell.tare()` after stabilization

## Code Patterns

### State Management
- Track pressure state: idle, pressing, timing, alert
- Timer logic: start on pressure threshold, countdown, reset on release
- Button handling: touch coordinates for +/-/tare buttons

### UI Design (Material Theme)
- Colors: `TFT_DARKGREY` backgrounds, `TFT_WHITE` text, `TFT_RED` alerts
- Buttons: Rounded rectangles with `tft.fillRoundRect()`, centered text
- Layout: Pressure display top-center, timer below, buttons bottom
- Clear areas: `tft.fillRect(x, y, w, h, TFT_BLACK)` before updates

### Timer Implementation
```cpp
unsigned long timerStart = 0;
int timerDuration = 30; // seconds, user adjustable
bool isTiming = false;
if (pressure > threshold && !isTiming) {
  timerStart = millis();
  isTiming = true;
}
if (isTiming && millis() - timerStart >= timerDuration * 1000) {
  // Alert: blink red
}
if (pressure < threshold) {
  isTiming = false;
}
```

### Touch Button Handling
```cpp
uint16_t x, y;
if (tft.getTouch(&x, &y)) {
  if (x >= btnX && x <= btnX+btnW && y >= btnY && y <= btnY+btnH) {
    // Button pressed
  }
}
```

### Initialization Sequence
```cpp
tft.init();
tft.setRotation(1);
tft.fillScreen(TFT_BLACK);

LoadCell.begin();
LoadCell.start(2000, true);
LoadCell.setCalFactor(200.0);
```

### Error Handling
- Check `LoadCell.getTareTimeoutFlag()` after start()
- Display error messages on TFT before `while(1)` halt

## Dependencies
- `bodmer/TFT_eSPI@^2.5.43` - Display and touch library
- `olkal/HX711_ADC@^1.2.12` - Load cell amplifier

## File Structure
- `src/main.cpp` - Single-file application with setup/loop
- `platformio.ini` - Build configuration with display-specific flags
- TFT configuration via build_flags (no separate UserSetup.h needed)