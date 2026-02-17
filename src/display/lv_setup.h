#ifndef LV_SETUP_H
#define LV_SETUP_H

#include <lvgl.h>

/**
 * Initialize LVGL, TFT display driver, and touch input driver.
 * Must be called from the UI task before any LVGL operations.
 */
void lv_setup_init();

/**
 * Call periodically from the UI task to process LVGL timers/rendering.
 * Should be called every ~33ms for 30fps.
 */
void lv_setup_update();

#endif /* LV_SETUP_H */
