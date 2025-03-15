#include "battery.h"

int16_t batteryBuffer[1];
static const struct device *batteryDevice = DEVICE_DT_GET(BATTERY_NODE);

static struct adc_channel_cfg batteryChannel = {
    .channel_id = ADC_CHANNEL,
    .reference = ADC_REFERENCE,
    .gain = ADC_GAIN,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .input_positive = ADC_PORT
};

static struct adc_sequence batterySequence = {
    .channels = BIT(ADC_CHANNEL),
    .buffer = batteryBuffer,
    .buffer_size = sizeof(batteryBuffer),
    .resolution = ADC_RESOLUTION
};

int8_t initializeBattery(){
    int8_t status = 0;

    // Ready Device
    status = (int8_t)device_is_ready(batteryDevice); // Configure Device
    if(status != 1){
        printk("Battery ADC Pin %d is not ready and returns with status %d\n", ADC_CHANNEL, status);
        return status;
    }

    // Channel Setup
    status = (int8_t)adc_channel_setup(batteryDevice,&batteryChannel); // Configure Channel
    if(status != 0){
        printk("Battery ADC Pin %d is not ready and returns with status %d\n", ADC_CHANNEL, status);
        return status;
    }
    
    return 0;
}

int16_t readBatteryChargePercentage(){
    // Read ADC
    int8_t status = adc_read(batteryDevice,&batterySequence);
    if(status != 0){
        printk("ADC read failed with status %d\n", status);
        return 0;
    }

    // Convert ADC
    return (batteryBuffer[0]*25)>>6;
}