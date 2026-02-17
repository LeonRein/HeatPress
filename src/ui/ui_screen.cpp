#include "ui_screen.h"
#include "ui_theme.h"
#include "../config.h"
#include "../logic/app_state.h"

/* ── Widget handles ─────────────────────────────────────── */
static lv_obj_t *scr             = nullptr;
static lv_obj_t *pressure_card   = nullptr;
static lv_obj_t *pressure_label  = nullptr;
static lv_obj_t *pressure_unit   = nullptr;
static lv_obj_t *status_label    = nullptr;
static lv_obj_t *timer_arc       = nullptr;
static lv_obj_t *timer_label     = nullptr;
static lv_obj_t *timer_set_label = nullptr;
static lv_obj_t *btn_minus       = nullptr;
static lv_obj_t *btn_plus        = nullptr;
static lv_obj_t *btn_tare        = nullptr;

static QueueHandle_t s_actionQueue = nullptr;

/* ── Button event callbacks ──────────────────────────────── */

static void btn_minus_cb(lv_event_t *e)
{
    UserAction action = { UserActionType::TIMER_DECREMENT };
    xQueueSend(s_actionQueue, &action, 0);
}

static void btn_plus_cb(lv_event_t *e)
{
    UserAction action = { UserActionType::TIMER_INCREMENT };
    xQueueSend(s_actionQueue, &action, 0);
}

static void btn_tare_cb(lv_event_t *e)
{
    UserAction action = { UserActionType::TARE };
    xQueueSend(s_actionQueue, &action, 0);
}

static void screen_click_cb(lv_event_t *e)
{
    /* Clicking anywhere dismisses alert */
    UserAction action = { UserActionType::ACKNOWLEDGE_ALERT };
    xQueueSend(s_actionQueue, &action, 0);
}

/* ── Helper: create a material button ────────────────────── */

static lv_obj_t* create_btn(lv_obj_t *parent, const char *text,
                             lv_event_cb_t cb, bool is_tare = false)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_add_style(btn, &style_btn, LV_STATE_DEFAULT);
    lv_obj_add_style(btn, &style_btn_pressed, LV_STATE_PRESSED);
    if (is_tare) {
        lv_obj_add_style(btn, &style_btn_tare, LV_STATE_DEFAULT);
    }
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_center(lbl);

    return btn;
}

/* ── Build the screen ────────────────────────────────────── */

void ui_screen_create(QueueHandle_t actionQueue)
{
    s_actionQueue = actionQueue;

    scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, COLOR_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Click handler for alert dismissal */
    lv_obj_add_event_cb(scr, screen_click_cb, LV_EVENT_CLICKED, nullptr);

    /* ── Timer arc (left side, hero element) ──────── */
    timer_arc = lv_arc_create(scr);
    lv_obj_set_size(timer_arc, 140, 140);
    lv_obj_align(timer_arc, LV_ALIGN_TOP_LEFT, 12, 18);
    lv_arc_set_rotation(timer_arc, 270);
    lv_arc_set_bg_angles(timer_arc, 0, 360);
    lv_arc_set_range(timer_arc, 0, 100);
    lv_arc_set_value(timer_arc, 0);
    lv_obj_remove_style(timer_arc, nullptr, LV_PART_KNOB);
    lv_obj_clear_flag(timer_arc, LV_OBJ_FLAG_CLICKABLE);

    /* Arc colors */
    lv_obj_set_style_arc_color(timer_arc, COLOR_SURFACE, LV_PART_MAIN);
    lv_obj_set_style_arc_color(timer_arc, COLOR_PRIMARY, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(timer_arc, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_width(timer_arc, 10, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(timer_arc, true, LV_PART_INDICATOR);

    /* Timer text inside arc */
    timer_label = lv_label_create(timer_arc);
    lv_obj_add_style(timer_label, &style_label_large, 0);
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", TIMER_DEFAULT_SECONDS);
    lv_label_set_text(timer_label, buf);
    lv_obj_align(timer_label, LV_ALIGN_CENTER, 0, -4);

    /* Timer "sec" subtitle */
    lv_obj_t *sec_label = lv_label_create(timer_arc);
    lv_obj_add_style(sec_label, &style_label_small, 0);
    lv_label_set_text(sec_label, "sec");
    lv_obj_align(sec_label, LV_ALIGN_CENTER, 0, 20);

    /* ── Right side info panel ────────────────────── */

    /* Pressure card */
    pressure_card = lv_obj_create(scr);
    lv_obj_add_style(pressure_card, &style_card, 0);
    lv_obj_set_size(pressure_card, 140, 70);
    lv_obj_align(pressure_card, LV_ALIGN_TOP_RIGHT, -10, 18);
    lv_obj_clear_flag(pressure_card, LV_OBJ_FLAG_SCROLLABLE);

    /* Pressure value */
    pressure_label = lv_label_create(pressure_card);
    lv_obj_add_style(pressure_label, &style_label_medium, 0);
    lv_label_set_text(pressure_label, "0.0");
    lv_obj_align(pressure_label, LV_ALIGN_LEFT_MID, 0, -8);

    /* Unit label */
    pressure_unit = lv_label_create(pressure_card);
    lv_obj_add_style(pressure_unit, &style_label_small, 0);
    lv_label_set_text(pressure_unit, "grams");
    lv_obj_align(pressure_unit, LV_ALIGN_BOTTOM_LEFT, 2, 0);

    /* Status label (right side, below pressure card) */
    status_label = lv_label_create(scr);
    lv_obj_add_style(status_label, &style_label_small, 0);
    lv_label_set_text(status_label, "IDLE");
    lv_obj_align(status_label, LV_ALIGN_TOP_RIGHT, -20, 104);

    /* Timer setting label (right side, below status) */
    timer_set_label = lv_label_create(scr);
    lv_obj_add_style(timer_set_label, &style_label_small, 0);
    char setbuf[24];
    snprintf(setbuf, sizeof(setbuf), "Timer: %ds", TIMER_DEFAULT_SECONDS);
    lv_label_set_text(timer_set_label, setbuf);
    lv_obj_align(timer_set_label, LV_ALIGN_TOP_RIGHT, -20, 126);

    /* ── Bottom button row ────────────────────────── */
    lv_obj_t *btn_row = lv_obj_create(scr);
    lv_obj_set_size(btn_row, 300, 56);
    lv_obj_align(btn_row, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_row, 0, 0);
    lv_obj_set_style_pad_all(btn_row, 0, 0);
    lv_obj_clear_flag(btn_row, LV_OBJ_FLAG_SCROLLABLE);

    /* Use flex layout for button row */
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    btn_minus = create_btn(btn_row, LV_SYMBOL_MINUS " 5s", btn_minus_cb);
    lv_obj_set_size(btn_minus, 80, 42);

    btn_plus = create_btn(btn_row, LV_SYMBOL_PLUS " 5s", btn_plus_cb);
    lv_obj_set_size(btn_plus, 80, 42);

    btn_tare = create_btn(btn_row, "TARE", btn_tare_cb, true);
    lv_obj_set_size(btn_tare, 90, 42);
}

/* ── Getters ─────────────────────────────────────────────── */

lv_obj_t* ui_get_pressure_label()      { return pressure_label; }
lv_obj_t* ui_get_timer_label()         { return timer_label; }
lv_obj_t* ui_get_timer_arc()           { return timer_arc; }
lv_obj_t* ui_get_pressure_card()       { return pressure_card; }
lv_obj_t* ui_get_status_label()        { return status_label; }
lv_obj_t* ui_get_timer_setting_label() { return timer_set_label; }
