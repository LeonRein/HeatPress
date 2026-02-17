/*******************************************************************
    Hello World for the ESP32 Cheap Yellow Display.

    https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/

    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

// Make sure to copy the UserSetup.h file into the library as
// per the Github Instructions. The pins are defined in there.

// Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
// Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
// Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
// Note the following larger fonts are primarily numeric only!
// Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
// Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
// Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.

#include <TFT_eSPI.h>
// A library for interfacing with LCD displays
//
// Can be installed from the library manager (Search for "TFT_eSPI")
//https://github.com/Bodmer/TFT_eSPI

#include <HX711_ADC.h>

// HX711 pins
const int HX711_DT = 27;
const int HX711_CLK = 22;

// HX711 object
HX711_ADC LoadCell(HX711_DT, HX711_CLK);

bool stabilized = false;

TFT_eSPI tft = TFT_eSPI();

void setup() {
  // Start the tft display and set it to black
  tft.init();
  tft.setRotation(1); //This is the display in landscape
  
  // Clear the screen before writing to it
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  int x = 5;
  int y = 10;
  int fontNum = 2; 
  tft.drawString("Initializing...", x, y, fontNum); // Left Aligned

  // Initialize HX711
  LoadCell.begin();
  // Allow load cell to settle
  unsigned long stabilizingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilizing time
  LoadCell.start(stabilizingtime, true);
  if (LoadCell.getTareTimeoutFlag()) {
    tft.drawString("Timeout, check MCU>HX711 wiring and pin designations", 5, 30, 2);
    while (1);
  }
  else {
    LoadCell.setCalFactor(200.0); // User set calibration factor (float), initial value 1.0 may be used for this sketch
    // NOTE: You need to calibrate the load cell. Use a known weight and adjust the cal factor.
    tft.drawString("Startup is complete", 5, 30, 2);
    stabilized = true;
  }
  delay(2000);
  tft.fillScreen(TFT_BLACK);
}

void loop() {
  // Check if data is waiting asynchronously
  if (LoadCell.dataWaitingAsync()) {
    // Update the dataset with the new data
    if (LoadCell.updateAsync()) {
      if (stabilized) {
        float weight = LoadCell.getData();
        
        // Clear previous weight display
        tft.fillRect(0, 0, 320, 50, TFT_BLACK);
        
        // Display weight
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawCentreString("Weight: " + String(weight, 2) + " g", 160, 10, 4);
      }
    }
  }
  
  // Small delay to not overwhelm the display
  delay(10);
}