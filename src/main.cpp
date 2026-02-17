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

/* ── Sensor init complete flag ────────────────────────────── */
static volatile bool sensorInitDone  = false;
static volatile bool sensorInitOk    = false;

/* ── UI Task ─────────────────────────────────────────────── */

static void uiTask(void *pvParam)
{
    /* Initialize display + LVGL */
    lv_setup_init();
    ui_theme_init();

    /* ── Init / splash screen ───────────────────────── */
    lv_obj_t *initScr = lv_obj_create(lv_scr_act());
    lv_obj_set_size(initScr, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align(initScr, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(initScr, COLOR_BG, 0);
    lv_obj_set_style_bg_opa(initScr, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(initScr, 0, 0);
    lv_obj_clear_flag(initScr, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *spinner = lv_spinner_create(initScr, 1000, 60);
    lv_obj_set_size(spinner, 60, 60);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_style_arc_color(spinner, COLOR_SURFACE, LV_PART_MAIN);
    lv_obj_set_style_arc_color(spinner, COLOR_PRIMARY, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_INDICATOR);

    lv_obj_t *initLabel = lv_label_create(initScr);
    lv_obj_add_style(initLabel, &style_label_small, 0);
    lv_label_set_text(initLabel, "Calibrating...\nDo not apply pressure");
    lv_obj_set_style_text_align(initLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(initLabel, LV_ALIGN_CENTER, 0, 30);

    /* Drive LVGL while waiting for sensor init */
    TickType_t xLastWake = xTaskGetTickCount();
    while (!sensorInitDone) {
        lv_setup_update();
        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(UI_REFRESH_PERIOD_MS));
    }

    /* Remove init screen */
    lv_obj_del(initScr);

    /* ── Build main screen ──────────────────────────── */
    ui_screen_create(actionQueue);

    xLastWake = xTaskGetTickCount();

    for (;;) {
        /* Process all pending UI commands from the logic task */
        UICommand cmd;
        while (xQueueReceive(uiQueue, &cmd, 0) == pdTRUE) {
            ui_handle_command(cmd);
        }

        /* Smooth arc animation (local timing, every frame) */
        ui_arc_tick();

        /* Drive LVGL (rendering, animations, input) */
        lv_setup_update();

        /* Maintain steady frame rate */
        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(UI_REFRESH_PERIOD_MS));
    }
}

/* ── Sensor Task ─────────────────────────────────────────── */

static void sensorTask(void *pvParam)
{
    /* Initialize load cell (blocking ~2s tare) */
    bool ok = loadcell_init();
    sensorInitOk   = ok;
    sensorInitDone = true;

    if (!ok) {
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