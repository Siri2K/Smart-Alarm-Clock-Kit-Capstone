#ifndef WIFI_H
#define WIFI_H
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#define WIFI_AUTH_METHOD WIFI_AUTH_WPA2_PSK

extern void wifi_init();
extern void wifi_configuration(char* ssid, char* pass);
extern void wifi_start();
extern void wifi_disconnect();
#endif