#ifndef BATTERY_H
#define BATTERY_H

/* Zephyr library */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>

/* C Library */
#include <stdio.h>
#include <stdbool.h>

// Battery device
#define BATTERY_NODE DT_NODELABEL(adc)

#define ADC_RESOLUTION 8
#define ADC_CHANNEL 4
#define ADC_PORT SAADC_CH_PSELN_PSELN_AnalogInput4 // AIN4
#define ADC_REFERENCE ADC_REF_INTERNAL // Use 0.6V
#define ADC_GAIN ADC_GAIN_1 // Vref = ADC_REFERENCE * 1

// Rated Voltage
#define BATTERY_RATED_VOLTAGE_IN_MV 0.7

// Global objects and structures
typedef enum battery_charge_level_t{
    BATTERY_MIN = 15, // 10% before full discharge
    BATTERY_LOW = 25, // 25% before full discharge
    BATTERY_MID = 50, // 50% before full discharge
    BATTERY_HIGH = 75, // 75% before full discharge
    BATTERY_MAX = 100, // 75% before full discharge
}battery_charge_level_t;

extern const struct device *batteryDevice;
extern struct adc_channel_cfg batteryChannel;
extern struct adc_sequence batterySequence;


// extern const struct adc_dt_spec batteryChannel;
extern int16_t batteryBuffer[1];

/* Tasks */
int intializeBattery();

void configureBattery();

battery_charge_level_t readbatteryCharge();

#endif