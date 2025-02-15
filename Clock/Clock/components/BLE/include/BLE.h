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

/* Definition */
#define CLOCK_NAME "CLOCK-BLE"

/* Globals */
extern esp_ble_adv_params_t advertisingParameters;
extern esp_ble_adv_data_t advertisingData;

/* Functions */
extern void initializeBLE();
extern void initializeNVS();
extern void initializeBLEController();
extern void initializeBluedroid();
extern void configureBLE();
extern void GAPEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

#endif