#include "SlideSwitch.h"
#include <string.h>  // Needed for memset

void initializeSlideSwitch(slide_switch_t *slideSwitch){
    
    // Configure Slide Switch
    ESP_ERROR_CHECK(configureSlideSwitch(slideSwitch));

    // Store Functions into struct
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

static int8_t getClockState(){
    return gpio_get_level(CLOCK_MODE_SWITCH);
}

static int8_t getAlarmState(){
    return gpio_get_level(ALARM_MODE_SWITCH);
}

static int8_t getTimeState(){
    return gpio_get_level(TIME_MODE_SWITCH);
}

clock_mode_t *readMode(){
    // Intialize Clock Mode & Variables
    static clock_mode_t clock_mode;
    static uint8_t clockOperationMode[3];
    clockOperationMode[0] = getClockState();
    clockOperationMode[1] = getAlarmState();
    clockOperationMode[2] = getTimeState();

    // Find Mode
    if(!clockOperationMode[0] ^ !clockOperationMode[1] ^ !clockOperationMode[2]){ // Check if only 1 mode is on
        if(!clockOperationMode[0]){
            clock_mode = CLOCK;
        }
        if (!clockOperationMode[1]){
            clock_mode = ALARM;
        }
        if(clockOperationMode[2]){
            clock_mode = TIME;
        }
    }
    else{
        clock_mode = ERROR;
    }

    return &clock_mode;
}
