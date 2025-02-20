#ifndef BLE_H
#define BLE_H

/* Zephyr library */
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>

/* Headers */
#include "../Component/Accelerometer/accelerometer.h" // FOr the Accelerometer Variable

/* Globals */
typedef struct collected_data_t{
    accelerometer_data_t *accelerometer; // Accelerometer
    uint8_t bpm; // HeartRate
}collected_data_t;

extern struct bt_conn *connection;
extern struct bt_conn_cb connCallbacks;
extern struct bt_conn_le_create_param param;

extern void connected(struct bt_conn *conn, uint8_t err);

extern void disconnected(struct bt_conn *conn, uint8_t err);

extern void notifyData();

#endif