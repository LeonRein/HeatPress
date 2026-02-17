#include "ui_theme.h"
#include "../config.h"

lv_style_t style_card;
lv_style_t style_btn;
lv_style_t style_btn_pressed;
lv_style_t style_btn_tare;
lv_style_t style_label_large;
lv_style_t style_label_medium;
lv_style_t style_label_small;

void ui_theme_init()
{
    /* ── Card style ────────────────────────────────── */
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, COLOR_CARD);
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, 12);
    lv_style_set_pad_all(&style_card, 10);
    lv_style_set_border_width(&style_card, 0);
    lv_style_set_shadow_width(&style_card, 8);
    lv_style_set_shadow_color(&style_card, lv_color_hex(0x000000));
    lv_style_set_shadow_opa(&style_card, LV_OPA_40);

    /* ── Button style ──────────────────────────────── */
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, COLOR_PRIMARY);
    lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_radius(&style_btn, 8);
    lv_style_set_text_color(&style_btn, lv_color_hex(0x000000));
    lv_style_set_text_font(&style_btn, &lv_font_montserrat_16);
    lv_style_set_pad_hor(&style_btn, 16);
    lv_style_set_pad_ver(&style_btn, 8);
    lv_style_set_border_width(&style_btn, 0);
    lv_style_set_shadow_width(&style_btn, 4);
    lv_style_set_shadow_color(&style_btn, lv_color_hex(0x000000));
    lv_style_set_shadow_opa(&style_btn, LV_OPA_30);

    /* ── Button pressed style ──────────────────────── */
    lv_style_init(&style_btn_pressed);
    lv_style_set_bg_color(&style_btn_pressed, lv_color_hex(0x9A67EA));
    lv_style_set_shadow_width(&style_btn_pressed, 1);
    lv_style_set_translate_y(&style_btn_pressed, 2);

    /* ── Tare button (secondary color) ─────────────── */
    lv_style_init(&style_btn_tare);
    lv_style_set_bg_color(&style_btn_tare, COLOR_SECONDARY);
    lv_style_set_text_color(&style_btn_tare, lv_color_hex(0x000000));

    /* ── Large label (pressure value) ──────────────── */
    lv_style_init(&style_label_large);
    lv_style_set_text_color(&style_label_large, COLOR_ON_BG);
    lv_style_set_text_font(&style_label_large, &lv_font_montserrat_36);

    /* ── Medium label (timer) ──────────────────────── */
    lv_style_init(&style_label_medium);
    lv_style_set_text_color(&style_label_medium, COLOR_ON_SURFACE);
    lv_style_set_text_font(&style_label_medium, &lv_font_montserrat_24);

    /* ── Small label (units, captions) ─────────────── */
    lv_style_init(&style_label_small);
    lv_style_set_text_color(&style_label_small, COLOR_DIMMED);
    lv_style_set_text_font(&style_label_small, &lv_font_montserrat_14);
}
