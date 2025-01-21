#include "battery.h"

const struct adc_dt_spec batteryChannel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));
int16_t batteryBuffer;
struct adc_sequence batterySequence;

int intializeBattery(){
    // Return value
    int status = 1;
    
    // Configure battery
    configureBattery();

    // Setup ADC
    status &= device_is_ready(&batteryChannel);
    status &= adc_channel_setup_dt(&batteryChannel);

    // Configure ADC
    configureBattery();

    // Initialize Sequence
    status &= adc_sequence_init_dt(&batteryChannel, &batterySequence);

    return status;
}

void configureBattery(){
    // Configure Sequence
    batterySequence.buffer = &batteryBuffer;
    batterySequence.buffer_size = sizeof(batteryBuffer);
    batterySequence.calibrate = true;
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