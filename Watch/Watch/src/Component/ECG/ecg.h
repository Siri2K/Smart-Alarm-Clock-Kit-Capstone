#ifndef ECG_H
#define ECG_H

/* Zephyr and nRF Library */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>

/* C Library */
#include <stdio.h>
#include <stdlib.h>

/* Defnitions */
#define ECG_ADDRESS 0x62 // ECG Address
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

// ECG Parameters
#define SAMPLE_COUNT 100
#define SAMPLE_RATE 128
#define THRESHOLD 50000  // Adjust based on sensor output
#define MIN_PEAK_DISTANCE 5  // Minimum distance between peaks (avoid noise)


extern uint8_t initializeECG();

extern uint8_t getBPM();

#endif
