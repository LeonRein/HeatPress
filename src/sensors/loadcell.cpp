#include "loadcell.h"
#include "../config.h"

#include <HX711_ADC.h>
#include <Arduino.h>

static HX711_ADC LoadCell(PIN_HX711_DT, PIN_HX711_CLK);

/* Atomic tare request flag (safe across tasks) */
static volatile bool tareRequested = false;

bool loadcell_init()
{
    LoadCell.begin();
    LoadCell.start(LOADCELL_STABILIZE_MS, true);

    if (LoadCell.getTareTimeoutFlag()) {
        return false;
    }

    LoadCell.setCalFactor(LOADCELL_CAL_FACTOR);

    /* Reduce smoothing for fast response.
     * Default is 16 samples (~1.6s lag at 10 SPS).
     * Use 1 for instant response (no averaging). */
    LoadCell.setSamplesInUse(1);

    return true;
}

bool loadcell_read(float &pressure)
{
    /* Handle pending tare request */
    if (tareRequested) {
        loadcell_do_tare();
        tareRequested = false;
    }

    if (LoadCell.dataWaitingAsync()) {
        if (LoadCell.updateAsync()) {
            pressure = LoadCell.getData();
            return true;
        }
    }
    return false;
}

void loadcell_request_tare()
{
    tareRequested = true;
}

void loadcell_do_tare()
{
    LoadCell.tareNoDelay();
    /* Wait for tare to complete */
    unsigned long start = millis();
    while (!LoadCell.getTareStatus() && (millis() - start < 2000)) {
        LoadCell.update();
        delay(10);
    }
}
