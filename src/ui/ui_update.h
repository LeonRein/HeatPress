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

/**
 * Call every UI frame to smoothly animate the arc.
 * Uses local timing for jitter-free 30fps updates.
 */
void ui_arc_tick();

/**
 * Toggle pressure display between kg and bar.
 * Called from the pressure card click handler.
 */
void ui_toggle_pressure_unit();

/**
 * Toggle mute state for the alert buzzer.
 * Called from the mute button click handler.
 */
void ui_toggle_mute();

#endif /* UI_UPDATE_H */
