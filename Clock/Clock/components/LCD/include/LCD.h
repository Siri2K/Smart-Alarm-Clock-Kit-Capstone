#ifndef LCD_H
#define LCD_H

/* Headers */ 
#include "freertos/FreeRTOS.h"

/* ESP-IDF Libraries */
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <sdkconfig.h>

/* C Library */
#include <stdio.h>
#include <stdbool.h>

/* Defnitions */
// Configurations
#define LCD_CMD_DISPLAY_OFF 0xAE
#define LCD_CMD_DISPLAY_ON  0xAF
#define LCD_CMD_SET_PAGE    0xB0
#define LCD_CMD_SET_COL_LSB 0x00
#define LCD_CMD_SET_COL_MSB 0x10

#define SPI_MODE 0x03 // SPI (CPOL=1, CPHA=1)
#define LCD_CLOCK_SPEED 1000000 // 1MHz

// Pins
#define LCD_MOSI_PIN   GPIO_NUM_4  // SPI Master Out Slave In
#define LCD_SCLK_PIN   GPIO_NUM_2  // SPI Clock
#define LCD_CS_PIN     GPIO_NUM_12   // Chip Select
#define LCD_RESET_PIN  GPIO_NUM_13   // Reset
#define LCD_A0_PIN     GPIO_NUM_15  // Data/Command Selection (A0)

/* Global Struct or Enums*/
typedef enum lcd_mode{
    COMMAND = 0,
    DATA = 1
}lcd_mode;

typedef struct lcd_t{
    // I2C Configuration
    gpio_config_t lcdPinConfig;
    spi_bus_config_t lcdSPIBusConfig;
    spi_device_interface_config_t lcdSPIDeviceInterfaceConfig;
    spi_device_handle_t lcdSPIDeviceHandle;
    spi_transaction_t lcdTransaction;

    // Tasks
    void *(*send)(void*,lcd_mode,uint8_t);
}lcd_t;

extern void initializeLCD(lcd_t *lcd);

extern void initializeDisplay(lcd_t *lcd);

extern esp_err_t configureLCDPins(lcd_t *lcd);

extern esp_err_t configureLCDBus(lcd_t *lcd);

extern esp_err_t configureLCDDeviceInterface(lcd_t *lcd);

extern void *LCDSend(void *lcd, lcd_mode mode, uint8_t value);

#endif 