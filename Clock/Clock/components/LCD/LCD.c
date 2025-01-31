#include "LCD.h"

void initializeLCD(lcd_t *lcd){
    // Initialize SPI
    ESP_ERROR_CHECK(configureLCDPins(lcd));
    ESP_ERROR_CHECK(configureLCDBus(lcd));
    ESP_ERROR_CHECK(configureLCDDeviceInterface(lcd));

    // Store Functions into Structure
    lcd->send = LCDSend;

    // Initialize Display
    initializeDisplay(lcd);
}

void initializeDisplay(lcd_t *lcd){
    // Reset the LCD
    gpio_set_level(LCD_RESET_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(LCD_RESET_PIN, 1);

    // Initialize the display
    LCDSend(lcd,COMMAND,LCD_CMD_DISPLAY_OFF); // Turn off display
    LCDSend(lcd,COMMAND,LCD_CMD_SET_PAGE); // Set page to 0
    LCDSend(lcd,COMMAND,LCD_CMD_SET_COL_LSB); // Set lower column address
    LCDSend(lcd,COMMAND,LCD_CMD_SET_COL_MSB); // Set higher column address
    LCDSend(lcd,COMMAND,LCD_CMD_DISPLAY_ON);  // Turn on display
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

void *LCDSend(void *lcdPtr, lcd_mode mode, uint8_t value){
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
    lcd->lcdTransaction.length = 8; //  Lenght 
    lcd->lcdTransaction.tx_buffer = (void*)(&value); // Transmitted Buffer
    spi_device_polling_transmit(lcd->lcdSPIDeviceHandle, &(lcd->lcdTransaction)); // Transmit Transaction
    return NULL;
}