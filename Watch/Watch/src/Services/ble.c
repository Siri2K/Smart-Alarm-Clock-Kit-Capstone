#include "ble.h"

// Static Variables
struct bt_conn *connection;
static struct bt_gatt_exchange_params exchangeParameters;
struct bt_conn_le_create_param param = BT_CONN_LE_CREATE_PARAM_INIT(BT_CONN_LE_OPT_NONE, BT_GAP_SCAN_FAST_INTERVAL, BT_GAP_SCAN_FAST_WINDOW);

void connected(struct bt_conn *conn, uint8_t status){
    if(!status){
        connection = bt_conn_ref(conn); // Establish Connection
    }
}

void disconnected(struct bt_conn *conn, uint8_t status){
    if(connection){
        bt_conn_unref(connection);  // Remove Connection
        connection = NULL;
    }
}

/* Connection callbacks */
struct bt_conn_cb connCallbacks = {
    .connected = connected,
    .disconnected = disconnected,
};

/* Minimal GATT service for data transmission */
static ssize_t write_callback(struct bt_conn *conn, const struct bt_gatt_attr *attr,
    const void *buf, uint16_t len, uint16_t offset, uint8_t flags){
    return len;
}

void notifyData(collected_data_t collectedData){
    if(connection){
        bt_gatt_notify(
            connection,  
            &simple_service.attrs[1],
            collectedData,
            sizeof(collectedData)
        );
    }
}

BT_GATT_SERVICE_DEFINE(
    simple_service,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_DECLARE_16(0x180F)),
    BT_GATT_CHARACTERISTIC(
        BT_UUID_DECLARE_16(0x2A19),
        BT_GATT_CHRC_WRITE | BT_GATT_CHRC_NOTIFY,
        BT_GATT_PERM_WRITE,
        NULL,
        write_callback,
        NULL
    )
);