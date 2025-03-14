#include <stdio.h>
#include "Wifi.h"
#include "esp_log.h"
static esp_netif_t *wifi_netif=NULL;
static esp_event_handler_instance_t ip_event_handler;
static esp_event_handler_instance_t wifi_event_handler;
//static EventGroupHandle_t s_wifi_event_group = NULL;
static const int WIFI_RETRY_ATTEMPT = 3;
static int wifi_retry_count = 0;
//static wifi_config_t wifi_config;
//#define WIFI_CONNECTED_BIT BIT0
//#define WIFI_FAIL_BIT BIT1
#define TAG "wifi component"


static void ip_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
   
    switch (event_id)
    {
    case (IP_EVENT_STA_GOT_IP):
        ip_event_got_ip_t *event_ip = (ip_event_got_ip_t *)event_data;
        
        wifi_retry_count = 0;
        //xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case (IP_EVENT_STA_LOST_IP):
       
        break;
    case (IP_EVENT_GOT_IP6):
        ip_event_got_ip6_t *event_ip6 = (ip_event_got_ip6_t *)event_data;
       
        wifi_retry_count = 0;
        //xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    default:
       
        break;
    }
}

static void wifi_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{

    switch (event_id)
    {
    case (WIFI_EVENT_WIFI_READY):
        
        break;
    case (WIFI_EVENT_SCAN_DONE):
        
        break;
    case (WIFI_EVENT_STA_START):
        
        esp_wifi_connect();
        break;
    case (WIFI_EVENT_STA_STOP):
        
        break;
    case (WIFI_EVENT_STA_CONNECTED):
       
        break;
    case (WIFI_EVENT_STA_DISCONNECTED):
        
        if (wifi_retry_count < WIFI_RETRY_ATTEMPT) {
            
            esp_wifi_connect();
            wifi_retry_count++;
        } else {
            //xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        break;
    case (WIFI_EVENT_STA_AUTHMODE_CHANGE):
        
        break;
    default:
        
        break;
    }
}


void wifi_init(){
    //s_wifi_event_group = xEventGroupCreate();

    int8_t ret = 0;

    ret = esp_netif_init();
    if(ret != ESP_OK){
        ESP_LOGI(TAG, "Netif Initialzation Failed error : %d",ret);
        return;
    }

    ret = esp_event_loop_create_default();
    if(ret != ESP_OK){
        ESP_LOGI(TAG, "Event Loop Create Failed error : %d",ret);
        return;
    }

    ret = esp_wifi_set_default_wifi_sta_handlers();
    if(ret != ESP_OK){
        ESP_LOGI(TAG, "Set Default Wifi STA Handler Failed error : %d",ret);
        return;
    }

    wifi_netif = esp_netif_create_default_wifi_sta();
    if(wifi_netif == NULL){
        ESP_LOGI(TAG, "Netif Create Default Wifi STA Handler Failed error : %d",ret);
        return;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ret = esp_wifi_init(&cfg);
    if(ret != ESP_OK){
        ESP_LOGI(TAG, "Initialize Wifi Failed error : %d",ret);
        return;
    }

    ret = esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_event_cb,
        NULL,
        &wifi_event_handler
    );
    if(ret != ESP_OK){
        ESP_LOGI(TAG, "Wifi Event Handler Instance Register Failed error : %d",ret);
        return;
    }

    ret = esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &ip_event_cb,
        NULL,
        &ip_event_handler
    );
    if(ret != ESP_OK){
        ESP_LOGI(TAG, "Ip Event Handler Instance Register Failed error : %d",ret);
        return;
    }
}

void wifi_configuration(char *ssid, char *pass){
   wifi_config_t wifi_config = {
        .sta = {
            // this sets the weakest authmode accepted in fast scan mode (default)
            .threshold.authmode = WIFI_AUTH_METHOD,
        },
    };
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE)); // default is WIFI_PS_MIN_MODEM
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM)); // default is WIFI_STORAGE_FLASH
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    //REMEMBER TO HANDLE EVENT BITS LATER (BASED OFF TUTORIAL) https://developer.espressif.com/blog/getting-started-with-wifi-on-esp-idf/#part-2-using-the-wi-fi-apis

    return;
}
void wifi_start(){
    ESP_ERROR_CHECK(esp_wifi_start());
    return;
}
void wifi_disconnect(){
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    return;
}
