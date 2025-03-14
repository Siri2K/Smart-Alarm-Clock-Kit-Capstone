#include "freertos/FreeRTOS.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
typedef enum bulb_mode{
    BULB_ON = 0,
    BULB_OFF= 1,
    BULB_CONFIG = 2,
}bulb_mode;

extern void bconfig(char* nsku,char*ndevice,char*nkey);
extern void sendRequest(bool o);