#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "nvs_flash.h" 
#include "esp_wifi.h"

#include "wifiController.h"
#include "mqttHandler.h"
#include "config.h"
#include "utils.h"
#include "connection.h"
#include "sensors.h"
#include "dht11.h"
#include "HAconfig/HApayloads.h"
#include "power.h"

#define TAG "Main"
#define SENS_QUEUE_LEN 5


static StaticEventGroup_t staticConnectedBits;
static StaticQueue_t staticSensorQueue;
static uint8_t staticSensorQueueBuffer[SENS_QUEUE_LEN*sizeof(sensorReading_t)];

EventGroupHandle_t connectedEvent;
esp_mqtt_client_handle_t mqttClient;

static int intToAscii(int num, char* a, int len){
    char tmp[len];
    int i = len;
    while(num > 0 && i >= 0){
        --i;
        tmp[i] = (num % 10) + 48;
        num /= 10;
    }
    if(i == len)
        return 0;
    memcpy(a, tmp+i, len-i);
    a[len-i] = 0;
    return len-i;
}

static void powerISR(void* args){
    DEBOUNCE
    setPower(!getPower(0).power, false, 0);
}

static void modeISR(void* args){
    DEBOUNCE
    switchAutoMode(0);
}

static void received(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    esp_mqtt_event_t* data = (esp_mqtt_event_t*)event_data;
    char msg[data->data_len+1];
    msg[data->data_len] = 0; 
    memcpy(msg, data->data, data->data_len);

    char topic[data->topic_len+1];
    topic[data->topic_len] = 0;
    memcpy(topic, data->topic, data->topic_len);

    TickType_t maxWait = pdMS_TO_TICKS(5000); 
    //Check if we received "offline" on the availability topic and send "online instead". For some reason the last will gets sent when reconnecting too fast 
    if(strncmp(topic, HA_AVAILABILITY_TOPIC, MIN(data->topic_len, sizeof(HA_AVAILABILITY_TOPIC))) == 0){
        if(strncmp(msg, PAYLOAD_NOT_AVAILABLE, MIN(data->data_len, sizeof(PAYLOAD_NOT_AVAILABLE))) == 0){
            esp_mqtt_client_publish(data->client, HA_AVAILABILITY_TOPIC, PAYLOAD_AVAILABLE, sizeof(PAYLOAD_AVAILABLE), 1, 1);
        }
    }
    else{
        bool on = strncmp(msg, POWER_ON_PAYLOAD, MIN(data->data_len, sizeof(POWER_ON_PAYLOAD))) == 0;
        if(strncmp(topic, POWER_SET_TOPIC, MIN(data->topic_len, sizeof(POWER_SET_TOPIC))) == 0){
            setPower(on, false, maxWait);
        }
        else{
            setPower(getPower(maxWait).power, on, maxWait);
        }
    }
}

static void connected(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    esp_mqtt_client_handle_t client = ((esp_mqtt_event_handle_t)event_data)->client;
    //Subscribe to topics
    esp_mqtt_client_subscribe_single(client, POWER_SET_TOPIC, 0);
    esp_mqtt_client_subscribe_single(client, AUTO_SET_TOPIC, 0);

    esp_mqtt_client_subscribe_single(client, HA_AVAILABILITY_TOPIC, 0);


    //Send data about device to Home Assistant
    esp_mqtt_client_publish(client, HA_DISCOVERY_POWER_TOPIC, HA_DISCOVERY_POWER_PAYLOAD, 0, 1, 1);
    esp_mqtt_client_publish(client, HA_DISCOVERY_AUTO_TOPIC, HA_DISCOVERY_AUTO_PAYLOAD, 0, 1, 1);
    esp_mqtt_client_publish(client, HA_DISCOVERY_MOVE_TOPIC, HA_DISCOVERY_MOVE_PAYLOAD, 0, 1, 1);
    esp_mqtt_client_publish(client, HA_DISCOVERY_TEMP_TOPIC, HA_DISCOVERY_TEMP_PAYLOAD, 0, 1, 1);
    esp_mqtt_client_publish(client, HA_DISCOVERY_HUMID_TOPIC, HA_DISCOVERY_HUMID_PAYLOAD, 0, 1, 1);

    esp_mqtt_client_publish(client, HA_AVAILABILITY_TOPIC, PAYLOAD_AVAILABLE, 0, 1, 1);

    power_t power = getPower(pdMS_TO_TICKS(10000));
    if(power.power > -1){
        esp_mqtt_client_publish(client, POWER_STATE_TOPIC, power.power ? "ON" : "OFF", 0, 1, 0);
        esp_mqtt_client_publish(client, AUTO_STATE_TOPIC, power.autoMode ? "ON" : "OFF", 0, 1, 0);
    }
}

static void disconnected(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    EventGroupHandle_t event = (EventGroupHandle_t)handler_args; 
    ESP_LOGI(TAG, "Disconnected from MQTT");
    xEventGroupClearBits(event, CONNECTED_BIT);
    xEventGroupSetBits(event, DISCONNECTED_BIT);
}

