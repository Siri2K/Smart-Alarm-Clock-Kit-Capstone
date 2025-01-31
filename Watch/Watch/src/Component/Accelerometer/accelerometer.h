#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

/* Include Library */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>

/* Defnitions */
// Accelerometer Registers
#define ACCEL_ID_REG 0x13
#define ACCEL_SENS_CONFIG_1 0x15 

#define ACCEL_OUTPUT_X_LSB 0x04 
#define ACCEL_OUTPUT_Y_LSB 0x06 
#define ACCEL_OUTPUT_Z_LSB 0x08  
#define ACCEL_OUTPUT_X_MSB 0x05
#define ACCEL_OUTPUT_Y_MSB 0x07
#define ACCEL_OUTPUT_Z_MSB 0x08 

// Accelerometer Pins
#define ACCEL_ID 0x86
#define ACCEL_NODE DT_NODELABEL(spi0)

#define ACCEL_CS_NODE DT_NODELABEL(gpio0)
#define CS_PIN 16

// Global variables
typedef struct accelerometer_data_t{
    uint8_t id;
    int16_t x,y,z;
}accelerometer_data_t;

// Setup Devices
extern const struct device *csDev, *accelDev;
extern struct spi_config accelConfig;

/* Tasks */
void initializeAccelerometer(accelerometer_data_t* accelerometer_data);
void validateConnectiontoAccelerometer();
uint8_t readID();
void readAccelerometer(uint8_t reg, uint8_t values[], uint8_t size);
void readXYZ(accelerometer_data_t* accelerometer_data);

#endif