#ifndef BLE_H
#define BLE_H

/* Drivers */
#include <esp_mac.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <esp_bt_defs.h>
#include <nvs_flash.h>

/* C Library */
#include <stdio.h>

/* Definition */
#define CLOCK_NAME "CLOCK-BLE"

#define GATTS_APP_ID    0x55
#define SERVICE_UUID    0x00FF
#define CHAR_UUID       0xFF01


/* Functions */
extern void initializeBLE();
extern void initializeNVS();
extern void initializeBLEController();
extern void initializeBluedroid();
extern void configureBLE();



#endif