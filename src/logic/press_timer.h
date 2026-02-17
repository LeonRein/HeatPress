#ifndef PRESS_TIMER_H
#define PRESS_TIMER_H

#include "app_state.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

/**
 * Press timer state machine.
 * Manages state transitions based on pressure readings and user actions.
 */
class PressTimer {
public:
    PressTimer(QueueHandle_t uiQueue, QueueHandle_t actionQueue);

    /**
     * Process a new pressure reading.
     * Evaluates state transitions and sends UI commands.
     */
    void processPressure(float pressure);

    /**
     * Process a user action (button press).
     */
    void processAction(const UserAction &action);

    /**
     * Called periodically (~100ms) to update countdown.
     */
    void tick();

    AppState getState() const { return state_; }
    int getTimerDuration() const { return timerDuration_; }
    int getTimerRemaining() const { return timerRemaining_; }

private:
    void transitionTo(AppState newState);
    void sendUICommand(const UICommand &cmd);
    void updateTimerDisplay();
    void updateArc();

    QueueHandle_t uiQueue_;
    QueueHandle_t actionQueue_;

    AppState state_           = AppState::IDLE;
    int      timerDuration_   = 0;   /* Set from config on construction */
    int      timerRemaining_  = 0;
    float    currentPressure_ = 0.0f;

    unsigned long timerStartMs_  = 0;
    unsigned long lastTickMs_    = 0;
};

#endif /* PRESS_TIMER_H */
