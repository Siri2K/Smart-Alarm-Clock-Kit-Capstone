#ifndef REAL_TIME_CLOCK_H
#define REAL_TIME_CLOCK_H

/* Driver Header */
#include "driver/i2c.h"
#include "driver/gpio.h"

/* C++ Header */
#include <string.h>


/* Defnitions */
// I2C Configuration
#define RTC_SCL GPIO_NUM_5
#define RTC_SDA GPIO_NUM_18
#define RTC_CLK_FREQ 100000

// RTC
#define RTC_ADDRESS 0x68
#define RTC_REGISTER_SECONDS 0x01   
#define RTC_REGISTER_MINUTES 0x02    
#define RTC_REGISTER_HOURS 0x03

typedef struct {
    // I2C Configuration
    i2c_config_t rtcConfig;

    // Functions 
    void *(*writeTime)(uint8_t,uint8_t,uint8_t, uint8_t);
    void *(*readTime)(uint8_t*,uint8_t*,uint8_t*, uint8_t*);
}real_time_clock_t;

extern void initializeRTC(real_time_clock_t *rtc);

extern void *writeToRTC(uint8_t* timeArray, uint8_t length);

extern void *writeTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t pm);

extern void *readTime(uint8_t *hour, uint8_t *minute, uint8_t *second, uint8_t *pm);

#endif 

