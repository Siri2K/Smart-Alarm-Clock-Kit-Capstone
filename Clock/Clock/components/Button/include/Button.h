#ifndef BUTTON_H
#define BUTTON_H

/* Driver */
#include <driver/gpio.h>
#include <esp_timer.h>

/* C Library */
#include <stdio.h>

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
    uint8_t *(*pressed)(gpio_num_t);
    int64_t  *(*pressDuration)(gpio_num_t);
    
} button_t;


extern void initializeButton(button_t *button, gpio_num_t buttonPin);

extern esp_err_t configureButton(button_t *button, gpio_num_t buttonPin);

extern uint8_t *getPressState(gpio_num_t buttonPin);

extern int64_t *calculatePressDuration(gpio_num_t buttonPin);

#endif