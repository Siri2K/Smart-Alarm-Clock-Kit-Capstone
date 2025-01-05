#include "button.h"

void initializeButtons(button_t **buttons){
    // Initialize Button Pointer
    *buttons = (button_t *)malloc(BUTTON_COUNT*sizeof(button_t)); // 3 buttons : UP,DOWN,SELECT

    // Configure Buttons
    ESP_ERROR_CHECK(configureSlideSwitch((*buttons)));

    // Store Functions into struct
    int i;
    for(i = 0; i<BUTTON_COUNT;i++){
        (*buttons)[i].pressed = getPressState;
        (*buttons)[i].pressDuration = calculatePressDuration;
    }
}

esp_err_t configureButtons(button_t *buttons){
    // Return Value
    esp_err_t status = ESP_OK;
    
    // Setup Pin Masks
    buttons[0].buttonConfig.pin_bit_mask = (1ULL << UP_BUTTON); // Up Button
    buttons[1].buttonConfig.pin_bit_mask = (1ULL << DOWN_BUTTON); // Down Button
    buttons[2].buttonConfig.pin_bit_mask = (1ULL << SELECT_BUTTON); // Select Button

    // Configure Each Pin
    int i;
    for(i = 0; i<BUTTON_COUNT; i++){
        // Configure Pin Mode
        buttons[i].buttonConfig.mode = GPIO_MODE_INPUT; // Set to Input Mode

        // Disable Pull-up, Pull-down and Interrupts
        buttons[i].buttonConfig.pull_up_en = GPIO_PULLUP_DISABLE; // Disable Built-in Pull-up
        buttons[i].buttonConfig.pull_down_en  = GPIO_PULLDOWN_DISABLE; // Disable Built-in Pull-down
        buttons[i].buttonConfig.intr_type  = GPIO_INTR_DISABLE; // Disable Built-in Interrupt

        // Configure GPIO
        status = gpio_config(&(buttons[i].buttonConfig));
    }

    // cleanup and return
    return status;
}

volatile bool *getPressState(gpio_num_t *buttonPin){
    return !gpio_get_level(buttonPin);
}

volatile int64_t *calculatePressDuration(gpio_num_t *buttonPin){
    // Lock task until Button is released
    int64_t duration;
    while(getPressState(buttonPin)){
        if(buttonPressedStartTime == 0){ // Notify that button has been pressed
            buttonPressedStartTime = esp_timer_get_time(); // Record Current Time
        }
        else{
            // Update Duration
            duration = esp_timer_get_time() - buttonPressedStartTime;
        }
    }
    
    // Reset Start Time on Release
    if(getPressState(buttonPin)){
        buttonPressedStartTime = 0;
    }

    return duration;
}