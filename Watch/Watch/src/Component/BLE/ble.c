#include "ble.h"

/* Data Attribute */
// UUID

static struct bt_uuid_128 SENSOR_UUID;
static struct bt_uuid_128 SENSOR_CHRC_UUID;

static const char *sensor_uuid_str = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
static const char *sensor_characteristic_uuid_str = "6E400005-B5A3-F393-E0A9-E50E24DCCA9E";

// Data
int8_t connectedBLE = 0;
collected_data_t collectedData;
char data_string[50];

struct bt_conn *current_conn = NULL;
struct bt_gatt_attr *gatt_chrc = NULL;

struct bt_gatt_discover_params discover_params;
struct bt_gatt_subscribe_params subscribe_params;

static int string_to_uuid_128(const char *uuid_str, struct bt_uuid_128 *uuid){
    // Ensure that the UUID string is correctly formatted (36 characters long)
    if (uuid_str == NULL || strlen(uuid_str) != 36) {
        return -EINVAL;
    }

    // Parse the string into the UUID
    uint32_t values[8];
    uint64_t last_part;  // To hold the last 12-byte part
    int parsed = sscanf(uuid_str,
                         "%8x-%4x-%4x-%4x-%12llx",
                         &values[0], &values[1], &values[2], &values[3], &last_part);

    if (parsed != 5) {
        return -EINVAL;
    }

    // Setup UUID Type
    uuid->uuid.type = BT_UUID_TYPE_128;

    // Fill the UUID structure with the parsed values (note that the function is little-endian)
    
    uuid->val[15] = (uint8_t)(values[0] >> 24);
    uuid->val[14] = (uint8_t)(values[0] >> 16);
    uuid->val[13] = (uint8_t)(values[0] >> 8);
    uuid->val[12] = (uint8_t)(values[0]);

    uuid->val[11] = (uint8_t)(values[1] >> 8);
    uuid->val[10] = (uint8_t)(values[1]);

    uuid->val[9] = (uint8_t)(values[2] >> 8);
    uuid->val[8] = (uint8_t)(values[2]);

    uuid->val[7] = (uint8_t)(values[3] >> 8);
    uuid->val[6] = (uint8_t)(values[3]);

    // For the last 12-byte portion, handle it as a 48-bit number
    uuid->val[5] = (uint8_t)(last_part >> 40); // High byte
    uuid->val[4] = (uint8_t)(last_part >> 32);
    uuid->val[3] = (uint8_t)(last_part >> 24);
    uuid->val[2] = (uint8_t)(last_part >> 16);
    uuid->val[1] = (uint8_t)(last_part >> 8);
    uuid->val[0] = (uint8_t)(last_part); // Low byte
    
    return 0;
}

static uint8_t gattDiscoverCallback(struct bt_conn *conn, const struct bt_gatt_attr *attr, struct bt_gatt_discover_params *params){
    if (!attr) {
        return BT_GATT_ITER_STOP;
    }

    char str[37];
    bt_uuid_to_str(attr->uuid,str,sizeof(str));

    if (bt_uuid_cmp(attr->uuid, (const struct bt_uuid *)&SENSOR_CHRC_UUID)) {
        subscribe_params.value_handle = attr->handle;

        uint8_t data[4] = {
            collectedData.bpm,
            collectedData.vx,
            collectedData.vy,
            collectedData.vz,
        };
        sendBLEData(conn,data);
        return BT_GATT_ITER_STOP;
    }

    return BT_GATT_ITER_CONTINUE;
}

static void connected(struct bt_conn *conn, uint8_t err) {
    if (err) {
        if(current_conn != NULL){
            bt_conn_unref(current_conn);
            current_conn = NULL;
        }
        connectedBLE = 0;
        bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
        return;
    }

    if(current_conn == NULL){
        current_conn = bt_conn_ref(conn);
        current_conn = conn;
    }
    connectedBLE = 1;

    // Start service discovery
    discover_params.uuid = (const struct bt_uuid *)&SENSOR_UUID;
    discover_params.func = gattDiscoverCallback;
    discover_params.start_handle = 0x0001;
    discover_params.end_handle = 0xFFFF;
    bt_gatt_discover(conn,&discover_params);
}

// Callback function for disconnection
static void disconnected(struct bt_conn *conn, uint8_t reason) {
    if(current_conn != NULL){
        bt_conn_unref(current_conn);
        current_conn = NULL;
    }
    connectedBLE = 0;
    k_msleep(1000);
}

// Connection parameters
struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
};

/* BLE Initialization */
static void connectDevice(const bt_addr_le_t *addr){
    int8_t status = 0;
    
    // Create Connection Parameter
    static const struct bt_le_conn_param conn_params = {
        .interval_min = 0x0010,  // Correct the parameters here
        .interval_max = 0x0020,
        .latency = 0,
        .timeout = 400,
    };

    status = bt_conn_le_create(
        addr, 
        BT_CONN_LE_CREATE_CONN,
        &conn_params,
        &current_conn
    );
    if (status != 0) {
		status = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
		if (status != 0) {
			return;
		}
	}
}

void findDevice(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type, struct net_buf_simple *ad) {
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));

    // Check if the device matches the target MAC address
    if (strncmp(addr_str, TARGET_MAC,17) == 0) {
        // Stop scanning and initiate connection
        bt_le_scan_stop();
        k_msleep(1000);
        connectDevice(addr);
    } 
}


int8_t initializeBLE() {
    int status;
    status = bt_enable(NULL);
    if(status != 0){
        return status;
    }

    // Initialize Collected Data
    collectedData.bpm = 0;
    collectedData.vx = 0;
    collectedData.vy = 0;
    collectedData.vz = 0;

    // Configure UUID's
    string_to_uuid_128(sensor_uuid_str,&SENSOR_UUID);
    string_to_uuid_128(sensor_characteristic_uuid_str,&SENSOR_CHRC_UUID);
    bt_conn_cb_register(&conn_callbacks);

    return 0;
}

/* Start Directed Advertising */
int8_t startScanningBLE(){
    // Setup Scan parameters
    struct bt_le_scan_param scan_params = {
        .type = BT_LE_SCAN_TYPE_ACTIVE,
        .interval = SCAN_INTERVAL,
        .window = SCAN_WINDOW,
    };

    int8_t status = 0;
    if (connectedBLE == 0) {
        status = bt_le_scan_start(&scan_params, findDevice);
        if (status != 0) {
            return status;
        }
    }
    return 0;
}

void sendBLEData(struct bt_conn *conn, uint8_t *data){
    // Send the data to the characteristic
    uint16_t handle = 44;
    bt_gatt_write_without_response(conn,handle,data,sizeof(data),0);
}