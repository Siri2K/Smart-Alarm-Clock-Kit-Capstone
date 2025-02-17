#include "SlideSwitch.h"
#include <string.h>  // Needed for memset

void initializeSlideSwitch(slide_switch_t *slideSwitch){
    
    // Configure Slide Switch
    ESP_ERROR_CHECK(configureSlideSwitch(slideSwitch));

    // Store Functions into struct
    slideSwitch->onClockMode = getClockState;
    slideSwitch->onAlarmMode = getAlarmState;
    slideSwitch->onTimeMode = getTimeState;
    slideSwitch->mode = readMode;

}

esp_err_t configureSlideSwitch(slide_switch_t *slideSwitch){
    memset(&(slideSwitch->sliderConfig), 0, sizeof(gpio_config_t));

    // Setup Pins used for Slide Switch
    slideSwitch->sliderConfig.pin_bit_mask = 
    (1ULL << TIME_MODE_SWITCH) | 
    (1ULL << CLOCK_MODE_SWITCH) | 
    (1ULL << ALARM_MODE_SWITCH);

    // Configure Pin Mode
    slideSwitch->sliderConfig.mode = GPIO_MODE_INPUT; // Set to Input Mode

    // Disable Pull-up, Pull-down and Interrupts
    slideSwitch->sliderConfig.pull_up_en = GPIO_PULLUP_DISABLE; // Disable Built-in Pull-up
    slideSwitch->sliderConfig.pull_down_en  = GPIO_PULLDOWN_DISABLE; // Disable Built-in Pull-down
    slideSwitch->sliderConfig.intr_type  = GPIO_INTR_DISABLE; // Disable Built-in Interrupt

    // Configure GPIO
    return gpio_config(&(slideSwitch->sliderConfig));
}

volatile bool *getClockState(){
    bool *state = (bool*)malloc(sizeof(bool));
    *state = !!(gpio_get_level(CLOCK_MODE_SWITCH));
    return state;
}

volatile bool *getAlarmState(){
    bool *state = (bool*)malloc(sizeof(bool));
    *state = !!(gpio_get_level(ALARM_MODE_SWITCH));
    return state;
}

volatile bool *getTimeState(){
    bool *state = (bool*)malloc(sizeof(bool));
    *state = !!(gpio_get_level(TIME_MODE_SWITCH));
    return state;
}

volatile clock_mode_t *readMode(bool clockOn, bool alarmOn, bool timeOn){
    clock_mode_t *clock_mode = (clock_mode_t*)malloc(sizeof(clock_mode_t));
    *clock_mode = ERROR;

    // Find Mode
    if(!clockOn ^ !alarmOn ^ !timeOn){ // Check if only 1 mode is on
        if(!clockOn){
            *clock_mode = CLOCK;
        }
        if (!alarmOn){
            *clock_mode = ALARM;
        }
        if(!timeOn){
            *clock_mode = TIME;
        }
    }
    else{
        *clock_mode = ERROR;
    }
    return clock_mode;
}
