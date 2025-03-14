#include <stdio.h>
#include "bulb.h"

static char* sku = "H6008"; //sku of device
static char* device = "DA:40:D0:C9:07:DD:1F:C2"; //device identifier (mac address)
static char* key ="01dd14bd-df8c-465c-894a-aeddc7fe3f5f"; //api key required to send request

//might need to rewrite all code here

void bconfig(char* nsku,char*ndevice,char*nkey){
    sku=nsku;
    device=ndevice;
    key=nkey;
    return;
}
void sendRequest(bool o){
    //true=on,false=off
    
    //don't know why, typcasting didn't work properly, so here's the alternative solution
    const char *value = o ? "1" : "0";

    char post_data[512];
    snprintf(post_data, sizeof(post_data),
        "{\"requestId\": \"uuid\",\"payload\": {\"sku\": \"%s\",\"device\": \"%s\","
        "\"capability\": {\"type\": \"devices.capabilities.on_off\",\"instance\": \"powerSwitch\","
        "\"value\": %s}}}",
        sku, device, value);
    
    esp_http_client_config_t config ={
        .url ="https://openapi.api.govee.com/router/api/v1/device/control",
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_http_client_handle_t client =esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client,"Content-type","application/json");
    esp_http_client_set_header(client,"Govee-API-Key",key);

    esp_http_client_set_post_field(client,post_data,strlen(post_data));

    ESP_ERROR_CHECK(esp_http_client_perform(client));
    int disp = esp_http_client_get_status_code(client);
    disp = esp_http_client_is_chunked_response(client);
    

    ESP_ERROR_CHECK(esp_http_client_cleanup(client));
    return;
}