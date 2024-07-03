#ifndef CONFIGSERVER_H
#define CONFIGSERVER_H
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "config.h"

typedef struct 
{
    unsigned char ssid[SSID_MAX_LEN+1];              //NULL terminated string with WIFI SSID
    unsigned char password[WIFI_PASS_MAX_LEN+1];     //NULL terminated string with WIFI password
    char mqttURL[URL_MAX_LEN+1];            //NULL terminated string with URL of MQTT broker
    uint16_t mqttPort;                      //Port of MQTT broker
} configData_t;


httpd_handle_t startConfigServer(EventGroupHandle_t finishedSignal, configData_t* data);
void stopConfigServer(httpd_handle_t server);
#endif