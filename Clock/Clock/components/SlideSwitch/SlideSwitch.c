#include "SlideSwitch.h"
#include <string.h>  // Needed for memset

static const char *TAG = "SlideSwitch Log ";

static esp_err_t configureSlideSwitch(){
    // Return Value
    esp_err_t status = ESP_OK;
    gpio_config_t io_conf;

    // Configure IO pins
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << TIME_MODE_SWITCH) | (1ULL << CLOCK_MODE_SWITCH) | (1ULL << ALARM_MODE_SWITCH);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    
    status = gpio_config(&io_conf);
    if(status != ESP_OK){
        ESP_LOGE(TAG, "Slide Switch GPIO Not Configured");
        return status;
    }

    return ESP_OK;
}


void initializeSlideSwitch(slide_switch_t *slideSwitch){
    esp_err_t status = ESP_OK;
    
    // Configure Slide Switch
    status = configureSlideSwitch();
    if(status != ESP_OK){
        ESP_LOGE(TAG,"SlideSwitch Failed to Initialize, error : %d",status);
    }

    // Store Functions into struct
    slideSwitch->mode = readMode;

}

clock_mode_t readMode(){
    // Intialize Clock Mode & Variables
    static clock_mode_t clock_mode;

    // Save mode Data
    int8_t clockMode[3] = {
        (int8_t)gpio_get_level(CLOCK_MODE_SWITCH),
        (int8_t)gpio_get_level(TIME_MODE_SWITCH),
        (int8_t)gpio_get_level(ALARM_MODE_SWITCH)
    };

    // Print Each State
    ESP_LOGI(TAG,"SlideSwitch Clock State : %d",clockMode[0]);
    ESP_LOGI(TAG,"SlideSwitch Alarm State : %d",clockMode[1]);
    ESP_LOGI(TAG,"SlideSwitch Time State : %d",clockMode[2]);

    if(clockMode[0] == 0){
        clock_mode = CLOCK;
    }
    else if(clockMode[1] == 0){
        clock_mode = ALARM;
    }
    else if(clockMode[2] == 0){
        clock_mode = TIME;
    }
    else{
        clock_mode = ERROR;
    }
    // Find Mode
    ESP_LOGI(TAG,"SlideSwitch Mode is %d", clock_mode);
    return clock_mode;
}
