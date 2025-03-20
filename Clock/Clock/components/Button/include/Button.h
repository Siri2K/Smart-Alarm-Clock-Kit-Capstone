#ifndef BUTTON_H
#define BUTTON_H

/* Driver */
#include <driver/gpio.h>
#include <esp_timer.h>

/* Debug Logs */
#include "esp_log.h"

/* C Library */
#include <stdio.h>

/* Definitions */
#define BUTTON_COUNT 3

// Pins
#define UP_BUTTON GPIO_NUM_36
#define DOWN_BUTTON GPIO_NUM_39
#define SELECT_BUTTON GPIO_NUM_34

    /* Globals */
extern int64_t buttonPressedStartTime;

/* Global Struct or Enums*/
typedef struct button_t { 
    // Functions
    int8_t (*pressed)(gpio_num_t);
    int64_t  (*pressDuration)(gpio_num_t);
    
} button_t;


extern void initializeButton(button_t *button);

extern int8_t getPressState(gpio_num_t buttonPin);

extern int64_t calculatePressDuration(gpio_num_t buttonPin);

#endif