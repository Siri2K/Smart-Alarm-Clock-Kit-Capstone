#include "InternalRealTimeClock.h"


void initializeRTC(internal_real_time_clock_t *rtc){
    writeTime(12,0,0,0);
}

void *writeTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t pm){
    struct tm timeinfo = { 0 };
    timeinfo.tm_hour = (pm == 1)?hour+12:hour;           // 12 PM
    timeinfo.tm_min = minute;             // 00 minutes
    timeinfo.tm_sec = second;             // 00 seconds

    // Set the time using mktime, which will set the system time
    time_t t = mktime(&timeinfo);
    struct timeval tv = { .tv_sec = t, .tv_usec = 0 };
    
    return NULL;
}

void *readTime(uint8_t *hour, uint8_t *minute, uint8_t *second, uint8_t *pm){
    time_t now;
    struct tm timeinfo;

    // Get the current time
    time(&now);
    localtime_r(&now, &timeinfo);

    // Store Hours
    *pm = (uint8_t)((timeinfo.tm_hour > 12)?1:0);
    *hour = (uint8_t)((timeinfo.tm_hour > 12)?timeinfo.tm_hour-12:timeinfo.tm_hour);
    *minute = (uint8_t)timeinfo.tm_min;
    *second = (uint8_t)timeinfo.tm_sec;


    return NULL;
}