#include "BLE.h"
#include "esp_log.h"
#include "esp_err.h"
#define GATTS_APP_ID    0x55
#define SERVICE_UUID    0x00FF
#define CHAR_UUID       0xFF01
static uint8_t service_uuid[2] = { (uint8_t)(SERVICE_UUID & 0xFF), (uint8_t)(SERVICE_UUID >> 8) };

static const char *TAG = CLOCK_NAME;
static esp_ble_adv_params_t advertisingParameters = {
    .adv_int_min         = 0x20, // 32 * 0.625 ms = 20 ms
    .adv_int_max         = 0x40, // 64 * 0.625 ms = 40 ms
    .adv_type            = ADV_TYPE_IND,
    .own_addr_type       = BLE_ADDR_TYPE_PUBLIC,
    .channel_map         = ADV_CHNL_ALL,
    .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static esp_ble_adv_data_t advertisingData = {
    .set_scan_rsp        = false,
    .include_name        = true,   // Include device name in advertising
    .include_txpower     = false,
    .min_interval        = 0x0020, // 32 * 0.625ms = 20ms
    .max_interval        = 0x0040, // 64 * 0.625ms = 40ms
    .appearance          = 0x00,
    .manufacturer_len    = 0,
    .p_manufacturer_data = NULL,
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = 0,
    .p_service_uuid      = NULL,
    .flag                = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};


// Variables for the GATTS server
static uint16_t service_handle = 0;
static esp_gatt_srvc_id_t service_id;
static uint16_t char_handle = 0;
// Array to hold up to two connection IDs (0xFFFF indicates an empty slot)
static uint16_t connection_ids[2] = {0xFFFF, 0xFFFF};

// Forward declaration for the GATTS event handler
static void gatts_event_handler(esp_gatts_cb_event_t event, 
                                esp_gatt_if_t gatts_if, 
                                esp_ble_gatts_cb_param_t *param);

void initializeBLE(){
    //initializeNVS();
    initializeBLEController();
    //ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    if (esp_bt_controller_enable(ESP_BT_MODE_BLE)) {
        ESP_LOGE(TAG, "%s enable controller failed\n", __func__);
        return;
    }
    initializeBluedroid();
    configureBLE();
}

// GATTS event handler supporting two simultaneous connections
static void gatts_event_handler(esp_gatts_cb_event_t event, 
    esp_gatt_if_t gatts_if, 
    esp_ble_gatts_cb_param_t *param) {
switch (event) {
case ESP_GATTS_REG_EVT: {
ESP_LOGI(TAG, "GATTS_REG_EVT, app_id %d", param->reg.app_id);
// Set up the primary service ID
service_id.is_primary = true;
service_id.id.inst_id = 0x00;
service_id.id.uuid.len = ESP_UUID_LEN_16;
service_id.id.uuid.uuid.uuid16 = SERVICE_UUID;
// Create the service with 4 attributes (service, char declaration, char value, and optional descriptor)
esp_ble_gatts_create_service(gatts_if, &service_id, 4);
break;
}
case ESP_GATTS_CREATE_EVT: {
service_handle = param->create.service_handle;
ESP_LOGI(TAG, "Service created, handle %d", service_handle);
// Start the service
esp_ble_gatts_start_service(service_handle);
// Add a characteristic to the service
esp_bt_uuid_t char_uuid;
char_uuid.len = ESP_UUID_LEN_16;
char_uuid.uuid.uuid16 = CHAR_UUID;
esp_ble_gatts_add_char(service_handle, &char_uuid,
   ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
   ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
   NULL, NULL);
break;
}
case ESP_GATTS_ADD_CHAR_EVT: {
char_handle = param->add_char.attr_handle;
ESP_LOGI(TAG, "Characteristic added, handle %d", char_handle);
break;
}
case ESP_GATTS_CONNECT_EVT: {
uint16_t conn_id = param->connect.conn_id;
ESP_LOGI(TAG, "Device connected, conn_id %d", conn_id);
// Save the connection ID in the first available slot
for (int i = 0; i < 2; i++) {
if (connection_ids[i] == 0xFFFF) {
connection_ids[i] = conn_id;
break;
}
}
    // Count current connections
    int num_conns = 0;
    for (int i = 0; i < 2; i++) {
        if (connection_ids[i] != 0xFFFF)
            num_conns++;
    }
    if (num_conns < 2) {
        esp_ble_gap_start_advertising(&advertisingParameters);
    }
break;
}
case ESP_GATTS_DISCONNECT_EVT: {
uint16_t conn_id = param->disconnect.conn_id;
ESP_LOGI(TAG, "Device disconnected, conn_id %d", conn_id);
// Clear the connection ID slot for the disconnected device
for (int i = 0; i < 2; i++) {
if (connection_ids[i] == conn_id) {
connection_ids[i] = 0xFFFF;
break;
}
}
// Restart advertising so new devices can connect
esp_ble_gap_start_advertising(&advertisingParameters);
break;
}
default:
break;
}
}


void initializeNVS(){
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /*uint8_t i=0;
    while (++i<=10){ // Limit to 10 attemtps
        if(nvs_flash_init() == ESP_OK){
            break;
        }
        ESP_ERROR_CHECK(nvs_flash_erase());
    }*/

    // Limit memory to BLE only
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    
}

void initializeBLEController(){
    // Initialize the Bluetooth Controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    
    if (esp_bt_controller_init(&bt_cfg)) {
        ESP_LOGE(TAG, "%s initialize controller failed\n", __func__);
        return;
    }
}

void initializeBluedroid(){
    // Initialize Bluedroid
    
    if (esp_bluedroid_init()) {
        ESP_LOGE(TAG, "%s init bluetooth failed\n", __func__);
        return;
    }
    if (esp_bluedroid_enable()) {
        ESP_LOGE(TAG, "%s enable bluetooth failed\n", __func__);
        return;
    }
}

void configureBLE(){
    // Register GAP callback to handle advertising events
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(GAPEventHandler));

    // Set the local device name
    ESP_ERROR_CHECK(esp_ble_gap_set_device_name(CLOCK_NAME));

    // Configure the advertising data
    esp_err_t config_adv_data_ret = (esp_ble_gap_config_adv_data(&advertisingData));
    if (config_adv_data_ret){
        ESP_LOGE(TAG, "config adv data failed, error code = %x", config_adv_data_ret);
    }

    ESP_LOGI(TAG, "BLE initialized. Configuring advertisement...");
    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_event_handler));

    ESP_ERROR_CHECK(esp_ble_gatts_app_register(GATTS_APP_ID));


}

static void GAPEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param){
    switch (event){
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT: // Wait for Advertising Data to be set
        ESP_LOGI(TAG, "Advertising data set complete, start advertising");
        esp_ble_gap_start_advertising(&advertisingParameters);
        break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT: // Start Advertising
        if(param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS){
            ESP_LOGE(TAG, "Advertising start failed: %d", param->adv_start_cmpl.status);
        } else {
            ESP_LOGI(TAG, "Advertising started successfully");
        }
        break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT: // Stop Advertising
        if(param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
            ESP_LOGE(TAG, "Advertising stop failed: %d", param->adv_stop_cmpl.status);
        }
        break;
        default:
            break;
    }
}