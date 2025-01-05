#ifndef BUTTON_H
#define BUTTON_H

/* Headers */ 
#include "Pins/pins.h"

/* Driver */
#include "esp_timer.h"

/* C Library */
#include <stdio.h>
#include <stdbool.h>

/* Definitions */
#define BUTTON_COUNT 3

/* Globals */
int64_t buttonPressedStartTime = 0;

/* Global Struct or Enums*/
typedef struct button_t {
    // Data
    gpio_config_t buttonConfig;
    
    // Functions
    volatile bool *(*pressed)(gpio_num_t *);
    volatile int64_t  *(*pressDuration)(gpio_num_t *);
    
} button_t;


void initializeButtons(button_t **buttons);

esp_err_t configureButtons(button_t *buttons);

volatile bool *getPressState(gpio_num_t *buttonPin);

volatile int64_t *calculatePressDuration(gpio_num_t *buttonPin);

void testPress(button_t *button);

#endif