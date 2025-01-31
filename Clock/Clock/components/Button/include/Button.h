#ifndef BUTTON_H
#define BUTTON_H

/* Driver */
#include <driver/gpio.h>
#include <esp_timer.h>

/* C Library */
#include <stdio.h>
#include <stdbool.h>

/* Definitions */
#define BUTTON_COUNT 3

// Pins
#define UP_BUTTON GPIO_NUM_4
#define DOWN_BUTTON GPIO_NUM_5
#define SELECT_BUTTON GPIO_NUM_6

/* Globals */
extern int64_t buttonPressedStartTime;

/* Global Struct or Enums*/
typedef struct button_t {
    // Data
    gpio_num_t gpioPin;
    gpio_config_t buttonConfig;
    
    // Functions
    volatile bool *(*pressed)(gpio_num_t);
    volatile int64_t  *(*pressDuration)(gpio_num_t);
    
} button_t;


extern void initializeButtons(button_t *buttons);

extern esp_err_t configureButtons(button_t *buttons);

extern volatile bool *getPressState(gpio_num_t buttonPin);

extern volatile int64_t *calculatePressDuration(gpio_num_t buttonPin);

#endif