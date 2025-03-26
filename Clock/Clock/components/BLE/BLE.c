#include "BLE.h"
#include "esp_log.h"
#include "esp_err.h"


#define GATTS_APP_ID    0x55
uint8_t SERVICE_UUID[16];
uint8_t PHONE_CHAR_UUID[16];
uint8_t WATCH_CHAR_UUID[16];

const char *service_uuid_str = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
const char *phone_char_uuid_str = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";
const char *watch_char_uuid_str = "6E400005-B5A3-F393-E0A9-E50E24DCCA9E";

esp_bd_addr_t connectedDeviceAddress[2];
int8_t num_conns = 0;

uint8_t storedWatchData[6];
uint8_t sleepState = 1;

// Setup Code
char storedPhoneData0[64]; 
char storedPhoneData1[64];
char alarmTime[8];
char WifiSSID[64] = " ";
char WifiPassword[64] = " "; 
char WifiMacAddress[18] = " ";
char WifiSku[64] = " ";
char WifiApiKey[64] = " ";


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
    .min_interval        = 0x20, // 32 * 0.625ms = 20ms
    .max_interval        = 0x40, // 64 * 0.625ms = 40ms
    .appearance          = 0x00,
    .manufacturer_len    = 0,
    .p_manufacturer_data = NULL,
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = sizeof(SERVICE_UUID),  // Set UUID length,
    .p_service_uuid      = SERVICE_UUID,  // Set the service UUID,
    .flag                = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};


// Variables for the GATTS server
static uint16_t service_handle = 0;
static esp_gatt_srvc_id_t service_id;
static uint16_t char_handle = 0;
// Array to hold up to two connection IDs (0xFFFF indicates an empty slot)
uint16_t connection_ids[2] = {0xFFFF, 0xFFFF};
uint16_t connection_handles[2] = {0xFFFF, 0xFFFF};
// Forward declaration for the GATTS event handler
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

static void storeStringData(char* inputBuffer, char*outBuffer){
    if(inputBuffer != NULL){
        strncpy(outBuffer, inputBuffer, sizeof(inputBuffer)); // Save first string
        inputBuffer = strtok(NULL, ",");
    }
}

static int string_to_uuid(const char *uuid_str, uint8_t *uuid) {
    // Ensure that the UUID string is correctly formatted (36 characters long)
    if (uuid_str == NULL || strlen(uuid_str) != 36) {
        ESP_LOGE(TAG,"Invalid UUID string\n");
        return -22;
    }

    // Parse the string into the UUID
    uint32_t values[4];  // To hold the first 4 parts (8-byte, 4-byte, 4-byte, 4-byte)
    uint64_t last_part;  // To hold the last 12-byte part
    int parsed = sscanf(uuid_str,
                         "%8lx-%4lx-%4lx-%4lx-%12llx",
                         &values[0], &values[1], &values[2], &values[3], &last_part);

    if (parsed != 5) {
        ESP_LOGE(TAG,"Invalid UUID string\n");
        return -22;
    }

    // Print parsed values for debugging
    printf("Parsed values: [0] 0x%08lx, [1] 0x%04lx, [2] 0x%04lx, [3] 0x%04lx, [4] 0x%012llx\n",
            values[0], values[1], values[2], values[3], last_part);

    // Fill the UUID structure with the parsed values (note that the function is little-endian)
    uuid[15] = (uint8_t)(values[0] >> 24);
    uuid[14] = (uint8_t)(values[0] >> 16);
    uuid[13] = (uint8_t)(values[0] >> 8);
    uuid[12] = (uint8_t)(values[0]);

    uuid[11] = (uint8_t)(values[1] >> 8);
    uuid[10] = (uint8_t)(values[1]);

    uuid[9] = (uint8_t)(values[2] >> 8);
    uuid[8] = (uint8_t)(values[2]);

    uuid[7] = (uint8_t)(values[3] >> 8);
    uuid[6] = (uint8_t)(values[3]);

    // For the last 12-byte portion, handle it as a 48-bit number
    uuid[5] = (uint8_t)(last_part >> 40); // High byte
    uuid[4] = (uint8_t)(last_part >> 32);
    uuid[3] = (uint8_t)(last_part >> 24);
    uuid[2] = (uint8_t)(last_part >> 16);
    uuid[1] = (uint8_t)(last_part >> 8);
    uuid[0] = (uint8_t)(last_part); // Low byte

    return 0;
}

void print_uuids() {
    ESP_LOGI(TAG,"Sensor UUID: %s\n", SERVICE_UUID);
    printf("\n");
    // Sensor Characteristic UUID
    for(int i=0;i<16;i++){
        printf("0x%02X ", PHONE_CHAR_UUID[15-i]);
    }
    printf("\n");

    for(int i=0;i<16;i++){
        printf("0x%02X ", WATCH_CHAR_UUID[15-i]);
    }
    printf("\n");
}

