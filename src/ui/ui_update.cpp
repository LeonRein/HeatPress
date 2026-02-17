#include "ui_update.h"
#include "ui_screen.h"
#include "ui_theme.h"
#include "../config.h"

#include <Arduino.h>
#include <lvgl.h>
#include <stdio.h>

/* Track current state for visual updates */
static AppState currentState = AppState::IDLE;
static bool alertBlinkOn = false;
static unsigned long lastBlinkMs = 0;

/* Arc animation state (local to UI task for smooth updates) */
static unsigned long arcStartMs  = 0;
static unsigned long arcDurationMs = 0;
static bool arcAnimating = false;

/* Pressure display mode */
static bool showBar = false;
static float lastPressureGrams = 0.0f;  /* cached for unit toggle */

/* Format pressure value into buf based on current unit mode */
static void format_pressure(float grams, char *buf, size_t len)
{
    if (showBar) {
        float forceN = (grams / 1000.0f) * 9.80665f;
        float areaM2 = (PRESS_AREA_WIDTH_MM * PRESS_AREA_HEIGHT_MM) * 1e-6f;
        float mbar = forceN / (areaM2 * 1e5f) * 1000.0f;
        snprintf(buf, len, "%.1f", mbar);
    } else {
        snprintf(buf, len, "%.2f", grams / 1000.0f);
    }
}

/* ── State color helpers ─────────────────────────────────── */

static void set_idle_colors()
{
    lv_obj_set_style_bg_color(ui_get_pressure_card(), COLOR_CARD, 0);
    lv_obj_set_style_arc_color(ui_get_timer_arc(), COLOR_PRIMARY, LV_PART_INDICATOR);

    lv_obj_t *status = ui_get_status_label();
    lv_label_set_text(status, "IDLE");
    lv_obj_set_style_text_color(status, COLOR_DIMMED, 0);
    lv_obj_set_style_text_font(status, &lv_font_montserrat_14, 0);

    /* Reset screen background */
    lv_obj_set_style_bg_color(lv_scr_act(), COLOR_BG, 0);
}

static void set_pressing_colors()
{
    lv_obj_set_style_bg_color(ui_get_pressure_card(), lv_color_hex(0x1B3A1B), 0);
    lv_obj_set_style_arc_color(ui_get_timer_arc(), COLOR_SUCCESS, LV_PART_INDICATOR);

    lv_obj_t *status = ui_get_status_label();
    lv_label_set_text(status, "PRESSING");
    lv_obj_set_style_text_color(status, COLOR_SUCCESS, 0);
}

static void set_timing_colors()
{
    lv_obj_set_style_bg_color(ui_get_pressure_card(), lv_color_hex(0x1B3A1B), 0);
    lv_obj_set_style_arc_color(ui_get_timer_arc(), COLOR_SUCCESS, LV_PART_INDICATOR);

    lv_obj_t *status = ui_get_status_label();
    lv_label_set_text(status, LV_SYMBOL_PLAY " TIMING");
    lv_obj_set_style_text_color(status, COLOR_SUCCESS, 0);
}

static void set_alert_colors()
{
    lv_obj_t *status = ui_get_status_label();
    lv_label_set_text(status, LV_SYMBOL_WARNING " DONE!");
    lv_obj_set_style_text_color(status, COLOR_ERROR, 0);
    lv_obj_set_style_text_font(status, &lv_font_montserrat_20, 0);
    lv_obj_set_style_arc_color(ui_get_timer_arc(), COLOR_ERROR, LV_PART_INDICATOR);

    /* Immediately set alert background */
    lv_obj_set_style_bg_color(lv_scr_act(), COLOR_ALERT_BG, 0);
    lv_obj_set_style_bg_color(ui_get_pressure_card(), COLOR_ERROR, 0);
}

/* ── Alert blink effect ──────────────────────────────────── */

static void alert_blink_tick()
{
    if (currentState != AppState::ALERT) return;

    unsigned long now = millis();
    if (now - lastBlinkMs > 500) {
        alertBlinkOn = !alertBlinkOn;
        lastBlinkMs = now;

        if (alertBlinkOn) {
            lv_obj_set_style_bg_color(ui_get_pressure_card(), COLOR_ERROR, 0);
            lv_obj_set_style_bg_color(lv_scr_act(), COLOR_ALERT_BG, 0);
        } else {
            lv_obj_set_style_bg_color(ui_get_pressure_card(), lv_color_hex(0x4A0000), 0);
            lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x2A0000), 0);
        }
    }
}

/* ── Public API ───────────────────────────────────────────── */

