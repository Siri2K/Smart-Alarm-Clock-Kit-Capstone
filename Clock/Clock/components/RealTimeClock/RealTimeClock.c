#include "RealTimeClock.h"

void initializeRTC(real_time_clock_t *rtc){
    // Configure RTC
    rtc->rtcConfig.mode = I2C_MODE_MASTER;
    rtc->rtcConfig.sda_io_num  = RTC_SDA;
    rtc->rtcConfig.scl_io_num  = RTC_SCL;
    rtc->rtcConfig.sda_pullup_en  = GPIO_PULLUP_DISABLE;
    rtc->rtcConfig.scl_pullup_en  = GPIO_PULLUP_DISABLE;
    rtc->rtcConfig.master.clk_speed = RTC_CLK_FREQ;

    i2c_param_config(I2C_NUM_0, &rtc->rtcConfig);
    i2c_driver_install(I2C_NUM_0,I2C_MODE_MASTER,0,0,0);
}

void I2CWriteRegister(uint8_t reg, uint8_t value){
    // Setup Command
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Setup Command to Device 
    /*
        Process : RTC Write Address -> Register -> Data
    */
    i2c_master_start(cmd);
    i2c_master_write_byte(
        cmd,
        (RTC_ADDRESS << 1) | I2C_MASTER_WRITE, // RTC Write Address
        true
    ); 
    i2c_master_write_byte(cmd, reg, true); // Send register address
    i2c_master_write_byte(cmd, value, true); // Send Data to Write
    i2c_master_stop(cmd);

    // Write to Device & Free Command
    i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

}

uint8_t I2CReadRegister(uint8_t reg){
    // Return Value
    uint8_t value;
    
    // Setup Command
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Setup Command to Device 
    /*
        Process : 
        1) Write First : RTC Read Address -> Register
        2) Read : RTC Read Address -> Register -> Data
    */
    i2c_master_start(cmd);
    i2c_master_write_byte(
        cmd,
        (RTC_ADDRESS << 1) | I2C_MASTER_WRITE, // RTC Write Address
        true
    ); 
    i2c_master_write_byte(cmd, reg, true); // Send register address

    i2c_master_start(cmd);
    i2c_master_write_byte(
        cmd,
        (RTC_ADDRESS << 1) | I2C_MASTER_WRITE, // RTC Write Address
        true
    ); 
    i2c_master_read_byte(cmd, &value, I2C_MASTER_NACK); // Read Data from Register
    i2c_master_stop(cmd);

    // Execute and Free Command
    i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    return value;
}

void *writeTime(uint8_t hour, uint8_t minute, uint8_t second, bool pm){
    uint8_t am_or_pm = 0x20; // 0010 0000
    
    I2CWriteRegister(RTC_REGISTER_SECONDS, second); // Seconds
    I2CWriteRegister(RTC_REGISTER_MINUTES, minute); // Minutes
    I2CWriteRegister(
        RTC_REGISTER_HOURS, 
        (pm == true)? hour|am_or_pm : hour&am_or_pm
    ); // Hours


    return NULL;
}

void *readTime(uint8_t *hour, uint8_t *minute, uint8_t *second, bool *pm){
    *second = I2CReadRegister(RTC_REGISTER_SECONDS);
    *minute = I2CReadRegister(RTC_REGISTER_MINUTES);
    *hour = I2CReadRegister(RTC_REGISTER_HOURS); 

    // Check for pm
    *pm = ((*hour & 0x20) >> 5) ? true : false;
    *hour &= 0x1F; // 0001 1111

    return NULL;
}