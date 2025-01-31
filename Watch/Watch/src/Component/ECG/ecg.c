#include "ecg.h"

const struct device *ECGDev = DEVICE_DT_GET(ECG_NODE);
uint8_t ECGBuffer[1] = {0x00};

int connectToECG(){
    int status = 1;
    uint8_t value;
    status &= (i2c_reg_read_byte(ECGDev,ECG_READ_ADDRESS,ECG_INTERRUPT_STATUS,&value) == 0)? 1:0;
    return status;
}

int initializeECG(){
    int status = 1;
    
    // Reset the device
    status &= (i2c_reg_write_byte(ECGDev,ECG_WRITE_ADDRESS, ECG_SYSTEM_CONTROL, 0x01))? 1:0; // PWR_RDY = 1
    k_sleep(K_MSEC(1)); // 1ms delay for reset

    // Configure PPG settings
    status &= (i2c_reg_write_byte(ECGDev,ECG_WRITE_ADDRESS, ECG_PPG_CONFIGURATION_1, 0x0B))? 1:0; // PPG1_ADC_RGE[1:0] = 10 , PPG_TINT[1:0] = 11
    status &= (i2c_reg_write_byte(ECGDev,ECG_WRITE_ADDRESS, ECG_PPG_CONFIGURATION_2, 0x00))? 1:0; // PPG_SR[4:0] = 00000, SMP_AVE[2:0] = 00 

    // Configure LED settings
    status &= (i2c_reg_write_byte(ECGDev,ECG_WRITE_ADDRESS, ECG_LED_RANGE_AMPLITUDE_1, 0x0F))? 1:0; // LED2_RGE[1:0] = 11, LED1_RGE[1:0] = 11
    status &= (i2c_reg_write_byte(ECGDev,ECG_WRITE_ADDRESS, ECG_LED_PA1, 0x20))? 1:0; // LED1_DRV = 0100 0000
    status &= (i2c_reg_write_byte(ECGDev,ECG_WRITE_ADDRESS, ECG_LED_PA2, 0x20))? 1:0; // LED2_DRV = 0100 0000

    // Configure FIFO
    status &= (i2c_reg_write_byte(ECGDev,ECG_WRITE_ADDRESS, ECG_FIFO_CONFIGURATION_1, 0x1C))? 1:0; // FIFO_A_FULL = 0001 1100 -> 28 space intterupt & 100 samples
    status &= (i2c_reg_write_byte(ECGDev,ECG_WRITE_ADDRESS, ECG_FIFO_CONFIGURATION_2, 0x01))? 1:0; // FIFO_RO = 1
    status &= (i2c_reg_write_byte(ECGDev,ECG_WRITE_ADDRESS, ECG_INTERRUPT_ENABLE_1, 0x80))? 1:0; // Enable A_FULL interrupt

    // Configure LED sequence
    status &= (i2c_reg_write_byte(ECGDev,ECG_WRITE_ADDRESS, ECG_LED_SEQUENCE_1, 0x23))? 1:0; // LED2 (IR) and LED3 (Red) exposure
    status &= (i2c_reg_write_byte(ECGDev,ECG_WRITE_ADDRESS, ECG_LED_SEQUENCE_2, 0x00))? 1:0; // Disable other LEDs

    // Exit shutdown mode
    status &= (i2c_reg_write_byte(ECGDev,ECG_WRITE_ADDRESS, ECG_SYSTEM_CONTROL, 0x00))? 1:0; // Clear SHDN bit to start sampling

    return status;
}

uint32_t* getFIFODataSamples(){
    int status = 1;
    uint8_t* fifoData;
    uint32_t* fullSample;

    // Intialize pointers
    fifoData = (uint8_t*)malloc(3*sizeof(uint8_t));
    fullSample = (uint32_t*)malloc(SAMPLE_RATE*sizeof(uint32_t));

    // Gather Samples
    for(int i=0; i<SAMPLE_RATE*3;i++){
        status &= (i2c_reg_read_byte(ECGDev,ECG_READ_ADDRESS,ECG_FIFO_DATA,&fifoData[i%3]) == 0)? 1:0;
        // Compile Sample onto full Sample
        if((i+1)%3 == 0){ // Stop After evey 3 reads, aka 24-bits
            fullSample[i/3] = fullSample[i]<<16 | fullSample[i-1]<<8 | fullSample[i-2]<<0;
        }
    }
    // Free and Return Full Sample
    free(fifoData);
    return fullSample; 
}


uint8_t countNumberOfPeaks(uint32_t* samples){
    // HeartBeat Data
    uint32_t peak = 0;
    uint8_t peakCount = 0;

    // Calculate Number of peaks
    for(int i=0; i<SAMPLE_RATE;i++){
        if(((samples[i] >> 19) & 0x0F) == 0x02){
            uint32_t oldPeak = peak;
            peak = (oldPeak < (samples[i] & 0x3FFFF)) ? (samples[i] & 0x3FFFF) : oldPeak;
            if(peak > oldPeak){
                peakCount++;
            }
        }
    }
    return peakCount;
}

uint8_t calculateBPM(uint32_t* samples){
    // Calculate Number of Peaks
    uint8_t peakCount = countNumberOfPeaks(samples);
    uint8_t timebetweenPeaks = 1/SAMPLE_RATE;
    uint8_t bpm = 0;

    // Calculate BPM
    bpm = (peakCount>1) ? 60 * (peakCount-1)/(timebetweenPeaks*peakCount):0;
    return bpm;
}
