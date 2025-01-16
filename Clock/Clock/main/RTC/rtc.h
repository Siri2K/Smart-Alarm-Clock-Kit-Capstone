#ifndef RTC_H
#define RTC_H

/* Headers */ 
#include "Pins/pins.h"
#include "FreeRTOS_Driver/freeRTOS.h"

/* ESP-IDF Libraries */
#include "driver/i2c.h"
#include "sdkconfig.h"

/* C Library */
#include <stdio.h>
#include <stdbool.h>

/* Definitions */
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

/* Global Struct or Enums*/
typedef struct rtc_t{
    // I2C Configuration
    i2c_config_t rtcConfig;

    // Tasks
    uint8_t *(*readRegister)(uint8_t);
    void *(*startTime)(uint8_t,uint8_t,uint8_t);
    uint8_t *(*currentTime)();
}rtc_t;

void initializeRTC(rtc_t *rtc);

esp_err_t configureRTC(rtc_t *rtc);

esp_err_t i2c_operation(i2c_cmd_handle_t command, uint8_t address, bool rw, uint8_t reg);

void *setTime(uint8_t hours, uint8_t minutes, uint8_t seconds);

uint8_t *getTime();

#endif 