static void onPowerSet(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    int8_t power = *(int8_t*)event_data;
    gpio_set_level(POWER_PIN, !power); // A low signal turns on the relay
    esp_mqtt_client_publish(mqttClient, POWER_STATE_TOPIC, power ? POWER_ON_PAYLOAD : POWER_OFF_PAYLOAD, 0, 1, 0);
}
static void onAutoSet(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    int8_t autoMode = *(int8_t*)event_data;
    gpio_set_level(MODE_DIODE_PIN, autoMode);
    esp_mqtt_client_publish(mqttClient, AUTO_STATE_TOPIC, autoMode ? POWER_ON_PAYLOAD : POWER_OFF_PAYLOAD, 0, 1, 0);
}

static void setGPIO(){
    gpio_install_isr_service(ESP_INTR_FLAG_EDGE);

    //Setup button gpio pins
    gpio_config_t buttonConfig = {
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = false,
        .pull_up_en = true,
        .intr_type = GPIO_INTR_NEGEDGE,
        .pin_bit_mask = RESET_PIN_MASK | POWER_BUTTON_PIN_MASK | MODE_PIN_MASK
    };
    gpio_config(&buttonConfig);


    //Setup output gpio pins
    gpio_config_t outConfig = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = false,
        .pull_up_en = false,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = MODE_DIODE_PIN_MASK
    };
    gpio_config(&outConfig);

    gpio_config_t powConfig = {
        .mode = GPIO_MODE_OUTPUT_OD,
        .pull_down_en = false,
        .pull_up_en = false,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = POWER_PIN_MASK
    };
    gpio_set_level(POWER_PIN, 1); //High is off
    gpio_config(&powConfig);
}

static void initialize(){
    //Initialize the non-volatile storage, which for some reason is needed for the WIFI
    ESP_ERROR_CHECK(nvs_flash_init());
    //Initialize the TCP/IP stack 
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t initCFG = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&initCFG));
    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    initPowerMonitor(false, true, onPowerSet, onAutoSet);

    setGPIO();
}

void app_main(void)
{
    initialize();
    connectedEvent = xEventGroupCreateStatic(&staticConnectedBits);
    xEventGroupClearBits(connectedEvent, CONNECTED_BIT);
    xEventGroupSetBits(connectedEvent, DISCONNECTED_BIT);

    //Create task responsible for connecting and reconnecting to WIFI and MQTT
    mqttClient = createMqttClient(NULL);
    configTaskArgs_t configTaskArgs = {.connectedHandle = connectedEvent, .mqttClient = mqttClient};
    xTaskCreate(connectionTask, "connTask", CONFIG_MAIN_TASK_STACK_SIZE, &configTaskArgs, 1, NULL);

    //Task responsible for sensing the environment
    QueueHandle_t sensorQ = xQueueCreateStatic(5, sizeof(sensorReading_t), staticSensorQueueBuffer, &staticSensorQueue);
    initSensors(sensorQ);
    xTaskCreate(sensorsTask, "sensTask", CONFIG_MAIN_TASK_STACK_SIZE, sensorQ, 1, NULL);

    //Set up callbacks for mqtt
    esp_mqtt_client_register_event(mqttClient, MQTT_EVENT_DATA, received, mqttClient);
    esp_mqtt_client_register_event(mqttClient, MQTT_EVENT_CONNECTED, connected, mqttClient);
    esp_mqtt_client_register_event(mqttClient, MQTT_EVENT_DISCONNECTED, disconnected, connectedEvent);
    
    gpio_isr_handler_add(POWER_BUTTON_PIN, powerISR, NULL);
    gpio_isr_handler_add(MODE_PIN, modeISR, NULL);

    int temperature = 0;
    bool move = 0;
    while(1){
        sensorReading_t reading;
        xQueueReceive(sensorQ, &reading, portMAX_DELAY);
        if(reading.dht){
            temperature = reading.dhtReading.temperature;
            ESP_LOGI(TAG, "Received dht reading, Temp: %d, Humid: %d", reading.dhtReading.temperature, reading.dhtReading.humidity);
        }else{
            move = reading.movementReading.move;
            ESP_LOGI(TAG, "Received move reading, Move: %d", move);
        }

        power_t curr = getPower(pdMS_TO_TICKS(3000));
        if(curr.autoMode){
            if(move && temperature >= TEMP_THRESHOLD){
                if(!curr.power)
                    switchPower(pdMS_TO_TICKS(3000));
            }
            else{
                if(curr.power)
                    switchPower(pdMS_TO_TICKS(3000));
            }
        }
        

        //Wait until connected
        EventBits_t bits = xEventGroupWaitBits(connectedEvent, CONNECTED_BIT, false, true, 0);
        if(bits & CONNECTED_BIT){
            if(reading.dht){
                char payload[4];
                int len = intToAscii(reading.dhtReading.temperature, payload, 4);
                esp_mqtt_client_publish(mqttClient, TEMP_STATE_TOPIC, payload, len, 1, 0);

                len = intToAscii(reading.dhtReading.humidity, payload, 4);
                esp_mqtt_client_publish(mqttClient, HUMID_STATE_TOPIC, payload, len, 1, 0);
            }
            else{
                esp_mqtt_client_publish(mqttClient, MOVE_STATE_TOPIC, reading.movementReading.move ? "ON" : "OFF", 0, 1, 0);
            }
        }
    }
}
