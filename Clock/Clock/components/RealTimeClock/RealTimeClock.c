#include "RealTimeClock.h"

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

esp_err_t i2c_write_register(uint8_t device_address, uint8_t register_address, uint8_t data) {
    // Return value
    esp_err_t error = ESP_OK;
    
    // First, write the register address
    error = i2c_master_write_to_device(
        I2C_MASTER_NUM, 
        device_address, 
        &register_address, 
        1, 
        1000 / portTICK_PERIOD_MS
    );
    if (error != ESP_OK) {
        return error;  // Return if writing the register address fails
    }
    
    // Read from Register
    error = i2c_master_read_from_device(
        I2C_MASTER_NUM, 
        device_address, 
        &data, 
        1, 
        1000 / portTICK_PERIOD_MS
    );

    return error;
}

esp_err_t i2c_read_register(uint8_t device_address, uint8_t register_address, uint8_t data) {
    // Return value
    esp_err_t error = ESP_OK;
    
    uint8_t *data_buffer = (uint8_t*)malloc(2*sizeof(uint8_t));
    data_buffer[0] = register_address;  // Register address
    data_buffer[1] = data;              // Data to write

    // Write register address and data to the device
    error &= i2c_master_write_to_device(I2C_MASTER_NUM, device_address, data_buffer, sizeof(data_buffer), 1000 / portTICK_PERIOD_MS);
    
    return error;
}

void initializeRTC(real_time_clock_t *rtc){
    // Configure I2C
    ESP_ERROR_CHECK(configureRTC(rtc));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, rtc->rtcConfig.mode, 0, 0, 0));

    // Store Power variable
    rtc->startTime = setTime;
    rtc->currentTime = getTime;
}
    
esp_err_t configureRTC(real_time_clock_t *rtc){
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
    ESP_ERROR_CHECK(i2c_write_register(RTC_ADDR,RTC_REGISTER_SECONDS,time[0])); // Second
    ESP_ERROR_CHECK(i2c_write_register(RTC_ADDR,RTC_REGISTER_MINUTES,time[1])); // Minute
    ESP_ERROR_CHECK(i2c_write_register(RTC_ADDR,RTC_REGISTER_HOURS,time[2])); // Hours
    
    // Clean And Return
    free(time);
    return NULL;
}

uint8_t *getTime(){
    // Set Time Array
    uint8_t* time = (uint8_t *)malloc(3*sizeof(uint8_t));
    time[0] = 0; time[1] = 0; time[2] = 0;

    // Read to RTC
    ESP_ERROR_CHECK(i2c_read_register(RTC_ADDR,RTC_REGISTER_SECONDS,time[0])); // Second
    ESP_ERROR_CHECK(i2c_read_register(RTC_ADDR,RTC_REGISTER_MINUTES,time[1])); // Minute
    ESP_ERROR_CHECK(i2c_read_register(RTC_ADDR,RTC_REGISTER_HOURS,time[2])); // Hours

    // Convert to DEC
    time[0] = BCD2DEC(time[0]);
    time[1] = BCD2DEC(time[1]);
    time[2] = BCD2DEC(time[2]);
    return time;
}