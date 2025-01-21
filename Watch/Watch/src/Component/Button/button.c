#include "button.h"

const struct gpio_dt_spec buttonNode = GPIO_DT_SPEC_GET_OR(BUTTON_NODE, gpios,{0});

int initializeButton(){
    // Status Check
    int status = 1;
    
    // Check if Button is ready and Configure
    status &= gpio_is_ready_dt(&buttonNode);
    status &= gpio_pin_configure_dt(&buttonNode,GPIO_INPUT);
    
    // Intialize Interrupt
    status &= gpio_pin_interrupt_configure_dt(&buttonNode,GPIO_INT_EDGE_TO_ACTIVE);

    return status;
}

volatile int pressed(){
    return !gpio_pin_get_dt(&buttonNode);
}

volatile int64_t calculatePressTime(){
    // Time variables
    int64_t buttonPressedStartTime = 0;
    int64_t duration = 0;

    // Lock task until Button is released
    while(pressed() != 0){
        if(buttonPressedStartTime == 0){
            buttonPressedStartTime = k_uptime_get(); // Record Current Time
        }
        else{
            duration = k_uptime_get() - buttonPressedStartTime;
        }
    }

    // Reset Start Time on Release
    if(pressed() > 0){
        buttonPressedStartTime = 0;
    }

    return duration;
}