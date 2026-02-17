#ifndef UI_THEME_H
#define UI_THEME_H

#include <lvgl.h>

/**
 * Shared LVGL styles for the Material-inspired dark theme.
 */
extern lv_style_t style_card;
extern lv_style_t style_btn;
extern lv_style_t style_btn_pressed;
extern lv_style_t style_btn_tare;
extern lv_style_t style_label_large;
extern lv_style_t style_label_medium;
extern lv_style_t style_label_small;

/**
 * Initialize all theme styles. Call once after lv_init().
 */
void ui_theme_init();

#endif /* UI_THEME_H */
