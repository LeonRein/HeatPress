#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdint.h>

/**
 * Application states for the heat press state machine
 */
enum class AppState : uint8_t {
    IDLE,       // No pressure detected
    PRESSING,   // Pressure above threshold, timer starting
    TIMING,     // Countdown in progress
    ALERT       // Timer expired, alerting user
};

/**
 * Message sent from sensor task → logic task
 */
struct SensorData {
    float pressure;       // Current pressure reading in grams
    bool  isValid;        // Whether the reading is valid
};

/**
 * Commands sent from logic task → UI task
 */
enum class UICommandType : uint8_t {
    UPDATE_PRESSURE,      // Update pressure display
    UPDATE_TIMER,         // Update timer display
    UPDATE_STATE,         // State transition
    START_ARC_ANIM,       // Start arc animation (sends durationMs)
    UPDATE_TIMER_SETTING, // Update "Timer: Xs" setting label
};

struct UICommand {
    UICommandType type;
    union {
        float    pressure;          // For UPDATE_PRESSURE
        int      timerSeconds;      // For UPDATE_TIMER / UPDATE_TIMER_SETTING
        AppState state;             // For UPDATE_STATE
        int      durationMs;        // For START_ARC_ANIM
    };
};

/**
 * Commands sent from UI task → logic task (button presses)
 */
enum class UserActionType : uint8_t {
    TIMER_INCREMENT,     // +5 seconds
    TIMER_DECREMENT,     // -5 seconds
    TARE,                // Tare the load cell
    ACKNOWLEDGE_ALERT,   // Dismiss alert
};

struct UserAction {
    UserActionType type;
};

#endif /* APP_STATE_H */
