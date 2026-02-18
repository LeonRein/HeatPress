#ifndef CONFIG_H
#define CONFIG_H

/*====================
   HARDWARE PINS
 *====================*/
#define PIN_HX711_DT   27
#define PIN_HX711_CLK  22

/* XPT2046 touch controller (separate VSPI bus) */
#define PIN_XPT2046_IRQ   36
#define PIN_XPT2046_MOSI  32
#define PIN_XPT2046_MISO  39
#define PIN_XPT2046_CLK   25
#define PIN_XPT2046_CS    33

/*====================
   DISPLAY
 *====================*/
#define SCREEN_WIDTH   320
#define SCREEN_HEIGHT  240

/*====================
   TOUCH CALIBRATION
 *====================*/
#define TOUCH_MIN_X    200      /* Raw XPT2046 X minimum */
#define TOUCH_MAX_X    3700     /* Raw XPT2046 X maximum */
#define TOUCH_MIN_Y    240      /* Raw XPT2046 Y minimum */
#define TOUCH_MAX_Y    3800     /* Raw XPT2046 Y maximum */

/*====================
   LOAD CELL
 *====================*/
#define LOADCELL_CAL_FACTOR     200.0f
#define LOADCELL_STABILIZE_MS   2000
#define LOADCELL_SAMPLES        1       /* HX711 smoothing (1 = no averaging) */
#define LOADCELL_TARE_TIMEOUT_MS 2000   /* Max wait for tare completion */
#define SENSOR_READ_INTERVAL_MS 100     /* 10 Hz sensor polling */

/*====================
   PRESS AREA (for bar calculation)
 *====================*/
#define PRESS_AREA_WIDTH_MM     300.0f  /* press plate width in mm */
#define PRESS_AREA_HEIGHT_MM    380.0f  /* press plate height in mm */

/*====================
   PRESSURE / TIMER
 *====================*/
#define PRESSURE_THRESHOLD      50.0f   /* grams to trigger "pressing" */
#define TIMER_DEFAULT_SECONDS   15      /* default countdown duration */
#define TIMER_MIN_SECONDS       5       /* minimum timer setting */
#define TIMER_MAX_SECONDS       300     /* maximum timer setting */
#define TIMER_STEP_SECONDS      5       /* +/- button increment */

/*====================
   ALERT
 *====================*/
#define ALERT_BLINK_INTERVAL_MS 500     /* Visual + audio blink period */
#define LOGIC_TICK_INTERVAL_MS  100     /* State machine update rate */

/*====================
   BUZZER / AUDIO
 *====================*/
#define PIN_AUDIO_OUT       26      /* DAC output on CYD (GPIO 26) */
#define BUZZER_LEDC_CHANNEL 7       /* LEDC channel (avoid 0–3 used by backlight/LVGL) */
#define BUZZER_FREQ_HZ      4000    /* Beep tone frequency */

/*====================
   TASK CONFIG
 *====================*/
#define UI_TASK_STACK_SIZE      8192
#define SENSOR_TASK_STACK_SIZE  4096
#define LOGIC_TASK_STACK_SIZE   4096

#define UI_TASK_PRIORITY        3
#define SENSOR_TASK_PRIORITY    2
#define LOGIC_TASK_PRIORITY     2

#define UI_TASK_CORE            1
#define SENSOR_TASK_CORE        0
#define LOGIC_TASK_CORE         0

#define QUEUE_SIZE              8

/*====================
   UI REFRESH
 *====================*/
#define UI_REFRESH_PERIOD_MS    33   /* ~30 fps */

/*====================
   MATERIAL COLORS (LVGL format: 0xRRGGBB)
 *====================*/
#define COLOR_BG           lv_color_hex(0x121212)
#define COLOR_SURFACE      lv_color_hex(0x1E1E1E)
#define COLOR_CARD         lv_color_hex(0x2C2C2C)
#define COLOR_PRIMARY      lv_color_hex(0xBB86FC)   /* Purple accent */
#define COLOR_SECONDARY    lv_color_hex(0x03DAC6)   /* Teal accent */
#define COLOR_ON_SURFACE   lv_color_hex(0xE0E0E0)
#define COLOR_ON_BG        lv_color_hex(0xFFFFFF)
#define COLOR_SUCCESS      lv_color_hex(0x4CAF50)   /* Green */
#define COLOR_WARNING      lv_color_hex(0xFFC107)   /* Amber */
#define COLOR_ERROR         lv_color_hex(0xFF1744)   /* Vivid red */
#define COLOR_ALERT_BG      lv_color_hex(0x8B0000)   /* Dark red background */
#define COLOR_DIMMED       lv_color_hex(0x757575)

#endif /* CONFIG_H */
