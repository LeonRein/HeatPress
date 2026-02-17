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

/* ── State color helpers ─────────────────────────────────── */

static void set_idle_colors()
{
    lv_obj_set_style_bg_color(ui_get_pressure_card(), COLOR_CARD, 0);
    lv_obj_set_style_arc_color(ui_get_timer_arc(), COLOR_PRIMARY, LV_PART_INDICATOR);

    lv_obj_t *status = ui_get_status_label();
    lv_label_set_text(status, "IDLE");
    lv_obj_set_style_text_color(status, COLOR_DIMMED, 0);

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
    lv_obj_set_style_arc_color(ui_get_timer_arc(), COLOR_ERROR, LV_PART_INDICATOR);
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
            lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x3A1010), 0);
        } else {
            lv_obj_set_style_bg_color(ui_get_pressure_card(), COLOR_CARD, 0);
            lv_obj_set_style_bg_color(lv_scr_act(), COLOR_BG, 0);
        }
    }
}

/* ── Public API ───────────────────────────────────────────── */

void ui_handle_command(const UICommand &cmd)
{
    switch (cmd.type) {
        case UICommandType::UPDATE_PRESSURE: {
            char buf[16];
            snprintf(buf, sizeof(buf), "%.1f", cmd.pressure);
            lv_label_set_text(ui_get_pressure_label(), buf);
            break;
        }

        case UICommandType::UPDATE_TIMER: {
            int secs = cmd.timerSeconds;
            int mins = secs / 60;
            int remainder = secs % 60;
            char buf[8];
            if (mins > 0) {
                snprintf(buf, sizeof(buf), "%d:%02d", mins, remainder);
            } else {
                snprintf(buf, sizeof(buf), "%d", secs);
            }
            lv_label_set_text(ui_get_timer_label(), buf);
            break;
        }

        case UICommandType::UPDATE_STATE: {
            currentState = cmd.state;
            switch (cmd.state) {
                case AppState::IDLE:     set_idle_colors();     break;
                case AppState::PRESSING: set_pressing_colors(); break;
                case AppState::TIMING:   set_timing_colors();   break;
                case AppState::ALERT:    set_alert_colors();    break;
            }
            break;
        }

        case UICommandType::UPDATE_ARC: {
            lv_arc_set_value(ui_get_timer_arc(), cmd.arcPercent);
            break;
        }
    }

    /* Run blink effect if in alert state */
    alert_blink_tick();
}

void ui_update_timer_setting(int durationSeconds)
{
    char buf[24];
    snprintf(buf, sizeof(buf), "Timer: %ds", durationSeconds);
    lv_label_set_text(ui_get_timer_setting_label(), buf);
}
