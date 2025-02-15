#include "BLE.h"

esp_ble_adv_params_t advertisingParameters = {
    .adv_int_min         = 0x20, // 32 * 0.625 ms = 20 ms
    .adv_int_max         = 0x40, // 64 * 0.625 ms = 40 ms
    .adv_type            = ADV_TYPE_IND,
    .own_addr_type       = BLE_ADDR_TYPE_PUBLIC,
    .channel_map         = ADV_CHNL_ALL,
    .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

esp_ble_adv_data_t advertisingData = {
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

void initializeBLE(){
    initializeNVS();
    initializeBLEController();
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    initializeBluedroid();
    configureBLE();
}

void initializeNVS(){
    // Initialize NVS
    uint8_t i=0;
    while (++i<=10){ // Limit to 10 attemtps
        if(nvs_flash_init() == ESP_OK){
            break;
        }
        ESP_ERROR_CHECK(nvs_flash_erase());
    }

    // Limit memory to BLE only
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    
}

void initializeBLEController(){
    // Initialize the Bluetooth Controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
}

void initializeBluedroid(){
    // Initialize Bluedroid
    ESP_ERROR_CHECK(esp_bluedroid_init());

    // Enable Bluedroid
    ESP_ERROR_CHECK(esp_bluedroid_enable());
}

void configureBLE(){
    // Register GAP callback to handle advertising events
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(GAPEventHandler));

    // Set the local device name
    ESP_ERROR_CHECK(esp_ble_gap_set_device_name(CLOCK_NAME));

    // Configure the advertising data
    ESP_ERROR_CHECK(esp_ble_gap_config_adv_data(&advertisingData));

}

void GAPEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param){
    switch (event){
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT: // Wait for Advertising Data to be set
            ESP_ERROR_CHECK(esp_ble_gap_start_advertising(&advertisingParameters));
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT: // Start Advertising
            ESP_ERROR_CHECK((param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS));
            break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT: // Stop Advertising
            ESP_ERROR_CHECK((param->adv_stop_cmpl.status == ESP_BT_STATUS_SUCCESS));
            break;
        default:
            break;
    }
}