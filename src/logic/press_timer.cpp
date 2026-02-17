#include "press_timer.h"
#include "../config.h"
#include <Arduino.h>

PressTimer::PressTimer(QueueHandle_t uiQueue, QueueHandle_t actionQueue)
    : uiQueue_(uiQueue)
    , actionQueue_(actionQueue)
    , timerDuration_(TIMER_DEFAULT_SECONDS)
    , timerRemaining_(TIMER_DEFAULT_SECONDS)
{
}

void PressTimer::processPressure(float pressure)
{
    currentPressure_ = pressure;

    /* Only send pressure to UI if the displayed value (2 decimal kg) changed */
    int displayVal = (int)(pressure / 10.0f); /* 0.01 kg resolution */
    if (displayVal != lastDisplayPressure_) {
        lastDisplayPressure_ = displayVal;
        UICommand cmd;
        cmd.type     = UICommandType::UPDATE_PRESSURE;
        cmd.pressure = pressure;
        sendUICommand(cmd);
    }

    bool aboveThreshold = (pressure > PRESSURE_THRESHOLD);

    switch (state_) {
        case AppState::CALIBRATING:
            /* First valid reading after calibration/tare */
            transitionTo(AppState::IDLE);
            return;

        case AppState::IDLE:
            if (aboveThreshold) {
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
            sendTimerSettingUpdate();
            break;

        case UserActionType::TIMER_DECREMENT:
            timerDuration_ -= TIMER_STEP_SECONDS;
            if (timerDuration_ < TIMER_MIN_SECONDS)
                timerDuration_ = TIMER_MIN_SECONDS;
            updateTimerDisplay();
            sendTimerSettingUpdate();
            break;

        case UserActionType::ACKNOWLEDGE_ALERT:
            if (state_ == AppState::ALERT) {
                transitionTo(AppState::IDLE);
            }
            break;

        case UserActionType::TARE:
            transitionTo(AppState::CALIBRATING);
            break;
    }
}

void PressTimer::tick()
{
    if (state_ == AppState::TIMING) {
        unsigned long elapsed = millis() - timerStartMs_;
        int elapsedSeconds    = (int)(elapsed / 1000);

        if (elapsedSeconds >= timerDuration_) {
            timerRemaining_ = timerDuration_ - elapsedSeconds;
            transitionTo(AppState::ALERT);
        }
    }
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
    cmd.timerSeconds = timerDuration_;
    sendUICommand(cmd);
}

void PressTimer::sendTimerSettingUpdate()
{
    UICommand cmd;
    cmd.type         = UICommandType::UPDATE_TIMER_SETTING;
    cmd.timerSeconds = timerDuration_;
    sendUICommand(cmd);
}
