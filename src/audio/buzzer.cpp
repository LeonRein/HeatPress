#include "buzzer.h"
#include "../config.h"
#include <Arduino.h>

void buzzer_init()
{
    ledcSetup(BUZZER_LEDC_CHANNEL, BUZZER_FREQ_HZ, 8);
    ledcAttachPin(PIN_AUDIO_OUT, BUZZER_LEDC_CHANNEL);
    ledcWrite(BUZZER_LEDC_CHANNEL, 0);
}

void buzzer_on()
{
    ledcWriteTone(BUZZER_LEDC_CHANNEL, BUZZER_FREQ_HZ);
}

void buzzer_off()
{
    ledcWriteTone(BUZZER_LEDC_CHANNEL, 0);
}
