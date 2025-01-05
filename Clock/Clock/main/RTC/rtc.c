#include "rtc.h"

static uint8_t DEC2BCD(uint8_t val){
    uint8_t bcd = 0;
    uint8_t factor = 0;

    while (val > 0) {
        bcd |= (val % 10) << (factor * 4);
        val /= 10;
        factor++;
    }
    return bcd;
}

static uint8_t BCD2DEC(uint8_t val){
    uint8_t dec = 0;
    uint8_t factor = 1;

    while (val > 0) {
        dec += (val & 0xF) << factor;
        val >>= 4;
        factor *= 10;
    }
    return dec;
}

esp_err_t i2c_operation(i2c_cmd_handle_t command, uint8_t address, bool rw, uint8_t reg){
    // Return value
    esp_err_t error = ESP_OK;

    // Go to correct register
    uint8_t operation = (address & 0xF0) | rw;
    error &= i2c_master_write_byte(command, operation, true);
    error &= i2c_master_write_byte(command, reg, true);

    return error;
}

void initializeRTC(rtc_t *rtc){
    // Configure I2C
    esp_err_t error = ESP_OK;
    ESP_ERROR_CHECK(configureRTC(rtc));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, rtc->rtcConfig.mode, 0, 0, 0));
}
    
esp_err_t configureRTC(rtc_t *rtc){
    rtc->rtcConfig.mode = I2C_MODE_MASTER;
    rtc->rtcConfig.sda_io_num  = RTC_MASTER_SDA_IO;
    rtc->rtcConfig.scl_io_num  = RTC_MASTER_SCL_IO;
    rtc->rtcConfig.sda_pullup_en  = GPIO_PULLUP_DISABLE;
    rtc->rtcConfig.scl_pullup_en  = GPIO_PULLUP_DISABLE;
    rtc->rtcConfig.master.clk_speed = I2C_MASTER_FREQ_HZ;
    return i2c_param_config(I2C_MASTER_NUM, &(rtc->rtcConfig));
}

void *setTime(uint8_t hours, uint8_t minutes, uint8_t seconds){
    // Set Time Array
    uint8_t* time = (uint8_t *)malloc(3*sizeof(uint8_t));
    time[0] = DEC2BCD(seconds);
    time[1] = DEC2BCD(minutes);
    time[2] = DEC2BCD(hours);

    // Write to RTC
    i2c_cmd_handle_t command = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(command));
    ESP_ERROR_CHECK(i2c_operation(command, RTC_ADDR, I2C_MASTER_WRITE, RTC_REGISTER_SECONDS));
    ESP_ERROR_CHECK(i2c_master_write_byte(command, time[0], RTC_REGISTER_SECONDS));
    ESP_ERROR_CHECK(i2c_master_write_byte(command, time[1], RTC_REGISTER_MINUTES));
    ESP_ERROR_CHECK(i2c_master_write_byte(command, time[2], RTC_REGISTER_HOURS));
    ESP_ERROR_CHECK(i2c_master_stop(command));

    // Clear Data and Command
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, command, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(command);
    free(time);
}

uint8_t *getTime(){
    // Set Time Array
    uint8_t* time = (uint8_t *)malloc(3*sizeof(uint8_t));

    // Write to RTC
    i2c_cmd_handle_t command = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(command));
    ESP_ERROR_CHECK(i2c_operation(command, RTC_ADDR, I2C_MASTER_READ, RTC_REGISTER_SECONDS));
    i2c_master_start(command);
    ESP_ERROR_CHECK(i2c_master_read_byte(command, time[0], RTC_REGISTER_SECONDS));
    ESP_ERROR_CHECK(i2c_master_read_byte(command, time[1], RTC_REGISTER_MINUTES));
    ESP_ERROR_CHECK(i2c_master_read_byte(command, time[2], RTC_REGISTER_HOURS));
    ESP_ERROR_CHECK(i2c_master_stop(command));

    // Clear Data and Command
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, command, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(command);
    free(time);
}