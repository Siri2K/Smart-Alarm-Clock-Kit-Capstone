#ifndef REAL_TIME_CLOCK_H
#define REAL_TIME_CLOCK_H

/* Headers */ 
/* ESP-IDF Libraries */
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <sdkconfig.h>

/* C Library */
#include <stdio.h>
#include <stdbool.h>

/* Definitions */
// RTC Configurations
#define I2C_TIMER  LEDC_TIMER_0 // TIMER
#define I2C_MASTER_NUM  I2C_NUM_0    // I2C port number for master dev 
#define I2C_MASTER_FREQ_HZ  100000     //I2C master clock frequency 
#define I2C_MASTER_TX_BUF_DISABLE   0   // I2C master doesn't need buffer 
#define I2C_MASTER_RX_BUF_DISABLE   0   // I2C master doesn't need buffer
#define I2C_MASTER_TIMEOUT_MS   1000 

#define RTC_ADDR    0x68     // DS1388 I2C address
#define RTC_REGISTER_SECONDS    0x01    // Seconds register
#define RTC_REGISTER_MINUTES    0x02    // Minutes register
#define RTC_REGISTER_HOURS  0x03    // Hours register

// Pins
#define RTC_MASTER_SDA_IO GPIO_NUM_18
#define RTC_MASTER_SCL_IO GPIO_NUM_5

/* Global Struct or Enums*/
typedef struct real_time_clock_t{
    // I2C Configuration
    i2c_config_t rtcConfig;

    // Tasks
    void *(*startTime)(uint8_t,uint8_t,uint8_t);
    uint8_t *(*currentTime)();
}real_time_clock_t;

extern void initializeRTC(real_time_clock_t *rtc);

extern esp_err_t configureRTC(real_time_clock_t *rtc);

extern esp_err_t i2c_write_register(uint8_t device_address, uint8_t register_address, uint8_t data);

extern esp_err_t i2c_read_register(uint8_t device_address, uint8_t register_address, uint8_t data);

extern void *setTime(uint8_t hours, uint8_t minutes, uint8_t seconds);

extern uint8_t *getTime();

#endif 