void initializeBLE(){
    // Initialize Service UUID
    string_to_uuid(service_uuid_str, SERVICE_UUID);
    string_to_uuid(phone_char_uuid_str, PHONE_CHAR_UUID);
    string_to_uuid(watch_char_uuid_str, WATCH_CHAR_UUID);
    print_uuids();

    // 1) Clear BLE Memory
    esp_err_t status = ESP_OK;
    status = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if(status != ESP_OK){
        if(status == ESP_ERR_INVALID_STATE || ESP_ERR_NOT_FOUND){
            ESP_LOGE(TAG, "%s Resource Invalid or Not Found, Error : %s", __func__,esp_err_to_name(status));
        }
        else{
            ESP_LOGE(TAG, "%s Failed due to Error : %s", __func__,esp_err_to_name(status));
        }
        return;
    }

    // 2) Initialize BLE Controller
    int8_t timeout = 0;
    initializeBLEController();
    while (esp_bt_controller_get_status() != ESP_BT_CONTROLLER_STATUS_ENABLED){
        vTaskDelay(100/portTICK_PERIOD_MS);
        if(timeout++ > 50){
            ESP_LOGE(TAG, "%s BLE COntroller Enabele Timeout: ", __func__);
            return;
        }
    }
    
    // 3) Intiialize Bluedroid
    initializeBluedroid();
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
        case ESP_GATTS_REG_EVT: {
            ESP_LOGI(TAG, "GATTS_REG_EVT, app_id %d", param->reg.app_id);
            // Set up the primary service ID
            service_id.is_primary = true;
            service_id.id.inst_id = 0x00;
            service_id.id.uuid.len = ESP_UUID_LEN_128;
            memcpy(service_id.id.uuid.uuid.uuid128, SERVICE_UUID, 16); // Set 128-bit UUID

            // Set the local device name
            ESP_ERROR_CHECK(esp_ble_gap_set_device_name(CLOCK_NAME));

            // Configure the advertising data
            esp_err_t config_adv_data_ret = (esp_ble_gap_config_adv_data(&advertisingData));
            if (config_adv_data_ret){
                ESP_LOGE(TAG, "config adv data failed, error code = %x", config_adv_data_ret);
            }

            // Create the service with 4 attributes (service, char declaration, char value, and optional descriptor)
            esp_ble_gatts_create_service(gatts_if, &service_id, 8);

            // Start advertising after registering the service
            esp_ble_gap_start_advertising(&advertisingParameters);
            break;
        }
        case ESP_GATTS_CREATE_EVT: {
            service_handle = param->create.service_handle;
            ESP_LOGI(TAG, "Service created, handle %d", service_handle);
            
            // Start the service
            esp_ble_gatts_start_service(service_handle);
            
            // Set UUID for phone and watch characteristics
            esp_bt_uuid_t phoneCharacteristicUUID, watchCharacteristicUUID; 
            phoneCharacteristicUUID.len = ESP_UUID_LEN_128;
            watchCharacteristicUUID.len = ESP_UUID_LEN_128;
            
            memcpy(phoneCharacteristicUUID.uuid.uuid128, &PHONE_CHAR_UUID, 16);  // Set Phone Characteristic UUID
            memcpy(watchCharacteristicUUID.uuid.uuid128, &WATCH_CHAR_UUID, 16);  // Set Watch Characteristic UUID
        
        ESP_LOGI(TAG, "Adding Phone Characteristic");
            // Add the phone characteristic
            esp_err_t ret = esp_ble_gatts_add_char(
                service_handle, 
                &phoneCharacteristicUUID,
                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                NULL, NULL
            );
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to add phone characteristic, error code %d", ret);
            } 
            else {
                ESP_LOGI(TAG, "Phone characteristic added successfully");
            }

            ESP_LOGI(TAG, "Adding Watch Characteristic");
            // Add the watch characteristic
            ret = esp_ble_gatts_add_char(
                service_handle, 
                &watchCharacteristicUUID,
                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                NULL, NULL
            );
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to add watch characteristic, error code %d", ret);
            } 
            else {
                ESP_LOGI(TAG, "Watch characteristic added successfully");
            }

            break;
        }       
        case ESP_GATTS_WRITE_EVT: {
            /*
            if(param->write.len == sizeof(storedWatchData)/sizeof(storedWatchData[0])-2){
                for (int i = 0; i < param->write.len; i++) {
                    storedWatchData[i] = param->write.value[i];
                }
            }
            */

            if(param->write.conn_id == ((connectedDeviceAddress[0] == WATCH_MAC)?connection_ids[0]:connection_ids[1])){
                
                ESP_LOGI(TAG, "Received Watch Data");
                for (int i = 0; i < param->write.len; i++) {
                    storedWatchData[i] = param->write.value[i];
                }
            }
            else if(param->write.conn_id != ((connectedDeviceAddress[0] == WATCH_MAC)?connection_ids[0]:connection_ids[1])){
                ESP_LOGI(TAG, "Received Phone Data");

                // Assuming incoming data is comma-separated: "string1,string2,string3,int"
                char receivedData[300];
                sniprintf(receivedData, sizeof(receivedData), "%s", param->write.value); // Grabbing Incoming Data

                 // Log the received data for debugging
                ESP_LOGI(TAG, "Received Phone Data: %s", receivedData);
                int tokenCount = 0;

                // Count Number of Tokens
                char* inputToken = strtok(receivedData,",");
                while (inputToken)
                {
                    tokenCount++;

                    ESP_LOGI(TAG, "Input Token : %s", inputToken);
                    ESP_LOGI(TAG,  "Token Count : %d", tokenCount);

                    if(tokenCount == 1){
                        storeStringData(inputToken,storedPhoneData0);
                    }
                    else if(tokenCount == 2){
                        storeStringData(inputToken,storedPhoneData1);
                    }
                    else if(tokenCount == 3){
                        storeStringData(inputToken,WifiMacAddress);
                    }
                    else if(tokenCount == 4){
                        storeStringData(inputToken,WifiSku);
                    }
                    else if(tokenCount == 5){
                        storeStringData(inputToken,WifiApiKey);
                    }

                    
                    inputToken = strtok(NULL, ",");
                    ESP_LOGI(TAG, "Remaining String: %s", receivedData);  // Log the remaining string after each token extraction
                }

                ESP_LOGI(TAG, "Token Count : %d", tokenCount);

                // Store Data
                if(tokenCount == 1){
                    storeStringData(storedPhoneData0,alarmTime); 
                    ESP_LOGI(TAG, "alarmTime: %s",WifiSSID); 
                }
                else if(tokenCount == 5){
                    storeStringData(storedPhoneData0,WifiSSID);  
                    storeStringData(storedPhoneData1,WifiPassword);  

                    // Log the received SSID, Password, MAC Address, SKU, API Key
                    ESP_LOGI(TAG, "Received SSID: %s, Password: %s, MAC Address: %s, SKU: %s, API Key: %s",
                        WifiSSID, WifiPassword, WifiMacAddress, WifiSku, WifiApiKey);
                }
                else{
                    // Handle unexpected number of tokens
                    ESP_LOGI(TAG, "Unexpected data format received.");
                }
            }
            
            // Example Log the received data
            /*
            ESP_LOGI(TAG, "Received from: ");
            for (int i = 0; i < 6; i++) {
                ESP_LOGI(TAG, "%02X ", param->write.bda[i]);
            }
            ESP_LOGI(TAG, "Number of Connections: %d",num_conns);
            ESP_LOGI(TAG, "Received data: ");
            for (int i = 0; i < param->write.len; i++) {
                ESP_LOGI(TAG, "%02X ", param->write.value[i]);
            }
            ESP_LOGI(TAG, "\n");
            */
            

            int8_t sameAddress = 1;
            for(int i=0;i<6;i++){
                sameAddress &= (param->write.bda[i] == WATCH_MAC[i])?1:0;
            }

            if(sameAddress && num_conns == 2){
                esp_err_t dataSent = ESP_OK; 

                char sentData[18];
                snprintf(sentData,sizeof(sentData),"%02d,%02d,%02d,%d",storedWatchData[0],storedWatchData[4],storedWatchData[5],sleepState);
                
                // Check Data Sent
                ESP_LOGI(TAG, "Connection ID: [%d,%d]", connection_ids[0],connection_ids[1]);
                ESP_LOGI(TAG, "Connection Handle: [%d,%d]", connection_handles[0],connection_handles[1]);

                dataSent = esp_ble_gatts_send_indicate(
                    gatts_if,
                    (connectedDeviceAddress[0] != WATCH_MAC)?connection_ids[0]:connection_ids[1],
                    connection_handles[0],
                    strlen(sentData),
                    (uint8_t*)sentData,
                    false
                );

                if (dataSent != ESP_OK) {
                    ESP_LOGI(TAG, "Sending Data to Phone Failed\n");
                } else {
                    ESP_LOGI(TAG, "Data Sent to Phone : %s\n", sentData);
                }
                
            }
            break;
        }
        case ESP_GATTS_ADD_CHAR_EVT: {
            char_handle = param->add_char.attr_handle;
            for (int i = 0; i < 2; i++) {
                if (connection_handles[i] == 0xFFFF){
                    connection_handles[i] = char_handle;
                    ESP_LOGI(TAG, "Characteristic added, handle %d", connection_handles[i]);
                    break;
                }
                ESP_LOGI(TAG, "Characteristic added, handle %d", char_handle);        
            }
            
            break;
        }
        case ESP_GATTS_CONNECT_EVT:
            uint16_t conn_id = param->connect.conn_id;
            ESP_LOGI(TAG, "Device connected, conn_id %d", param->connect.conn_id);
            // Save the connection ID in the first available slot
            for (int i = 0; i < 2; i++) {
                if (connection_ids[i] == 0xFFFF) {
                    connection_ids[i] = conn_id;
                    memcpy(connectedDeviceAddress[i], param->connect.remote_bda, sizeof(esp_bd_addr_t));

                    ESP_LOGI(TAG, "Stored MAC address num %d: %02X:%02X:%02X:%02X:%02X:%02X", i,
                        connectedDeviceAddress[i][0], connectedDeviceAddress[i][1], connectedDeviceAddress[i][2], 
                        connectedDeviceAddress[i][3], connectedDeviceAddress[i][4], connectedDeviceAddress[i][5]
                    );

                    break;
                }
            }
            // Count current connections
            num_conns = 0;
            for (int i = 0; i < 2; i++) {
                if (connection_ids[i] != 0xFFFF){
                    num_conns++;
                }
            }
            if (num_conns < 2) {
                esp_ble_gap_start_advertising(&advertisingParameters);
            }
            else{
                esp_ble_gap_stop_advertising();
            }
            break;
        case ESP_GATTS_DISCONNECT_EVT: {
            uint16_t conn_id = param->disconnect.conn_id;
            ESP_LOGI(TAG, "Device disconnected, conn_id %d", conn_id);
            // Clear the connection ID slot for the disconnected device
            for (int i = 0; i < 2; i++) {
                if (connection_ids[i] == conn_id) {
                    connection_ids[i] = 0xFFFF;
                    num_conns--;
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

void initializeNVS(){
    // Initialize NVS WifiPassword, WifiMacAddress, WifiSku, WifiApiKey
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
    esp_err_t status = ESP_OK;

    // Intialize Controller
    if(esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE){
        status = esp_bt_controller_init(&bt_cfg);
    }    
    if(status != ESP_OK){
        if(status == ESP_ERR_INVALID_STATE || ESP_ERR_NOT_FOUND){
            ESP_LOGE(TAG, "%s esp_bt_controller_init due to Resource Invalid or Not Found, Error : %s", __func__,esp_err_to_name(status));
        }
        else{
            ESP_LOGE(TAG, "%s esp_bt_controller_init Failed due to Error : %s", __func__,esp_err_to_name(status));
        }
        return;
    }

    // Enable Controller
    if(esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED){
        status = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    }
    if(status != ESP_OK){
        if(status == ESP_ERR_INVALID_STATE || ESP_ERR_NOT_FOUND){
            ESP_LOGE(TAG, "%s esp_bt_controller_enable Resource Invalid or Not Found, Error : %s", __func__,esp_err_to_name(status));
        }
        else{
            ESP_LOGE(TAG, "%s esp_bt_controller_enable Failed due to Error : %s", __func__,esp_err_to_name(status));
        }
        return;
    }
}

void initializeBluedroid(){
    // Initialize Bluedroid
    esp_err_t status = ESP_OK;
    status = esp_bluedroid_init();
    if(status != ESP_OK){
        ESP_LOGE(TAG, "%s esp_bluedroid_init Failed due to Error : %s", __func__,esp_err_to_name(status));
        return;
    }

    // Enable Controller
    status = esp_bluedroid_enable();
    if(status != ESP_OK){
        ESP_LOGE(TAG, "%s  esp_bluedroid_enable Failed due to Error : %s", __func__,esp_err_to_name(status));
        return;
    }
}

void configureBLE(){
    // Initialize Gatt and Gap
    esp_err_t status = ESP_OK;

    status = esp_ble_gap_register_callback(GAPEventHandler);
    if(status != ESP_OK){
        ESP_LOGE(TAG, "%s GAPEventHandler Failed due to Error : %s", __func__,esp_err_to_name(status));
        return;
    }

    status = esp_ble_gatts_register_callback(gatts_event_handler);
    if(status != ESP_OK){
        ESP_LOGE(TAG, "%s gatts_event_handler Failed due to Error : %s", __func__,esp_err_to_name(status));
        return;
    }

    status = esp_ble_gatts_app_register(GATTS_APP_ID);
    if(status != ESP_OK){
        ESP_LOGE(TAG, "%s GATTS_APP_ID Failed due to Error : %s", __func__,esp_err_to_name(status));
        return;
    }
    
}