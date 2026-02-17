#include "press_timer.h"
#include "../config.h"
#include <Arduino.h>

PressTimer::PressTimer(QueueHandle_t uiQueue, QueueHandle_t actionQueue)
    : uiQueue_(uiQueue)
    , actionQueue_(actionQueue)
    , timerDuration_(TIMER_DEFAULT_SECONDS)
    , timerRemaining_(TIMER_DEFAULT_SECONDS)
{
    lastTickMs_ = millis();
}

void PressTimer::processPressure(float pressure)
{
    currentPressure_ = pressure;

    /* Send pressure to UI */
    UICommand cmd;
    cmd.type     = UICommandType::UPDATE_PRESSURE;
    cmd.pressure = pressure;
    sendUICommand(cmd);

    bool aboveThreshold = (pressure > PRESSURE_THRESHOLD);

    switch (state_) {
        case AppState::IDLE:
            if (aboveThreshold) {
                transitionTo(AppState::PRESSING);
            }
            break;

        case AppState::PRESSING:
            if (!aboveThreshold) {
                transitionTo(AppState::IDLE);
            } else {
                /* Start countdown immediately */
                timerRemaining_ = timerDuration_;
                timerStartMs_   = millis();
                transitionTo(AppState::TIMING);
            }
            break;

        case AppState::TIMING:
            if (!aboveThreshold) {
                transitionTo(AppState::IDLE);
            }
            break;

        case AppState::ALERT:
            if (!aboveThreshold) {
                transitionTo(AppState::IDLE);
            }
            break;
    }
}

void PressTimer::processAction(const UserAction &action)
{
    switch (action.type) {
        case UserActionType::TIMER_INCREMENT:
            timerDuration_ += TIMER_STEP_SECONDS;
            if (timerDuration_ > TIMER_MAX_SECONDS)
                timerDuration_ = TIMER_MAX_SECONDS;
            updateTimerDisplay();
            break;

        case UserActionType::TIMER_DECREMENT:
            timerDuration_ -= TIMER_STEP_SECONDS;
            if (timerDuration_ < TIMER_MIN_SECONDS)
                timerDuration_ = TIMER_MIN_SECONDS;
            updateTimerDisplay();
            break;

        case UserActionType::ACKNOWLEDGE_ALERT:
            if (state_ == AppState::ALERT) {
                transitionTo(AppState::IDLE);
            }
            break;

        case UserActionType::TARE:
            /* Handled externally — sensor task picks it up */
            break;
    }
}

void PressTimer::tick()
{
    unsigned long now = millis();

    if (state_ == AppState::TIMING) {
        unsigned long elapsed = now - timerStartMs_;
        int elapsedSeconds    = (int)(elapsed / 1000);
        timerRemaining_       = timerDuration_ - elapsedSeconds;

        if (timerRemaining_ <= 0) {
            timerRemaining_ = timerDuration_ - elapsedSeconds;
            transitionTo(AppState::ALERT);
        }
    }

    if (state_ == AppState::ALERT) {
        /* Keep counting overtime */
        unsigned long elapsed = now - timerStartMs_;
        int elapsedSeconds    = (int)(elapsed / 1000);
        timerRemaining_       = timerDuration_ - elapsedSeconds;
    }

    lastTickMs_ = now;
}

void PressTimer::transitionTo(AppState newState)
{
    state_ = newState;

    UICommand cmd;
    cmd.type  = UICommandType::UPDATE_STATE;
    cmd.state = newState;
    sendUICommand(cmd);

    if (newState == AppState::IDLE) {
        timerRemaining_ = timerDuration_;
        updateTimerDisplay();
        /* Arc reset is handled by UI on state change */
    }

    if (newState == AppState::TIMING) {
        updateTimerDisplay();

        /* Tell UI to start arc animation over the full duration */
        UICommand arcCmd;
        arcCmd.type       = UICommandType::START_ARC_ANIM;
        arcCmd.durationMs = timerDuration_ * 1000;
        sendUICommand(arcCmd);
    }
}

void PressTimer::sendUICommand(const UICommand &cmd)
{
    /* Non-blocking: if queue is full, drop the message */
    xQueueSend(uiQueue_, &cmd, 0);
}

void PressTimer::updateTimerDisplay()
{
    UICommand cmd;
    cmd.type         = UICommandType::UPDATE_TIMER;
    cmd.timerSeconds = (state_ == AppState::TIMING || state_ == AppState::ALERT)
                        ? timerRemaining_
                        : timerDuration_;
    sendUICommand(cmd);
}
