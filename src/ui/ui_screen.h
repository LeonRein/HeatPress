#ifndef UI_SCREEN_H
#define UI_SCREEN_H

#include <lvgl.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

/**
 * Build the main screen UI. Call once after lv_setup_init() and ui_theme_init().
 * @param actionQueue  Queue for sending user actions to the logic task
 */
void ui_screen_create(QueueHandle_t actionQueue);

/**
 * Get references to UI widgets for updates.
 */
lv_obj_t* ui_get_pressure_label();
lv_obj_t* ui_get_timer_label();
lv_obj_t* ui_get_timer_arc();
lv_obj_t* ui_get_pressure_card();
lv_obj_t* ui_get_status_label();
lv_obj_t* ui_get_timer_setting_label();
lv_obj_t* ui_get_pressure_unit_kg();
lv_obj_t* ui_get_pressure_unit_bar();
lv_obj_t* ui_get_mute_btn();
lv_obj_t* ui_get_mute_label();

#endif /* UI_SCREEN_H */
