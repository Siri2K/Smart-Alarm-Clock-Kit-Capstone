#include "LCD.h"

void initializeLCD(lcd_t *lcd){
    // Initialize SPI
    ESP_ERROR_CHECK(configureLCDPins(lcd));
    ESP_ERROR_CHECK(configureLCDBus(lcd));
    ESP_ERROR_CHECK(configureLCDDeviceInterface(lcd));

    // Store Functions into Structure
    lcd->send = sendToLCD;
    lcd->cursor = setCursorToLCD;
    lcd->display = displayToLCD;

    // Initialize Display
    initializeDisplay(lcd);
}

void initializeDisplay(lcd_t *lcd){
    // Reset the LCD
    gpio_set_level(LCD_RESET_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(LCD_RESET_PIN, 1);

    // Initialize the display
    sendToLCD(lcd,COMMAND,LCD_DISPLAY_OFF); 
    sendToLCD(lcd,COMMAND,LCD_ADC_NORMAL); 
    sendToLCD(lcd,COMMAND,LCD_COMMON_OUTPUT_MODE);
    sendToLCD(lcd,COMMAND,LCD_NORMAL_DISPLAY_MODE); 
    sendToLCD(lcd,COMMAND,LCD_DISPLAY_NORMAL_MODE); 
    sendToLCD(lcd,COMMAND,LCD_BIAS_RATIO); 
    sendToLCD(lcd,COMMAND,LCD_POWER_CONTROL); 
    sendToLCD(lcd,COMMAND,LCD_CONTRAST); 
    sendToLCD(lcd,COMMAND,LCD_SET_CONTRAST); 
    sendToLCD(lcd,COMMAND,LCD_CONTRAST_LEVEL);  
    sendToLCD(lcd,COMMAND,LCD_TEMPERATURE_COMPENSATION); 
    sendToLCD(lcd,COMMAND,LCD_TEMPERATURE_COEFFICIENT);
    sendToLCD(lcd,COMMAND,LCD_DISPLAY_ON); 
}

esp_err_t configureLCDPins(lcd_t *lcd){
    // Configure A0 & RST as GPIO Ouputs
    lcd->lcdPinConfig.pin_bit_mask = (1ULL << LCD_A0_PIN) | (1ULL << LCD_RESET_PIN);
    lcd->lcdPinConfig.mode  = GPIO_MODE_OUTPUT;
    lcd->lcdPinConfig.pull_down_en  = GPIO_PULLDOWN_DISABLE;
    lcd->lcdPinConfig.pull_up_en  = GPIO_PULLDOWN_DISABLE;
    lcd->lcdPinConfig.intr_type  = GPIO_PULLDOWN_DISABLE;
    return gpio_config(&(lcd->lcdPinConfig));
}

esp_err_t configureLCDBus(lcd_t *lcd){
    // Configure SPI Pins
    lcd->lcdSPIBusConfig.mosi_io_num = LCD_MOSI_PIN;
    lcd->lcdSPIBusConfig.miso_io_num = GPIO_NUM_NC;
    lcd->lcdSPIBusConfig.sclk_io_num = LCD_SCLK_PIN;
    lcd->lcdSPIBusConfig.quadwp_io_num = GPIO_NUM_NC;
    lcd->lcdSPIBusConfig.quadhd_io_num = GPIO_NUM_NC;
    lcd->lcdSPIBusConfig.max_transfer_sz = 132;
    return spi_bus_initialize(SPI2_HOST, &(lcd->lcdSPIBusConfig),SPI_DMA_CH_AUTO);
}

esp_err_t configureLCDDeviceInterface(lcd_t *lcd){
    lcd->lcdSPIDeviceInterfaceConfig.clock_speed_hz = LCD_CLOCK_SPEED;
    lcd->lcdSPIDeviceInterfaceConfig.mode = SPI_MODE;
    lcd->lcdSPIDeviceInterfaceConfig.spics_io_num = LCD_CS_PIN; 
    lcd->lcdSPIDeviceInterfaceConfig.queue_size = 1;
    return spi_bus_add_device(SPI2_HOST, &(lcd->lcdSPIDeviceInterfaceConfig), &(lcd->lcdSPIDeviceHandle));
}

void *sendToLCD(void *lcdPtr, lcd_mode mode, uint8_t value){
    // Convert to LCD
    lcd_t* lcd = (lcd_t*)lcdPtr;
    
    // Set A0 Pin
    switch (mode)
    {
    case COMMAND: // Command Mode
        gpio_set_level(LCD_A0_PIN, 0);
        break;
    case DATA: // Data  Mode
        gpio_set_level(LCD_A0_PIN, 1);
        break;
    default:
        break;
    }

    // Transmit value
    lcd->lcdTransaction.flags = SPI_TRANS_USE_TXDATA;
    lcd->lcdTransaction.length = 8; //  Lenght 
    lcd->lcdTransaction.tx_data[0] = value; // Command or Data
    spi_device_transmit(lcd->lcdSPIDeviceHandle, &lcd->lcdTransaction);
    return NULL;
}

void *setCursorToLCD(void *lcdPtr, uint8_t page, uint8_t column){
    // Set Cursor
    sendToLCD(lcdPtr,COMMAND, LCD_PAGE_0 | (page & 0x03));
    sendToLCD(lcdPtr,COMMAND, LCD_COLUMN_MSB | (column >> 4));
    sendToLCD(lcdPtr,COMMAND, LCD_COLUMN_LSB | (column & 0x0F));
    return NULL;
}

void *displayToLCD(void *lcdPtr, const uint8_t *font, uint8_t page, uint8_t start_col, const char *text){
    // Set the Cursor
    setCursorToLCD(lcdPtr,page,start_col);

    // Write Text in ASCII
    while (*text) {
        uint8_t c = *text++ - 32; // ASCII offset
        for (int i = 0; i < 8; i++) {
            sendToLCD(lcdPtr,DATA,font[c * 8 + i]); // Send font data
        }
        sendToLCD(lcdPtr,DATA,0x00); // Space between characters
    }
    return NULL;
}