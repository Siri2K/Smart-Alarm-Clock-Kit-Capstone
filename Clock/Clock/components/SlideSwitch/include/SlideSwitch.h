#ifndef SLIDE_SWITCH_H
#define SLIDE_SWITCH_H

/* Headers */ 
#include <driver/gpio.h>

/* C Library */
#include <stdio.h>

/* Definition */
#define TIME_MODE_SWITCH GPIO_NUM_35
#define CLOCK_MODE_SWITCH GPIO_NUM_32
#define ALARM_MODE_SWITCH GPIO_NUM_33 


/* Global Struct or Enums*/
typedef enum {
    CLOCK = 1,
    TIME = 2,
    ALARM = 3,
    ERROR = -1
} clock_mode_t;

typedef struct slide_switch_t {
    // Data
    gpio_config_t sliderConfig;
    
    // Task
    clock_mode_t *(*mode)();
    
} slide_switch_t;

/* Tasks */
extern void initializeSlideSwitch(slide_switch_t *slideSwitch);

extern esp_err_t configureSlideSwitch(slide_switch_t *slideSwitch);

extern clock_mode_t *readMode();

#endif