#include "wifiController.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "config.h"
#include "utils.h"
#include <string.h>

#define TAG "Wifi"


#define CONNECTED_BIT 1
#define FAIL_BIT      (1<<1)

EventGroupHandle_t eventGroup = NULL;

void startWifiAP(){
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    
    wifi_config_t cfg = {
        .ap = {
            .ssid = "ESP",
            .ssid_len = strlen("ESP"),
            .max_connection = 1,
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "STARTING AP");
}

void stopWifi(){
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_LOGI(TAG, "Stopped");
}

static void handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
        xEventGroupSetBits(event_handler_arg, CONNECTED_BIT);
        return;
    }

    if(event_id == WIFI_EVENT_STA_START){
        ESP_LOGI(TAG, "Started. Connecting...");
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
    else if(event_id == WIFI_EVENT_STA_CONNECTED){
        ESP_LOGI(TAG, "Connected");
    }
    else if(event_id == WIFI_EVENT_STA_DISCONNECTED){
        ESP_LOGI(TAG, "Failed to connect");
        xEventGroupSetBits(event_handler_arg, FAIL_BIT);
    }
}

bool startWifiSTA(const unsigned char* ssid, const unsigned char* pass){
    if(eventGroup == NULL)
        eventGroup = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    esp_event_handler_instance_t wifiHandler, ipHandler;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, handler, eventGroup, &wifiHandler));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, handler, eventGroup, &ipHandler));

    wifi_config_t cfg = {
        .sta={
            .bssid_set = false,
            .threshold.authmode = WIFI_AUTH_WPA_PSK,
        }
    };
    memcpy(cfg.sta.ssid, ssid, MIN(SSID_MAX_LEN, sizeof(cfg.sta.ssid)));
    memcpy(cfg.sta.password, pass, MIN(WIFI_PASS_MAX_LEN, sizeof(cfg.sta.password)));


    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));

    ESP_ERROR_CHECK(esp_wifi_start());
    //Block until we connect to wifi or we fail connect
    xEventGroupWaitBits(eventGroup, CONNECTED_BIT | FAIL_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(10000));
    bool connected = xEventGroupGetBits(eventGroup) & CONNECTED_BIT;

    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifiHandler);
    esp_event_handler_instance_unregister(IP_EVENT, ESP_EVENT_ANY_ID, ipHandler);
    
    //vEventGroupDelete(eventGroup);
    xEventGroupClearBits(eventGroup, CONNECTED_BIT | FAIL_BIT);
    return connected;
}


