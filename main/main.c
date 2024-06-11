#include <stdio.h>
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

#define TAG "Main"
#define SENS_QUEUE_LEN 5


static StaticEventGroup_t staticConnectedBits;
static StaticQueue_t staticSensorQueue;
static uint8_t staticSensorQueueBuffer[SENS_QUEUE_LEN*sizeof(sensorReading_t)];

typedef struct {
    bool autoMode;
    bool power;
} mode_t;

int numPress = 0;

//TODO: Use monitor on mode settings
static void powerISR(void* args){
    DEBOUNCE
    mode_t* mode = (mode_t*)args;
    mode->autoMode = false;
    mode->power = !mode->power;
    ++numPress;
}

static void modeISR(void* args){
    DEBOUNCE
    mode_t* mode = (mode_t*)args;
    mode->autoMode = !mode->autoMode;
}

static void received(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    ESP_LOGI(TAG, "Received msg from MQTT");
}

static void connected(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)handler_args;
    //TODO: Subscribe to topics
    //TODO: Send data about device to Home Assistant
}

static void disconnected(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    EventGroupHandle_t event = (EventGroupHandle_t)handler_args; 
    ESP_LOGI(TAG, "Disconnected from MQTT");
    xEventGroupClearBits(event, CONNECTED_BIT);
    xEventGroupSetBits(event, DISCONNECTED_BIT);
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
        .pin_bit_mask = DIOD_PIN_MASK
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

    initSensors();

    setGPIO();
}

void app_main(void)
{
    initialize();

    EventGroupHandle_t connectedEvent = xEventGroupCreateStatic(&staticConnectedBits);
    xEventGroupClearBits(connectedEvent, CONNECTED_BIT);
    xEventGroupSetBits(connectedEvent, DISCONNECTED_BIT);

    //Create task responsible for connecting and reconnecting to WIFI and MQTT
    esp_mqtt_client_handle_t mqttClient = createMqttClient(NULL);
    configTaskArgs_t configTaskArgs = {.connectedHandle = connectedEvent, .mqttClient = mqttClient};
    xTaskCreate(connectionTask, "connTask", CONFIG_MAIN_TASK_STACK_SIZE, &configTaskArgs, 1, NULL);

    //Task responsible for sensing the environment
    QueueHandle_t sensorQ = xQueueCreateStatic(5, sizeof(sensorReading_t), staticSensorQueueBuffer, &staticSensorQueue);
    xTaskCreate(sensorsTask, "sensTask", CONFIG_MAIN_TASK_STACK_SIZE, sensorQ, 1, NULL);

    //Set up callbacks for mqtt
    esp_mqtt_client_register_event(mqttClient, MQTT_EVENT_DATA, received, mqttClient);
    esp_mqtt_client_register_event(mqttClient, MQTT_EVENT_CONNECTED, connected, connectedEvent);
    esp_mqtt_client_register_event(mqttClient, MQTT_EVENT_DISCONNECTED, disconnected, connectedEvent);

    mode_t mode = {
        .autoMode = true,
        .power = false 
    };
    gpio_isr_handler_add(POWER_BUTTON_PIN, powerISR, &mode);
    gpio_isr_handler_add(MODE_PIN, modeISR, &mode);

    bool movement = false;

    while(1){
        //TODO: Receive sensor data and process it
        sensorReading_t reading;
        xQueueReceive(sensorQ, &reading, portMAX_DELAY);
        if(reading.dht){
            ESP_LOGI(TAG, "Received dht reading, Temp: %d, Humid: %d", reading.dhtReading.temperature, reading.dhtReading.humidity);
        }else{
            movement = reading.movementReading.move;
            ESP_LOGI(TAG, "Received move reading, Move: %d", movement);
        }

        //Wait until connected
        EventBits_t bits = xEventGroupWaitBits(connectedEvent, CONNECTED_BIT, false, true, 0);
        if(bits & CONNECTED_BIT){
            //TODO: Send data to Home Assistant

        }



        gpio_set_level(DIOD_PIN, mode.power);
        gpio_set_level(POWER_PIN, !mode.power); // A low signal turns on the relay
        ESP_LOGI(TAG, "Number of presses: %d", numPress);
    }
}
