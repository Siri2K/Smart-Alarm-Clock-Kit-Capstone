#ifndef BUZZER_H
#define BUZZER_H


/* Drivers */
#include "freertos/FreeRTOS.h"
#include <driver/gpio.h>
#include <driver/ledc.h>

/* C Library */
#include <stdio.h>

/* Debug Logs */
#include "esp_log.h"

/* Definitions */
// PWM
#define BUZZER_TIMER  LEDC_TIMER_0 // TIMER
#define BUZZER_MODE   LEDC_LOW_SPEED_MODE // Speed Mode
#define BUZZER_CHANNEL LEDC_CHANNEL_0 // Timer Channel
#define BUZZER_DUTY_RES  LEDC_TIMER_10_BIT // 11-Bit Duty Resolution
#define BUZZER_FREQUENCY (2048) // Frequency

// Pin
#define BUZZER GPIO_NUM_14

/* Global Struct or Enums*/
typedef enum buzzer_power_t{
    NONE = 0,
    LOW = 25,
    MID = 50,
    HIGH = 75,
    MAX = 99
}buzzer_power_t;

typedef struct buzzer_t {
   // PWM Configuation
   ledc_timer_config_t buzzerTimerConfig; // Timer
   ledc_channel_config_t buzzerChannelConfig; // PWM Channel

   // Task
   void (*powerOn)(ledc_mode_t,ledc_channel_t, buzzer_power_t);

} buzzer_t;

/* Functions */
extern void initializeBuzzer(buzzer_t *buzzer);

extern void setPowerOn(ledc_mode_t mode,ledc_channel_t channel, buzzer_power_t buzzerPower);

#endif