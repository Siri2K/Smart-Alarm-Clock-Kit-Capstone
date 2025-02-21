#include "Button.h"

int64_t buttonPressedStartTime = 0;


void initializeButton(button_t *button, gpio_num_t buttonPin){
    // Check If Button is empty
    if(button == NULL){
        return; // Cut the Code if button is empty
    }
    // Configure Buttons
    ESP_ERROR_CHECK(configureButton(button,buttonPin));

    // Store Functions into struct
    button->pressed = getPressState;
    button->pressDuration = calculatePressDuration;

    // Store pin Numbers
    button->gpioPin = buttonPin;
}

esp_err_t configureButton(button_t *button, gpio_num_t buttonPin){
    // Return Value
    esp_err_t status = ESP_OK;

    // Check If Button is empty
    if(button == NULL){
        return -1; // Cut the Code if button is empty
    }
    
    // Setup Pin Masks
    button->buttonConfig.pin_bit_mask = (1ULL << buttonPin);

    // Configure Each Pin
    button->buttonConfig.mode = GPIO_MODE_INPUT; // Set to Input Mode
    button->buttonConfig.pull_up_en = GPIO_PULLUP_DISABLE; // Set to Input Mode
    button->buttonConfig.pull_down_en = GPIO_PULLDOWN_DISABLE; // Set to Input Mode
    button->buttonConfig.intr_type = GPIO_INTR_DISABLE; // Set to Input Mode

    // Configure GPIO
    status = gpio_config(&button->buttonConfig);

    // cleanup and return
    return status;
}

uint8_t *getPressState(gpio_num_t buttonPin){
    static uint8_t state;
    state = (uint8_t)(gpio_get_level(buttonPin));
    return &state;
}

int64_t *calculatePressDuration(gpio_num_t buttonPin){
    // Lock task until Button is released
    static int64_t duration;
    duration = 0;

    while(!(*(getPressState(buttonPin) + 0))){
        if(buttonPressedStartTime == 0){ // Notify that button has been pressed
            buttonPressedStartTime = esp_timer_get_time(); // Record Current Time
        }
        else{
            // Update Duration
            duration = esp_timer_get_time() - buttonPressedStartTime;
        }
    }
    
    // Reset Start Time on Release
    if(*(getPressState(buttonPin)+0)){
        buttonPressedStartTime = 0;
    }

    return &duration;
}
