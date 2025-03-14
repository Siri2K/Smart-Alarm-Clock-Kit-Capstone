#ifndef BLE_H
#define BLE_H

/* Drivers */
#include <esp_mac.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <nvs_flash.h>

/* C Library */
#include <stdio.h>
#include <string.h>

/* Definition */
#define CLOCK_NAME "CLOCK-BLE"
#define WATCH_MAC (uint8_t[6]){0xCC,0xC2,0xCC,0xFC,0x8A,0x09}

/* Globals */
extern uint8_t storedData[4];

/* Functions */
extern void initializeBLE();
extern void initializeNVS();
extern void initializeBLEController();
extern void initializeBluedroid();
extern void configureBLE();
static void GAPEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

#endif