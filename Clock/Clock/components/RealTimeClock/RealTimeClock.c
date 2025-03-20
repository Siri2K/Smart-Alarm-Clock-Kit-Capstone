#include "RealTimeClock.h"
#include <string.h>  // For memset

void initializeRTC(real_time_clock_t *rtc){
    
    memset(&rtc->rtcConfig, 0, sizeof(rtc->rtcConfig));
    
    // Configure RTC
    rtc->rtcConfig.mode = I2C_MODE_MASTER;
    rtc->rtcConfig.sda_io_num  = RTC_SDA;
    rtc->rtcConfig.scl_io_num  = RTC_SCL;
    rtc->rtcConfig.sda_pullup_en  = GPIO_PULLUP_DISABLE;
    rtc->rtcConfig.scl_pullup_en  = GPIO_PULLUP_DISABLE;
    rtc->rtcConfig.master.clk_speed = RTC_CLK_FREQ;
    rtc->rtcConfig.clk_flags = 0;

    i2c_param_config(I2C_NUM_0, &rtc->rtcConfig);
    i2c_driver_install(I2C_NUM_0,I2C_MODE_MASTER,0,0,0);
}

void *writeToRTC(uint8_t* timeArray, uint8_t length){
    // Create Time Buffer
    uint8_t writeBuffer[length+1];

    // Place Starting Register
    writeBuffer[0] = RTC_REGISTER_SECONDS; // Start at the Second Register
    memcpy(&writeBuffer[1], timeArray, length);

    // Write to Device
    ESP_ERROR_CHECK(
        i2c_master_write_to_device(
            I2C_NUM_0,
            RTC_ADDRESS,
            writeBuffer,
            length+1,
            pdMS_TO_TICKS(1000)
        )
    );


    return NULL;
}

void *readToRTC(uint8_t* timeArray, uint8_t length){
    // Place Starting Register
    uint8_t startRegister = RTC_REGISTER_SECONDS; // Start at the Second Register

    // Go to Device
    ESP_ERROR_CHECK(
        i2c_master_write_to_device(
            I2C_NUM_0,
            RTC_ADDRESS,
            &startRegister,
            1,
            pdMS_TO_TICKS(1000)
        )
    );

    // Read to Device
    ESP_ERROR_CHECK(
        i2c_master_read_from_device(
            I2C_NUM_0,
            RTC_ADDRESS,
            timeArray,
            length,
            pdMS_TO_TICKS(1000)
        )
    );

    return NULL;
}

void *writeTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t pm){
    // Set Hour
    hour &= 0x0F; // 0000 1111 --> Set Hour
    hour |= 0x40; // 0100 0000 --> Make sure it is 12 hour format
    hour |= (pm == 1)?0x20:0x00; // 0010 0000 --> Check for AM or PM

    // Write to Array
    uint8_t timeArray[] = {second,minute,hour};
    writeToRTC(timeArray, 3);

    return NULL;
}

void *readTime(uint8_t *hour, uint8_t *minute, uint8_t *second, uint8_t *pm){
    // Read from Array
    uint8_t timeArray[3] = {0};
    readToRTC(timeArray,3);

    // Set Seconds and Minutes
    *second = timeArray[0];
    *minute = timeArray[1];
    *hour = timeArray[2] & 0x0F;
    *pm = (timeArray[2] & 0x20) >> 5;

    return NULL;
}