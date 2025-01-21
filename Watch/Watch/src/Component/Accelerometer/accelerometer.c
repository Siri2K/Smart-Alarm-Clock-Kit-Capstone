#include "accelerometer.h"

const struct device *csDev = DEVICE_DT_GET(ACCEL_NODE);
const struct device *batteryDev = DEVICE_DT_GET(ACCEL_NODE);

struct spi_config accelConfig = {
    .frequency = 40000000,
    .operation = SPI_WORD_SET(8),
    .slave = 0,
};

void initializeAccelerometer(accelerometer_data_t* accelerometer_data){
    // Setup CS GPIO Pin
    gpio_pin_configure(csDev, CS_PIN, GPIO_OUTPUT);
    gpio_pin_set(csDev, CS_PIN, GPIO_OUT_PIN16_High);

    // Initialize Accelerometer Data
    accelerometer_data->id = ACCEL_ID;
    accelerometer_data->x = 0;
    accelerometer_data->y = 0;
    accelerometer_data->z = 0;
}

void validateConnectiontoAccelerometer(){
    // Read CHIP ID
    uint8_t accelID[1] = {0};
    readAccelerometer(ACCEL_ID_REG,accelID,1);
}

void readAccelerometer(uint8_t reg, uint8_t values[], uint8_t size) {
    // Return value
    int status = 0;

    // Setup Buffer
    uint8_t txBuffer[1];
    txBuffer[0] = reg;

    // Setup SPI
    struct spi_buf txSPIBuffer[] = {{
        .buf = txBuffer,
        .len = sizeof(txBuffer),
       }
    };

    struct spi_buf_set txSPIBufferSet = {
        .buffers = txSPIBuffer,
        .count = 1,
    };

    struct spi_buf rxSPIBuffer[] = {{
        .buf = values,
        .len = sizeof(values),
       }
    };

    struct spi_buf_set rxSPIBufferSet = {
        .buffers = rxSPIBuffer,
        .count = size,
    };
    
    // Perform SPI Read
    gpio_pin_set(csDev, CS_PIN, 0);
    do{
        txBuffer[0] = reg;
        status = spi_write(accelDev, &accelConfig, &txSPIBufferSet);

        if(status < 0){
            break;
        }

        status = spi_read(accelDev, &accelConfig, &rxSPIBufferSet);
    }while(false);
    gpio_pin_set(csDev, CS_PIN, 1);
    
}

void readXYZ(accelerometer_data_t* accelerometer_data){
    // Stored Axis values
    uint8_t xLSB[1],xMSB[1]; // Store X values
    uint8_t yLSB[1],yMSB[1]; // Store Y values
    uint8_t zLSB[1],zMSB[1]; // Store Z values

    // Extract Data from Accelerometer
    readAccelerometer(ACCEL_OUTPUT_X_LSB,xLSB,1);
    readAccelerometer(ACCEL_OUTPUT_X_MSB,xMSB,1);
    readAccelerometer(ACCEL_OUTPUT_Y_LSB,yLSB,1);
    readAccelerometer(ACCEL_OUTPUT_Y_MSB,yMSB,1);
    readAccelerometer(ACCEL_OUTPUT_Z_LSB,zLSB,1);
    readAccelerometer(ACCEL_OUTPUT_Z_MSB,zMSB,1);

    // Store data
    accelerometer_data->x = (xMSB[0] << 8) | xLSB[0];
    accelerometer_data->y = (yMSB[0] << 8) | yLSB[0];
    accelerometer_data->z = (zMSB[0] << 8) | zLSB[0];
}