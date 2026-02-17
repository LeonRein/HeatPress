#include "lv_setup.h"

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>
#include "../config.h"

/* ── TFT + Touch + LVGL internals ────────────────────────── */

static TFT_eSPI tft = TFT_eSPI();

/* XPT2046 on its own VSPI bus */
static SPIClass touchSpi = SPIClass(VSPI);
static XPT2046_Touchscreen ts(PIN_XPT2046_CS, PIN_XPT2046_IRQ);

/* Double-buffered: two 320×20 line buffers */
static const uint32_t BUF_PX = SCREEN_WIDTH * 20;
static lv_color_t buf1[BUF_PX];
static lv_color_t buf2[BUF_PX];

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t      disp_drv;
static lv_indev_drv_t     indev_drv;

/* ── Display flush callback ──────────────────────────────── */

static void tft_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)color_p, w * h, false);
    tft.endWrite();

    lv_disp_flush_ready(drv);
}

/* ── Touch read callback (XPT2046) ───────────────────────── */

static void touch_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    if (ts.tirqTouched() && ts.touched()) {
        TS_Point p = ts.getPoint();

        /* Map raw XPT2046 coordinates to screen pixels.
         * Raw range is roughly 200..3800 for both axes.
         * Landscape rotation 1: raw X → screen X, raw Y → screen Y */
        int16_t sx = map(p.x, 200, 3800, 0, SCREEN_WIDTH  - 1);
        int16_t sy = map(p.y, 200, 3800, 0, SCREEN_HEIGHT - 1);

        /* Clamp to screen bounds */
        if (sx < 0) sx = 0;
        if (sx >= SCREEN_WIDTH)  sx = SCREEN_WIDTH  - 1;
        if (sy < 0) sy = 0;
        if (sy >= SCREEN_HEIGHT) sy = SCREEN_HEIGHT - 1;

        data->state   = LV_INDEV_STATE_PRESSED;
        data->point.x = sx;
        data->point.y = sy;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/* ── Public API ───────────────────────────────────────────── */

void lv_setup_init()
{
    /* Initialize TFT display */
    tft.init();
    tft.setRotation(1);  /* Landscape */
    tft.fillScreen(TFT_BLACK);

    /* Initialize XPT2046 touch on VSPI */
    touchSpi.begin(PIN_XPT2046_CLK, PIN_XPT2046_MISO,
                   PIN_XPT2046_MOSI, PIN_XPT2046_CS);
    ts.begin(touchSpi);
    ts.setRotation(1);

    /* Initialize LVGL */
    lv_init();

    /* Set up draw buffers (double-buffered for smooth rendering) */
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, BUF_PX);

    /* Display driver */
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res  = SCREEN_WIDTH;
    disp_drv.ver_res  = SCREEN_HEIGHT;
    disp_drv.flush_cb = tft_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /* Input (touch) driver */
    lv_indev_drv_init(&indev_drv);
    indev_drv.type    = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touch_read_cb;
    lv_indev_drv_register(&indev_drv);
}

void lv_setup_update()
{
    lv_timer_handler();
}
