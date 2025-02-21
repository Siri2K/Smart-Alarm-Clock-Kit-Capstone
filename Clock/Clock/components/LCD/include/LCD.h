#ifndef LCD_H
#define LCD_H

/* ESP-IDF Libraries */
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <sdkconfig.h>

/* C Library */
#include <stdio.h>
#include <stdbool.h>

/* Defnitions */
// Commands
#define LCD_DISPLAY_OFF                 0xAE
#define LCD_ADC_NORMAL                  0xA0
#define LCD_COMMON_OUTPUT_MODE          0xC0
#define LCD_NORMAL_DISPLAY_MODE         0xA4
#define LCD_DISPLAY_NORMAL_MODE         0xA6
#define LCD_BIAS_RATIO                  0xA2
#define LCD_POWER_CONTROL               0x2F
#define LCD_CONTRAST                    0x27
#define LCD_SET_CONTRAST                0x81
#define LCD_CONTRAST_LEVEL              0x10
#define LCD_TEMPERATURE_COMPENSATION    0xFA
#define LCD_TEMPERATURE_COEFFICIENT     0x90
#define LCD_DISPLAY_ON                  0xAF

#define LCD_PAGE_0                      0xB0 
#define LCD_COLUMN_MSB                  0x10 
#define LCD_COLUMN_LSB                  0x00  


// SPI variables
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
    void *(*send)(void*, lcd_mode, uint8_t);
    void *(*cursor)(void*, uint8_t, uint8_t);
    void *(*display)(void*, const uint8_t*, uint8_t, uint8_t, const char*);
}lcd_t;

// Constant
const uint8_t lcdCharacters[] = {
    0x7E, 0x81, 0x81, 0x81, 0x81, 0x7E, 0x00, 0x00,  // Code for char 0
    0x00, 0x42, 0x41, 0x7F, 0x40, 0x40, 0x00, 0x00,  // Code for char 1
    0xC6, 0xE3, 0xB1, 0x99, 0x8F, 0x86, 0x00, 0x00,  // Code for char 2
    0x00, 0xD3, 0x91, 0x91, 0x91, 0xFF, 0x00, 0x00,  // Code for char 3
    0x00, 0x00, 0x18, 0x14, 0x12, 0xFF, 0x10, 0x00,  // Code for char 4
    0x00, 0x4F, 0x89, 0x89, 0x89, 0xD9, 0x71, 0x00,  // Code for char 5
    0x7E, 0x91, 0x89, 0x89, 0x89, 0x7A, 0x00, 0x00,  // Code for char 6
    0x00, 0x01, 0x01, 0x01, 0x01, 0xFF, 0x00, 0x00,  // Code for char 7
    0x7E, 0x91, 0x91, 0x91, 0x91, 0x7E, 0x00, 0x00,  // Code for char 8
    0x4E, 0x89, 0x91, 0x91, 0x89, 0x7E, 0x00, 0x00,  // Code for char 9
    0x00, 0x00, 0x00, 0x33, 0x33, 0x00, 0x00, 0x00,  // Code for char :
    0x7E, 0x42, 0x42, 0x42, 0xFE, 0x40, 0x00, 0x00,  // Code for char a
    0xFC, 0x90, 0x90, 0xF0, 0x00, 0x00, 0x00, 0x00,  // Code for char b
    0xF0, 0x10, 0x08, 0xF8, 0x08, 0x10, 0xF0, 0x00,  // Code for char m
    0xFC, 0x24, 0x24, 0x3C, 0x00, 0x00, 0x00, 0x00  // Code for char p
};


// Functions
extern void initializeLCD(lcd_t *lcd);

extern void initializeDisplay(lcd_t *lcd);

extern esp_err_t configureLCDPins(lcd_t *lcd);

extern esp_err_t configureLCDBus(lcd_t *lcd);

extern esp_err_t configureLCDDeviceInterface(lcd_t *lcd);

extern void *sendToLCD(void *lcdPtr, lcd_mode mode, uint8_t value);

extern void *setCursorToLCD(void *lcdPtr, uint8_t page, uint8_t column);

extern void *displayToLCD(void *lcdPtr, const uint8_t *font, uint8_t page, uint8_t start_col, const char *text);

#endif 