#ifndef SLIDE_SWITCH_H
#define SLIDE_SWITCH_H

/* Headers */ 
#include "Pins/pins.h"

/* C Library */
#include <stdio.h>
#include <stdbool.h>


/* Global variable */
uint8_t mode = 0;

/* Global Struct or Enums*/
typedef enum {
    CLOCK = 1,
    TIME = 2,
    ALARM = 3
} clock_mode_t;

typedef struct slide_switch {
    // Data
    gpio_config_t sliderConfig;
    
    // Task
    volatile bool *(onClockMode);
    volatile bool *(onTimeMode);
    volatile bool *(onAlarmMode);
    volatile uint8_t *(*mode)(bool,bool, bool);
    
} slide_switch;

/* Tasks */
void initializeSlideSwitch(slide_switch *slideSwitch);

esp_err_t configureSlideSwitch(slide_switch *slideSwitch);

volatile bool *getClockState();

volatile bool *getAlarmState();

volatile bool *getTimeState();

volatile uint8_t *readMode(bool clockOn, bool alarmOn, bool timeOn);

#endif