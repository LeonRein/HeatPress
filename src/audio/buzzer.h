#ifndef BUZZER_H
#define BUZZER_H

/**
 * Initialize the buzzer (LEDC PWM on the CYD audio output pin).
 * Call once from the task that will drive the buzzer.
 */
void buzzer_init();

/**
 * Start emitting a tone at the configured frequency.
 */
void buzzer_on();

/**
 * Stop the tone.
 */
void buzzer_off();

#endif /* BUZZER_H */
