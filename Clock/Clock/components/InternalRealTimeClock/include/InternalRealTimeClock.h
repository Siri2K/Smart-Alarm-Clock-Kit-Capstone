#ifndef INTERNAL_REAL_TIME_CLOCK_H
#define INTERNAL_REAL_TIME_CLOCK_H

#include <stdio.h>
#include <time.h>
#include "esp_log.h"
#include "esp_system.h"

typedef struct internal_real_time_clock_t{
    void (*writeTime)(uint8_t,uint8_t,uint8_t, uint8_t);
    void (*readTime)(uint8_t*,uint8_t*,uint8_t*, uint8_t*);
}internal_real_time_clock_t;

extern void initializeRTC(internal_real_time_clock_t *rtc);

extern void writeTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t pm);

extern void readTime(uint8_t *hour, uint8_t *minute, uint8_t *second, uint8_t *pm);

#endif