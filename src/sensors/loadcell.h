#ifndef LOADCELL_H
#define LOADCELL_H

/**
 * Initialize the HX711 load cell.
 * Blocks during stabilization (~2 seconds).
 * @return true on success, false on timeout/error
 */
bool loadcell_init();

/**
 * Non-blocking read of the load cell.
 * Call this periodically from the sensor task.
 * @param[out] pressure  Filled with current reading if data is available
 * @return true if a new reading was obtained
 */
bool loadcell_read(float &pressure);

/**
 * Tare (zero) the load cell.
 * Can be called from any task — the actual tare happens on next read cycle.
 */
void loadcell_request_tare();

/**
 * Check if a tare has been requested.
 * Called internally by the sensor task.
 */
bool loadcell_tare_pending();

/**
 * Execute the tare operation (blocking, ~1s).
 */
void loadcell_do_tare();

#endif /* LOADCELL_H */
