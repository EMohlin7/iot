#ifndef CONFIGTASK_H
#define CONFIGTASK_H
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "mqtt_client.h"


#define CONNECTED_BIT 1
#define DISCONNECTED_BIT 2
#define RESET_BIT 4
#define FINISHED_CONFIG_BIT 8
#define STOP_CONFIG_BIT 16

typedef struct {
    EventGroupHandle_t connectedHandle;
    esp_mqtt_client_handle_t mqttClient;
} configTaskArgs_t;

void connectionTask(void* args);


#endif