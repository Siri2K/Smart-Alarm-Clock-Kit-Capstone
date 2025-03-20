#include "Button.h"

static const char *TAG = "Button Log ";

static esp_err_t configureButton(){
    // Return Value
    esp_err_t status = ESP_OK;
    gpio_config_t io_conf;

    // Configure IO pins
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << UP_BUTTON) | (1ULL << DOWN_BUTTON) | (1ULL << SELECT_BUTTON);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    
    status = gpio_config(&io_conf);
    if(status != ESP_OK){
        ESP_LOGE(TAG, "Button Switch GPIO Not Configured");
        return status;
    }

    return ESP_OK;
}

void initializeButton(button_t *button){
    esp_err_t status = ESP_OK;
    
    // Configure Slide Switch
    status = configureButton();
    if(status != ESP_OK){
        ESP_LOGE(TAG,"Buttons Failed to Initialize, error : %d",status);
    }

    // Store Functions into struct
    button->pressed = getPressState;
    button->pressDuration = calculatePressDuration;

}

int8_t getPressState(gpio_num_t buttonPin){
    return (int8_t)(gpio_get_level(buttonPin));
}

int64_t calculatePressDuration(gpio_num_t buttonPin){
    // Time variables
    int64_t buttonPressedStartTime = 0;
    int64_t duration = 0;
    
    // Button not Pressed
    while(getPressState(buttonPin) != 0){
        esp_rom_delay_us(50000);
    }

    // Lock Task Until Buttons are released
    while(getPressState(buttonPin) == 0){
        if(buttonPressedStartTime == 0){ // Notify that button has been pressed
            buttonPressedStartTime = esp_timer_get_time(); // Record Current Time
        }
        else{
            // Update Duration
            duration = esp_timer_get_time() - buttonPressedStartTime;
        }
    }
    
    // Ensure a non-zero duration is returned
    if (duration == 0) {
        duration = esp_timer_get_time() - buttonPressedStartTime;
    }
    esp_rom_delay_us(50000);

    return duration;
}
