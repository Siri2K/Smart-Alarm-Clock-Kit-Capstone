#include "battery.h"

const struct device *batteryDevice = DEVICE_DT_GET(BATTERY_NODE);

struct adc_channel_cfg batteryChannel = {
    .channel_id = ADC_CHANNEL,
    .reference = ADC_REFERENCE,
    .gain = ADC_GAIN,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    
    #ifdef CONFIG_ADC_NRFX_SAADC
        .input_positive = ADC_PORT
    #endif

};

struct adc_sequence batterySequence = {
    .channels = BIT(ADC_CHANNEL),
    .buffer = batteryBuffer,
    .buffer_size = sizeof(batteryBuffer),
    .resolution = ADC_RESOLUTION
};

int intializeBattery(){
    // Return value
    int status = 1;

    // Configure Battery ADC
    status &= device_is_ready(batteryDevice); // Configure Device
    status &= adc_channel_setup(batteryDevice,&batteryChannel); // Configure Channel
}

battery_charge_level_t readbatteryCharge(){
    // Return Value
    int batterymV = 0;
    int status = 1;
    battery_charge_level_t chargeLevel = BATTERY_MIN;

    // Read battery in mV
    status &= adc_raw_to_millivolts_dt(&batteryChannel, &batterymV);

    // Convert Battery Levels to Percentage
    batterymV *= 7/6;
    batterymV *= 100;
    batterymV /= BATTERY_RATED_VOLTAGE_IN_MV;

    // Determine battery Level
    if(batterymV <= BATTERY_MIN ){
        chargeLevel = BATTERY_MIN;
    }
    else if(batterymV < BATTERY_LOW){
        chargeLevel = BATTERY_LOW;
    }
    else if(batterymV >= BATTERY_LOW && batterymV < BATTERY_MID){
        chargeLevel = BATTERY_MID;
    }
    else if(batterymV >= BATTERY_MID && batterymV < BATTERY_HIGH){
        chargeLevel = BATTERY_HIGH;
    }
    else{
        chargeLevel = BATTERY_MAX;
    }
    
    return chargeLevel;
    
}