#ifndef ECG_H
#define ECG_H

/* Zephyr and nRF Library */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>

/* C Library */
#include <stdlib.h>

/* Defnitions */
// ECG Read and Write Registers
#define ECG_WRITE_ADDRESS   0xC4
#define ECG_READ_ADDRESS    0xC5
#define ECG_NODE DT_NODELABEL(i2c0)

// Interrupt
#define ECG_INTERRUPT_STATUS 0x00 // Interrupt Status
#define ECG_INTERRUPT_ENABLE_1 0x02 // All Interrupts Ready

// System
#define ECG_SYSTEM_CONTROL 0x0D

// PPG Settings
#define ECG_PPG_CONFIGURATION_1 0x11 // PPG_TINT & ADC range
#define ECG_PPG_CONFIGURATION_2 0x12 // Sample Avg & Sample rate

// LED Settings
#define ECG_LED_RANGE_AMPLITUDE_1 0x2A // Configure Current Drive max
#define ECG_LED_PA1 0x23 // Drive current for LED1
#define ECG_LED_PA2 0x24 // Drive current for LED2

// FIFO
#define ECG_FIFO_CONFIGURATION_1 0x09
#define ECG_FIFO_CONFIGURATION_2 0x0A

// LED Sequence Control
#define ECG_LED_SEQUENCE_1 0x20 // Control LED 1 & 2 Sequence
#define ECG_LED_SEQUENCE_2 0x21 // Control LED 3 & 4 Sequence

// FIFO Data Register
#define ECG_FIFO_DATA 0x08


#define SAMPLE_RATE 25

/* Globals */
extern const struct device *ECGDev;
extern struct i2c_config ECGConfig;

extern uint8_t ECGBuffer[1];

int connectToECG();

int initializeECG();

void writeECG();

uint8_t readECG();

uint8_t* readFIFO();

uint32_t readFIFOData();

uint8_t* readHeartBeat(uint32_t fifo_sample);

#endif
