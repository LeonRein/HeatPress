/**
 * HeatPress — ESP32 Heat Press Pressure Monitor
 *
 * Architecture:
 *   - UI Task     (Core 1, high priority)  : LVGL rendering + touch
 *   - Sensor Task (Core 0, medium priority) : HX711 async reads
 *   - Logic Task  (Core 0, medium priority) : State machine + timer
 *
 * Communication:
 *   sensorQueue : SensorData   (sensor → logic)
 *   uiQueue     : UICommand    (logic  → UI)
 *   actionQueue : UserAction   (UI     → logic)
 */

#include <Arduino.h>
#include <lvgl.h>

#include "config.h"
#include "display/lv_setup.h"
#include "sensors/loadcell.h"
#include "logic/app_state.h"
#include "logic/press_timer.h"
#include "ui/ui_theme.h"
#include "ui/ui_screen.h"
#include "ui/ui_update.h"

/* ── FreeRTOS Queues ─────────────────────────────────────── */
static QueueHandle_t sensorQueue = nullptr;   // SensorData
static QueueHandle_t uiQueue     = nullptr;   // UICommand
static QueueHandle_t actionQueue = nullptr;   // UserAction

/* ── UI Task ─────────────────────────────────────────────── */

static void uiTask(void *pvParam)
{
    /* Initialize display + LVGL (must happen on the task that will drive it) */
    lv_setup_init();
    ui_theme_init();
    ui_screen_create(actionQueue);

    TickType_t xLastWake = xTaskGetTickCount();

    for (;;) {
        /* Process all pending UI commands from the logic task */
        UICommand cmd;
        while (xQueueReceive(uiQueue, &cmd, 0) == pdTRUE) {
            ui_handle_command(cmd);
        }

        /* Drive LVGL (rendering, animations, input) */
        lv_setup_update();

        /* Maintain steady frame rate */
        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(UI_REFRESH_PERIOD_MS));
    }
}

/* ── Sensor Task ─────────────────────────────────────────── */

static void sensorTask(void *pvParam)
{
    /* Initialize load cell */
    if (!loadcell_init()) {
        /* Send error pressure forever so UI shows something */
        SensorData errData = { 0.0f, false };
        for (;;) {
            xQueueOverwrite(sensorQueue, &errData);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    TickType_t xLastWake = xTaskGetTickCount();

    for (;;) {
        float pressure = 0.0f;
        bool isNew = loadcell_read(pressure);

        if (isNew) {
            SensorData data = { pressure, true };
            xQueueOverwrite(sensorQueue, &data);
        }

        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(SENSOR_READ_INTERVAL_MS));
    }
}

/* ── Logic Task ──────────────────────────────────────────── */

static void logicTask(void *pvParam)
{
    PressTimer timer(uiQueue, actionQueue);

    /* Send initial timer display */
    UICommand initCmd;
    initCmd.type         = UICommandType::UPDATE_TIMER;
    initCmd.timerSeconds = TIMER_DEFAULT_SECONDS;
    xQueueSend(uiQueue, &initCmd, portMAX_DELAY);

    for (;;) {
        /* Check for new sensor data */
        SensorData sensorData;
        if (xQueueReceive(sensorQueue, &sensorData, 0) == pdTRUE) {
            if (sensorData.isValid) {
                timer.processPressure(sensorData.pressure);
            }
        }

        /* Check for user actions */
        UserAction action;
        while (xQueueReceive(actionQueue, &action, 0) == pdTRUE) {
            if (action.type == UserActionType::TARE) {
                loadcell_request_tare();
            } else {
                timer.processAction(action);
            }

            /* Update timer setting label after +/- press */
            if (action.type == UserActionType::TIMER_INCREMENT ||
                action.type == UserActionType::TIMER_DECREMENT) {
                UICommand cmd;
                cmd.type         = UICommandType::UPDATE_TIMER;
                cmd.timerSeconds = timer.getTimerDuration();
                xQueueSend(uiQueue, &cmd, 0);

                UICommand setCmd;
                setCmd.type         = UICommandType::UPDATE_TIMER_SETTING;
                setCmd.timerSeconds = timer.getTimerDuration();
                xQueueSend(uiQueue, &setCmd, 0);
            }
        }

        /* Tick the state machine (countdown updates) */
        timer.tick();

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* ── Arduino setup & loop ────────────────────────────────── */

void setup()
{
    Serial.begin(115200);
    Serial.println("HeatPress starting...");

    /* Create queues */
    sensorQueue = xQueueCreate(1, sizeof(SensorData));     // Overwrite-style
    uiQueue     = xQueueCreate(QUEUE_SIZE, sizeof(UICommand));
    actionQueue = xQueueCreate(QUEUE_SIZE, sizeof(UserAction));

    /* Create tasks pinned to specific cores */
    xTaskCreatePinnedToCore(
        uiTask, "UI", UI_TASK_STACK_SIZE, nullptr,
        UI_TASK_PRIORITY, nullptr, UI_TASK_CORE);

    xTaskCreatePinnedToCore(
        sensorTask, "Sensor", SENSOR_TASK_STACK_SIZE, nullptr,
        SENSOR_TASK_PRIORITY, nullptr, SENSOR_TASK_CORE);

    xTaskCreatePinnedToCore(
        logicTask, "Logic", LOGIC_TASK_STACK_SIZE, nullptr,
        LOGIC_TASK_PRIORITY, nullptr, LOGIC_TASK_CORE);
}

void loop()
{
    /* All work is done in FreeRTOS tasks; nothing needed here.
     * Yield to avoid watchdog issues. */
    vTaskDelay(pdMS_TO_TICKS(1000));
}