#include "ecg.h"

static const struct device *ECGDev = DEVICE_DT_GET(ECG_NODE);

uint8_t initializeECG(){
    // Status
    uint8_t status = 0;

    // Registers and Values to write into
    uint8_t resetRegisterAndData[] = {ECG_SYSTEM_CONTROL, 0x01}; // PWR_RDY = 1

    uint8_t writeRegisterAndData[] = { 
        // Configure PPG settings
        ECG_PPG_CONFIGURATION_1, 0x0B, // PPG1_ADC_RGE[1:0] = 10 , PPG_TINT[1:0] = 11
        ECG_PPG_CONFIGURATION_2, 0x00, // PPG_SR[4:0] = 00000, SMP_AVE[2:0] = 00

        // Configure LED settings
        ECG_LED_RANGE_AMPLITUDE_1, 0x0F, // LED2_RGE[1:0] = 11, LED1_RGE[1:0] = 11
        ECG_LED_PA1, 0x20, // LED1_DRV = 0100 0000
        ECG_LED_PA2, 0x20, // LED2_DRV = 0100 0000

        // Configure FIFO
        ECG_FIFO_CONFIGURATION_1, 0x1C, // FIFO_A_FULL = 0001 1100 -> 28 space intterupt & 100 samples
        ECG_FIFO_CONFIGURATION_2, 0x01, // FIFO_RO = 1
        ECG_INTERRUPT_ENABLE_1, 0x80, // Enable A_FULL interrupt

        // Configure LED sequence
        ECG_LED_SEQUENCE_1, 0x23, // LED2 (IR) and LED3 (Red) exposure
        ECG_LED_SEQUENCE_2, 0x00, // Disable other LEDs

        // Exit shutdown mode
        ECG_SYSTEM_CONTROL, 0x00, // Clear SHDN bit to start sampling
    };

    /* Write to ECG*/
    // Reset ECG
    status = i2c_write(ECGDev,resetRegisterAndData, sizeof(resetRegisterAndData), ECG_ADDRESS);
    k_sleep(K_MSEC(1)); // 1ms delay for reset

    // Configure ECG
    status = i2c_write(ECGDev,writeRegisterAndData, sizeof(writeRegisterAndData), ECG_ADDRESS);
     
    return status;
}

static uint8_t getFIFODataSamples(uint32_t *data){
    uint8_t status;
    uint8_t fifoData[SAMPLE_COUNT*3]; // Gather 100 BP samples

    // Read ECG
    status = i2c_burst_read(ECGDev,ECG_ADDRESS,ECG_FIFO_DATA,fifoData,sizeof(fifoData));
    if(status < 0){
        return status;
    }

    // Combine Data into Samples
    for(int i=0;i<SAMPLE_COUNT;i++){
        data[i] = (((fifoData[i*3]&0x03)<<16) | (fifoData[i*3+1]<<8) | fifoData[i*3+2]);
    }
    return status;
}

static void lowPassFilters(uint32_t *data, uint32_t *filtered, int windowSize){
    for(uint8_t i = 0; i < SAMPLE_COUNT; i++) {
        int sum = 0;
        int count = 0;
        for (int j=i;j>i-windowSize && j>=0; j--) {
            sum += data[j];
            count++;
        }
        filtered[i] = sum / count;  // Average over the window
    }
}

static uint8_t findPeaks(uint32_t *data, uint8_t *peak_indices){
    uint8_t peakCount = 0;
    for(uint8_t i = 1; i < SAMPLE_COUNT - 1; i++) {
        if (data[i] > THRESHOLD && data[i] > data[i - 1] && data[i] > data[i + 1]) {
            if (peakCount == 0 || (i - peak_indices[peakCount - 1]) > MIN_PEAK_DISTANCE) {
                peak_indices[peakCount++] = i;
            }
        }
    }
    return peakCount;
}

static uint8_t calculateBPM(uint8_t *peakIndex, uint8_t peakCount){
    // Ensure there's enough to sample
    if(peakCount < 2){
        return 0;
    }

    // Count Number of Intervals
    uint8_t intervalCount=0;
    for(uint8_t i = 1; i < peakCount; i++) {
        intervalCount += (peakIndex[i] - peakIndex[i - 1]);
    }

    // Calculate BPM
    float averageInterval = (float)intervalCount/(peakCount-1);
    uint8_t bpm = (uint8_t)((60.0f*SAMPLE_RATE)/averageInterval);
    return bpm;
}

uint8_t getBPM(){
    // Variables
    uint32_t rawData[SAMPLE_COUNT];  // Raw PPG samples
    uint32_t filteredData[SAMPLE_COUNT];
    uint8_t peakIndices[SAMPLE_COUNT];
    uint8_t peakCount=0, bpm=0;

    // Read BPM
    if(getFIFODataSamples(rawData)){
        lowPassFilters(rawData,filteredData,5);

        // Calculate BPM
        peakCount=findPeaks(filteredData,peakIndices);
        bpm = calculateBPM(peakIndices,peakCount);
    }
    return bpm;
}
