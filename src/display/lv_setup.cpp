#include "lv_setup.h"

#include <TFT_eSPI.h>
#include <lvgl.h>
#include "../config.h"

/* ── TFT + LVGL internals ────────────────────────────────── */

static TFT_eSPI tft = TFT_eSPI();

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
    tft.pushColors((uint16_t *)color_p, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(drv);
}

/* ── Touch read callback ─────────────────────────────────── */

static void touch_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    uint16_t tx, ty;
    bool pressed = tft.getTouch(&tx, &ty, 40);

    if (pressed) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = tx;
        data->point.y = ty;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/* ── Public API ───────────────────────────────────────────── */

void lv_setup_init()
{
    /* Initialize TFT */
    tft.init();
    tft.setRotation(1);  /* Landscape */
    tft.fillScreen(TFT_BLACK);

    /* Touch calibration for CYD (landscape).
     * These values may need adjustment for your specific display.
     * Format: calData[5] = { x_min, x_max, y_min, y_max, orientation } */
    uint16_t calData[5] = { 300, 3600, 300, 3600, 1 };
    tft.setTouch(calData);

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
