#ifndef BLE_H
#define BLE_H

/* Libraries */
// Zephyr & BLE
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/hci.h>
#include <bluetooth/gatt_dm.h>
#include <bluetooth/scan.h>

#include <stdbool.h>


// Scan parameters
#define SCAN_INTERVAL 0x0010
#define SCAN_WINDOW 0x0010

#define TARGET_MAC "A0:A3:B3:96:EF:EA"
// #define TARGET_MAC "5C:28:F1:1D:9A:57"

/* Global */
// Structure
typedef struct collected_data_t{
    uint8_t bpm;
    uint8_t vx,vy,vz;
}collected_data_t;

extern bool ble_ready;
extern collected_data_t collectedData;
extern int8_t connectedBLE;
extern int8_t discoverCCC;
extern struct bt_conn *current_conn;
extern char data_string[50];

/* Tasks */
/* Initialize BLE */
extern int8_t initializeBLE();

/* Start scanning only to the specified device */
extern int8_t startScanningBLE();
extern void findDevice(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type, struct net_buf_simple *ad);

/* Send collected data over BLE */
// extern int8_t send_data_to_server();
extern void sendBLEData(struct bt_conn *conn, uint8_t *data);

#endif 