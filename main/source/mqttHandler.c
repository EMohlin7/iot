#include "mqttHandler.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "config.h"
#include <string.h>
#include "freertos/event_groups.h"


#define TAG "MQTT"
#define edata ((esp_mqtt_event_t*)event_data)

#define moveStringData(destName, src, len) \
    char destName[len+1];\
    destName[len] = 0; \
    memcpy(destName, src, len);


static esp_mqtt_client_config_t mqttConfig;
static EventGroupHandle_t eventGroup = NULL;

#define CONNECTED_BIT 1
#define FAIL_BIT (1<<1)

static void beforeConnect(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    ESP_LOGI(TAG, "Connecting to broker...");
}

static void connected(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    ESP_LOGI(TAG, "Successfully connected to broker");
    if(eventGroup != NULL)
        xEventGroupSetBits(eventGroup, CONNECTED_BIT);
    esp_mqtt_client_subscribe_single(edata->client, "test", 1);
}

static void disconnected(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    if(eventGroup != NULL)
        xEventGroupSetBits(eventGroup, FAIL_BIT);
    ESP_LOGI(TAG, "Disconnected from broker");
}

static void error(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    ESP_LOGE(TAG, "An error occured");
}

static void subscribed(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    ESP_LOGI(TAG, "Subscribed");
    ESP_LOGI(TAG, "Packet id %d", esp_mqtt_client_enqueue(edata->client, "test", "hejsan", 7, 0, 0, true));
}

static void unsubscribed(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    ESP_LOGI(TAG, "Unsubscribed");
}

static void received(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    moveStringData(data, edata->data, edata->data_len);
    moveStringData(topic, edata->topic, edata->topic_len);

    ESP_LOGI(TAG, "Received msg %s on topic %s", data, topic);
}



static void registerEvents(esp_mqtt_client_handle_t client){

    esp_mqtt_client_register_event(client, MQTT_EVENT_BEFORE_CONNECT, beforeConnect, client);
    esp_mqtt_client_register_event(client, MQTT_EVENT_CONNECTED, connected, client);
    esp_mqtt_client_register_event(client, MQTT_EVENT_DISCONNECTED, disconnected, client);

    esp_mqtt_client_register_event(client, MQTT_EVENT_ERROR, error, client);
    esp_mqtt_client_register_event(client, MQTT_EVENT_SUBSCRIBED, subscribed, client);
    esp_mqtt_client_register_event(client, MQTT_EVENT_UNSUBSCRIBED, unsubscribed, client);
    esp_mqtt_client_register_event(client, MQTT_EVENT_DATA, received, client);

}

esp_mqtt_client_handle_t createMqttClient(const char* clientId){
    mqttConfig.broker.address.uri = NULL;
    mqttConfig.broker.address.transport = MQTT_TRANSPORT_OVER_TCP;
    mqttConfig.broker.address.path = "/";


    mqttConfig.session.protocol_ver = MQTT_PROTOCOL_V_3_1_1;
    mqttConfig.network.disable_auto_reconnect = true;
    mqttConfig.credentials.client_id = clientId;
    mqttConfig.credentials.set_null_client_id = clientId == NULL ? true : false;

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqttConfig);
    registerEvents(client);
    return client;
}

bool startMqtt(esp_mqtt_client_handle_t mqttClient, const char* brokerUri, uint16_t brokerPort){
    eventGroup = xEventGroupCreate();

    mqttConfig.broker.address.hostname = brokerUri;
    mqttConfig.broker.address.port = brokerPort;
    esp_mqtt_set_config(mqttClient, &mqttConfig);
    ESP_ERROR_CHECK(esp_mqtt_client_start(mqttClient));    

    xEventGroupWaitBits(eventGroup, CONNECTED_BIT | FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    bool connected = xEventGroupGetBits(eventGroup) & CONNECTED_BIT;
    vEventGroupDelete(eventGroup);
    eventGroup = NULL;
    if(!connected)
        esp_mqtt_client_stop(mqttClient);
    return connected;
}

void stopMqtt(esp_mqtt_client_handle_t mqttClient){
    esp_mqtt_client_stop(mqttClient);
}

