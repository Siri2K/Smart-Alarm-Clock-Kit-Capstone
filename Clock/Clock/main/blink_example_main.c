#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h" // Ensure ESP-IDF path is set correctly
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_defs.h"
#include "esp_gattc_api.h"
#include "esp_gatts_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DEVICE_NAME "ESP32-BLE"
#define SERVICE_UUID 0x00FF
#define CHAR_UUID 0xFF01

static const char *TAG = "BLE-Example";

void ble_event_handler(esp_gatts_cb_event_t event, 
                        esp_gatt_if_t gatts_if, 
                        esp_ble_gatts_cb_param_t *param) {
    if (event == ESP_GATTS_CONNECT_EVT) {
        ESP_LOGI(TAG, "Device connected!");
    } else if (event == ESP_GATTS_DISCONNECT_EVT) {
        ESP_LOGI(TAG, "Device disconnected!");
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Initializing BLE...");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_LOGI(TAG, "BLE initialized. Ready to advertise!");
}
