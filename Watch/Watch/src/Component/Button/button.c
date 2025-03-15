#include "button.h"

static const struct gpio_dt_spec buttonNode = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

int8_t initializeButton(){
    // Initialize GPIO
    int8_t status = 0;

    // Get Button Node Ready
    status = (int8_t)gpio_is_ready_dt(&buttonNode);
    if(status != 1){
        return status;
    }

    status = (int8_t)gpio_pin_configure_dt(&buttonNode,GPIO_INPUT);
    if(status != 0){
        return status;
    }

    status = (int8_t)gpio_pin_interrupt_configure_dt(&buttonNode,GPIO_INT_EDGE_TO_INACTIVE);
    if(status != 0){
        return status;
    }

    return 0; // 0 = true, 1 = Error
}

int8_t pressed(){
    return gpio_pin_get_dt(&buttonNode);
}


int64_t calculatePressTime(){
    // Time variables
    int64_t buttonPressedStartTime = 0;
    int64_t duration = 0;

    // Button not Pressed
    while(pressed() == 0){
        k_msleep(50);
    }

    // Lock task until Button is released
    while(pressed() != 0){
        if(buttonPressedStartTime == 0){
            buttonPressedStartTime = k_uptime_get(); // Record Current Time
        }
        else{
            duration = k_uptime_get() - buttonPressedStartTime;
        }
    }

    // Ensure a non-zero duration is returned
    if (duration == 0) {
        duration = k_uptime_get() - buttonPressedStartTime;
    }
    k_msleep(50);

    return duration;
}
