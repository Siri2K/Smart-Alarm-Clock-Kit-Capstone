#include "Buzzer.h"

static const char *TAG = "Buzzer Log ";

static esp_err_t configureBuzzerTimer(){
    esp_err_t status;
    ledc_timer_config_t buzzerTimerConfig; // Timer

    // Configure Buzzer Timer
    buzzerTimerConfig.speed_mode = BUZZER_MODE;
    buzzerTimerConfig.duty_resolution = BUZZER_DUTY_RES;
    buzzerTimerConfig.timer_num = BUZZER_TIMER;
    buzzerTimerConfig.freq_hz = BUZZER_FREQUENCY;
    buzzerTimerConfig.clk_cfg = LEDC_AUTO_CLK;

    status = ledc_timer_config(&buzzerTimerConfig);
    if(status != ESP_OK){
        ESP_LOGE(TAG, "Failed To configure Buzzer Timer, err:%d",(int8_t)status);
        return status;
    }
    return ESP_OK;
}

static esp_err_t configureBuzzerChannel(){
    esp_err_t status;
    ledc_channel_config_t buzzerChannelConfig; // PWM Channel

    buzzerChannelConfig.speed_mode = BUZZER_MODE;
    buzzerChannelConfig.channel = BUZZER_CHANNEL;
    buzzerChannelConfig.timer_sel = BUZZER_TIMER; 
    buzzerChannelConfig.intr_type = LEDC_INTR_DISABLE; 
    buzzerChannelConfig.gpio_num = BUZZER;
    buzzerChannelConfig.duty = NONE; 
    buzzerChannelConfig.hpoint = NONE;

    status = ledc_channel_config(&buzzerChannelConfig);
    if(status != ESP_OK){
        ESP_LOGE(TAG, "Failed To configure Buzzer Channel, err:%d",(int8_t)status);
        return status;
    }
    return ESP_OK;
}

void initializeBuzzer(buzzer_t *buzzer){
    // Configure Buzzer
    esp_err_t status;

    status = configureBuzzerTimer();
    if(status != ESP_OK){
        ESP_LOGE(TAG, "Failed To configure Buzzer Timer");
        return;
    }

    status = configureBuzzerChannel();
    if(status != ESP_OK){
        ESP_LOGE(TAG, "Failed To configure Buzzer Channel");
        return;
    }

    // Store Power variable
    buzzer->powerOn = setPowerOn;
}

void setPowerOn(ledc_mode_t mode,ledc_channel_t channel, buzzer_power_t buzzerPower){
    // Determine Duty Cycle from Power
    esp_err_t status;
    uint32_t dutyCycle = (1 << BUZZER_DUTY_RES) - 1;
    dutyCycle *= buzzerPower;
    dutyCycle /= 100;
    
    // Set Duty to Power
    status = ledc_set_duty(mode, channel, dutyCycle);
    if(status != ESP_OK){
        ESP_LOGE(TAG, "Failed To Set Buzzer Cycle, err : %d", (int8_t)status);
        return;
    }

    status = ledc_update_duty(mode,channel);
    if(status != ESP_OK){
        ESP_LOGE(TAG, "Failed To Update Buzzer Cycle, err : %d", (int8_t)status);
        return;
    }
    vTaskDelay(100/portTICK_PERIOD_MS);

    status = ledc_stop(mode,channel,NONE);
    if(status != ESP_OK){
        ESP_LOGE(TAG, "Failed To Stop Buzzer Cycle, err : %d", (int8_t)status);
        return;
    }
    
}