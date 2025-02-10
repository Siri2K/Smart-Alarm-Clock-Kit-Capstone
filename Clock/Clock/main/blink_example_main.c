#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "BLE-Example";

// Change to your desired device name
#define DEVICE_NAME "ESP32-BLE"

// Forward declaration of our GAP event handler
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

// Setup advertising parameters
static esp_ble_adv_params_t adv_params = {
    .adv_int_min         = 0x20, // 32 * 0.625 ms = 20 ms
    .adv_int_max         = 0x40, // 64 * 0.625 ms = 40 ms
    .adv_type            = ADV_TYPE_IND,
    .own_addr_type       = BLE_ADDR_TYPE_PUBLIC,
    .channel_map         = ADV_CHNL_ALL,
    .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// Setup advertising data
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp        = false,
    .include_name        = true,   // Include device name in advertising
    .include_txpower     = false,
    .min_interval        = 0x0006, // 7.5ms
    .max_interval        = 0x0010, // 20ms
    .appearance          = 0x00,
    .manufacturer_len    = 0,
    .p_manufacturer_data = NULL,
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = 0,
    .p_service_uuid      = NULL,
    .flag                = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

void app_main(void)
{
    esp_err_t ret;

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Release Classic BT memory, since we're only using BLE
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // Initialize the Bluetooth Controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s initialize controller failed\n", __func__);
        return;
    }

    // Enable BLE mode
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed\n", __func__);
        return;
    }

    // Initialize Bluedroid stack
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "%s init bluetooth failed\n", __func__);
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "%s enable bluetooth failed\n", __func__);
        return;
    }

    // Register GAP callback to handle advertising events
    esp_ble_gap_register_callback(gap_event_handler);

    // Set the local device name
    esp_err_t set_name_ret = esp_ble_gap_set_device_name(DEVICE_NAME);
    if (set_name_ret){
        ESP_LOGE(TAG, "set device name failed, error code = %x", set_name_ret);
    }

    // Configure the advertising data
    esp_err_t config_adv_data_ret = esp_ble_gap_config_adv_data(&adv_data);
    if (config_adv_data_ret){
        ESP_LOGE(TAG, "config adv data failed, error code = %x", config_adv_data_ret);
    }

    ESP_LOGI(TAG, "BLE initialized. Configuring advertisement...");
}

// GAP event handler
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {

    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        // Once advertising data is set, we can start advertising
        ESP_LOGI(TAG, "Advertising data set complete, start advertising");
        esp_ble_gap_start_advertising(&adv_params);
        break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(TAG, "Advertising started successfully");
        } else {
            ESP_LOGE(TAG, "Failed to start advertising, error status = %x", param->adv_start_cmpl.status);
        }
        break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(TAG, "Advertising stopped successfully");
        } else {
            ESP_LOGE(TAG, "Failed to stop advertising, error status = %x", param->adv_stop_cmpl.status);
        }
        break;

    default:
        break;
    }
}
