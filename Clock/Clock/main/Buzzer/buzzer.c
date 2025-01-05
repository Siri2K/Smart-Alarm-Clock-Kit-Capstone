#include "buzzer.h"

void initializeBuzzer(buzzer_t *buzzer){
    // Configure Buzzer
    ESP_ERROR_CHECK(configureBuzzerTimer(buzzer)); // Timer
    ESP_ERROR_CHECK(configureBuzzerChannel(buzzer)); // Channel

    // Store Power variable
    buzzer->powerOn = setPowerOn;
}

esp_err_t configureBuzzerTimer(buzzer_t *buzzer){
    buzzer->buzzerTimerConfig.speed_mode = BUZZER_MODE;
    buzzer->buzzerTimerConfig.duty_resolution = BUZZER_DUTY_RES;
    buzzer->buzzerTimerConfig.timer_num = BUZZER_TIMER; 
    buzzer->buzzerTimerConfig.freq_hz = BUZZER_FREQUENCY; 
    buzzer->buzzerTimerConfig.clk_cfg = LEDC_AUTO_CLK;
    return ledc_timer_config(&(buzzer->buzzerTimerConfig));
}

esp_err_t configureBuzzerChannel(buzzer_t *buzzer){
    buzzer->buzzerChannelConfig.speed_mode = BUZZER_MODE;
    buzzer->buzzerChannelConfig.channel = BUZZER_CHANNEL;
    buzzer->buzzerChannelConfig.timer_sel = BUZZER_TIMER; 
    buzzer->buzzerChannelConfig.intr_type = LEDC_INTR_DISABLE; 
    buzzer->buzzerChannelConfig.gpio_num = BUZZER;
    buzzer->buzzerChannelConfig.duty = NONE; 
    buzzer->buzzerChannelConfig.hpoint = NONE;
    return ledc_timer_config(&(buzzer->buzzerChannelConfig));
}

void *setPowerOn(ledc_mode_t mode,ledc_channel_t channel, buzzer_power_t buzzerPower){
    // Determine Duty Cycle from Power
    uint32_t dutyCycle = (1 << BUZZER_DUTY_RES) - 1;
    dutyCycle *= buzzerPower;
    dutyCycle /= 100;
    
    // Set Duty to Power
    ESP_ERROR_CHECK(ledc_set_duty(mode, channel, dutyCycle));
    ESP_ERROR_CHECK(ledc_update_duty(mode, channel));
}