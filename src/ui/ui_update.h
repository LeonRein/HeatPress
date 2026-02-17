#ifndef UI_UPDATE_H
#define UI_UPDATE_H

#include "../logic/app_state.h"

/**
 * Process a UICommand and update the corresponding widget.
 * MUST be called from the UI task only (LVGL is not thread-safe).
 */
void ui_handle_command(const UICommand &cmd);

/**
 * Update the timer setting label with the configured duration.
 * Called when user changes timer via +/- buttons.
 */
void ui_update_timer_setting(int durationSeconds);

#endif /* UI_UPDATE_H */
