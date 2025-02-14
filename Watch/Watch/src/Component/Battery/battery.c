#include "battery.h"

uint8_t batteryBuffer[1];
static const struct device *batteryDevice = DEVICE_DT_GET(BATTERY_NODE);

static struct adc_channel_cfg batteryChannel = {
    .channel_id = ADC_CHANNEL,
    .reference = ADC_REFERENCE,
    .gain = ADC_GAIN,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    
    #ifdef CONFIG_ADC_NRFX_SAADC
        .input_positive = ADC_PORT
    #endif

};

static struct adc_sequence batterySequence = {
    .channels = BIT(ADC_CHANNEL),
    .buffer = batteryBuffer,
    .buffer_size = sizeof(batteryBuffer),
    .resolution = ADC_RESOLUTION
};

uint8_t initializeBattery(){
    return (uint8_t)device_is_ready(batteryDevice) && (uint8_t)adc_channel_setup(batteryDevice,&batteryChannel); // Configigure Device and Channel
}

uint16_t readBatteryChargePercentage(){
    return (adc_read(batteryDevice,&batterySequence)*25)>>6;
}