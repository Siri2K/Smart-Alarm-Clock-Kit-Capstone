#include "button.h"

static const struct gpio_dt_spec buttonNode = GPIO_DT_SPEC_GET_OR(BUTTON_NODE, gpios,{0});

uint8_t initializeButton(){
    // Initialize GPIO
    uint8_t status = gpio_is_ready_dt(&buttonNode) && gpio_pin_configure_dt(&buttonNode,GPIO_INPUT) && \
    gpio_pin_interrupt_configure_dt(&buttonNode,GPIO_INT_EDGE_TO_ACTIVE);

    return status; // 0 = true, 1 = Error
}

uint8_t pressed(){
    return gpio_pin_get_dt(&buttonNode);
}

int64_t calculatePressTime(){
    // Time variables
    int64_t buttonPressedStartTime = 0;
    int64_t duration = 0;

    // Lock task until Button is released
    while(pressed() == 0){
        if(buttonPressedStartTime == 0){
            buttonPressedStartTime = k_uptime_get(); // Record Current Time
        }
        else{
            duration = k_uptime_get() - buttonPressedStartTime;
        }
    }
    return duration;
}