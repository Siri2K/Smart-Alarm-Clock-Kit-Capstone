#ifndef LCD_H
#define LCD_H

/* Headers */ 
#include "Pins/pins.h"
#include "FreeRTOS_Driver/freeRTOS.h"

/* ESP-IDF Libraries */
#include "driver/spi_master.h"
#include "sdkconfig.h"

/* C Library */
#include <stdio.h>
#include <stdbool.h>

// Definitions
#define LCD_CMD_DISPLAY_OFF 0xAE
#define LCD_CMD_DISPLAY_ON  0xAF
#define LCD_CMD_SET_PAGE    0xB0
#define LCD_CMD_SET_COL_LSB 0x00
#define LCD_CMD_SET_COL_MSB 0x10

#define SPI_MODE 0x03 // SPI (CPOL=1, CPHA=1)
#define LCD_CLOCK_SPEED 1000000 // 1MHz

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
    void *(*send)(uint8_t*,uint8_t,uint8_t*);
}lcd_t;

void initializeLCD(lcd_t *lcd);

esp_err_t configureLCDPins(lcd_t *lcd);

esp_err_t configureLCDBus(lcd_t *lcd);

esp_err_t configureLCDDeviceInterface(lcd_t *lcd);

void *LCDSend(lcd_t *lcd, lcd_mode mode, uint8_t* value);

#endif 