void ui_handle_command(const UICommand &cmd)
{
    switch (cmd.type) {
        case UICommandType::UPDATE_PRESSURE: {
            lastPressureGrams = cmd.pressure;
            char buf[16];
            format_pressure(cmd.pressure, buf, sizeof(buf));
            lv_label_set_text(ui_get_pressure_label(), buf);
            break;
        }

        case UICommandType::UPDATE_TIMER: {
            int secs = cmd.timerSeconds;
            char buf[12];
            if (secs < 0) {
                /* Overtime: show as +Xs */
                int over = -secs;
                int mins = over / 60;
                int remainder = over % 60;
                if (mins > 0) {
                    snprintf(buf, sizeof(buf), "+%d:%02d", mins, remainder);
                } else {
                    snprintf(buf, sizeof(buf), "+%d", over);
                }
            } else {
                int mins = secs / 60;
                int remainder = secs % 60;
                if (mins > 0) {
                    snprintf(buf, sizeof(buf), "%d:%02d", mins, remainder);
                } else {
                    snprintf(buf, sizeof(buf), "%d", secs);
                }
            }
            lv_label_set_text(ui_get_timer_label(), buf);
            break;
        }

        case UICommandType::UPDATE_STATE: {
            currentState = cmd.state;
            switch (cmd.state) {
                case AppState::IDLE:
                    set_idle_colors();
                    arcAnimating = false;
                    lv_arc_set_value(ui_get_timer_arc(), 0);
                    break;
                case AppState::PRESSING: set_pressing_colors(); break;
                case AppState::TIMING:   set_timing_colors();   break;
                case AppState::ALERT:
                    set_alert_colors();
                    arcAnimating = false;
                    lv_arc_set_value(ui_get_timer_arc(), 100);
                    break;
            }
            break;
        }

        case UICommandType::START_ARC_ANIM: {
            arcStartMs    = millis();
            arcDurationMs = (unsigned long)cmd.durationMs;
            arcAnimating  = true;
            lv_arc_set_value(ui_get_timer_arc(), 0);
            break;
        }

        case UICommandType::UPDATE_TIMER_SETTING: {
            ui_update_timer_setting(cmd.timerSeconds);
            break;
        }
    }
}

void ui_update_timer_setting(int durationSeconds)
{
    char buf[24];
    snprintf(buf, sizeof(buf), "Timer: %ds", durationSeconds);
    lv_label_set_text(ui_get_timer_setting_label(), buf);
}

void ui_arc_tick()
{
    /* Always run blink effect (not just on queue messages) */
    alert_blink_tick();

    if (!arcAnimating && currentState != AppState::ALERT) return;

    unsigned long elapsed = millis() - arcStartMs;

    /* Update arc (only during TIMING) */
    if (arcAnimating) {
        int percent;
        if (arcDurationMs == 0) {
            percent = 100;
        } else {
            percent = (int)((elapsed * 100UL) / arcDurationMs);
        }
        if (percent > 100) percent = 100;
        lv_arc_set_value(ui_get_timer_arc(), percent);
    }

    /* Update timer text (whole seconds) */
    if (currentState == AppState::TIMING || currentState == AppState::ALERT) {
        long remainingMs = (long)arcDurationMs - (long)elapsed;
        char buf[16];

        if (remainingMs < 0) {
            /* Overtime */
            int overSec = (int)(-remainingMs / 1000);
            int mins = overSec / 60;
            int secs = overSec % 60;
            if (mins > 0) {
                snprintf(buf, sizeof(buf), "+%d:%02d", mins, secs);
            } else {
                snprintf(buf, sizeof(buf), "+%d", secs);
            }
        } else {
            int totalSec = (int)(remainingMs / 1000);
            int mins = totalSec / 60;
            int secs = totalSec % 60;
            if (mins > 0) {
                snprintf(buf, sizeof(buf), "%d:%02d", mins, secs);
            } else {
                snprintf(buf, sizeof(buf), "%d", secs);
            }
        }
        lv_label_set_text(ui_get_timer_label(), buf);
    }
}

void ui_toggle_pressure_unit()
{
    showBar = !showBar;

    /* Highlight active unit, dim inactive */
    lv_obj_set_style_text_color(ui_get_pressure_unit_kg(),
                                showBar ? COLOR_DIMMED : COLOR_ON_SURFACE, 0);
    lv_obj_set_style_text_color(ui_get_pressure_unit_bar(),
                                showBar ? COLOR_ON_SURFACE : COLOR_DIMMED, 0);

    /* Refresh displayed value with cached pressure */
    char buf[16];
    format_pressure(lastPressureGrams, buf, sizeof(buf));
    lv_label_set_text(ui_get_pressure_label(), buf);
}
