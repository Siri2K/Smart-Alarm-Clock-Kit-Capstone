#ifndef PINS_H
#define PINS_H

/* GPIO Header */
#include "driver/gpio.h"

/* Slide Switch pins */
#define TIME_MODE_SWITCH GPIO_NUM_25
#define CLOCK_MODE_SWITCH GPIO_NUM_26
#define ALARM_MODE_SWITCH GPIO_NUM_27 

/* Push Button  pins */
#define UP_BUTTON GPIO_NUM_34
#define DOWN_BUTTON GPIO_NUM_35
#define SELECT_BUTTON GPIO_NUM_14

/* Buzzer */
#define BUZZER GPIO_NUM_15

/* RTC */
#define RTC_MASTER_SDA_IO GPIO_NUM_21
#define RTC_MASTER_SCL_IO GPIO_NUM_22

/* LCD */
#define LCD_MOSI_PIN   GPIO_NUM_23  // SPI Master Out Slave In
#define LCD_SCLK_PIN   GPIO_NUM_18  // SPI Clock
#define LCD_CS_PIN     GPIO_NUM_5   // Chip Select
#define LCD_RESET_PIN  GPIO_NUM_4   // Reset
#define LCD_A0_PIN     GPIO_NUM_16  // Data/Command Selection (A0)

#endif 