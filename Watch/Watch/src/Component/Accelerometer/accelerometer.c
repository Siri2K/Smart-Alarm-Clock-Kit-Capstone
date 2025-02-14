#include "accelerometer.h"

static const struct device *csDev = DEVICE_DT_GET(ACCEL_NODE);
static const struct device *accelDev = DEVICE_DT_GET(ACCEL_NODE);

static struct spi_config accelConfig = {
    .frequency = 40000000,
    .operation = SPI_WORD_SET(8),
    .slave = 0,
};

uint8_t initializeAccelerometer(accelerometer_data_t* accelerometer_data){
    // Initialize Accelerometer Data
    accelerometer_data->vx = 0;
    accelerometer_data->vy = 0;
    accelerometer_data->vz = 0;

    // Setup CS GPIO Pin
    return gpio_pin_configure(csDev, CS_PIN, GPIO_OUTPUT) && gpio_pin_set(csDev, CS_PIN, GPIO_OUT_PIN16_High);
}

static uint8_t readAccelerometer(uint8_t reg, uint8_t *values, uint8_t size) {
    // Setup Buffer
    uint8_t status = 0;
    uint8_t txBuffer[1] = {reg};

    // Setup SPI
    struct spi_buf txSPIBuffer = {
        .buf = txBuffer,
        .len = sizeof(txBuffer),
    };

    struct spi_buf_set txSPIBufferSet = {
        .buffers = &txSPIBuffer,
        .count = 1,
    };

    struct spi_buf rxSPIBuffer = {
        .buf = values,
        .len = size,  // Corrected to use the 'size' parameter directly
    };

    struct spi_buf_set rxSPIBufferSet = {
        .buffers = &rxSPIBuffer,
        .count = 1,  // This should be 1 because you're only passing one buffer
    };
    
    // Perform SPI Read
    status = gpio_pin_set(csDev, CS_PIN, 0);
    status = spi_transceive(accelDev, &accelConfig, &txSPIBufferSet, &rxSPIBufferSet);
    status = gpio_pin_set(csDev, CS_PIN, 1);
    return status;
}

uint8_t readXYZ(accelerometer_data_t* accelerometer_data){
    // Get Status
    uint8_t status = 0;

    // Stored Axis values
    int8_t rawData[6]; // X, Y, Z

    // Get Current Time and Duration
    int64_t startTime = k_uptime_get();

    // Extract Data from Accelerometer
    status = readAccelerometer(ACCEL_OUTPUT_X_LSB,rawData,6);
    
    if(status == 0){
        // Store velocities
        int64_t duration  = k_uptime_get() - startTime;
        accelerometer_data->vx = (int8_t)(((rawData[1] << 8) | rawData[0])/(duration));
        accelerometer_data->vy = (int8_t)(((rawData[3] << 8) | rawData[2])/(duration));
        accelerometer_data->vz = (int8_t)(((rawData[5] << 8) | rawData[4])/(duration));
    }

    return status;
    
}