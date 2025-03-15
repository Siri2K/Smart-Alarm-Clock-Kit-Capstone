#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

/* Include Library */
// Zephyr
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>

// C
#include <stdio.h>
#include <stdlib.h>

/* Definitions */
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
    int8_t vx,vy,vz;
}accelerometer_data_t;

extern accelerometer_data_t accelerometerData;

/* Tasks */
extern uint8_t initializeAccelerometer();
extern uint8_t readXYZ(accelerometer_data_t* accelerometer_data);

#endif