#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "nvs_flash.h" 
#include "esp_wifi.h"

#include "wifiController.h"
#include "mqttHandler.h"
#include "configServer.h"
#include "sensors.h"

#define TAG "Main"

#define setupNvsGet(type, nvs, key, dest, size) nvs_get_##type(nvs, key, dest, size)
#define NVSreadConfig(nvs, key, dest, sizePtr) _Generic((dest), \
    char*: setupNvsGet(str, nvs, key, dest, sizePtr),           \
    unsigned char*: setupNvsGet(str, nvs, key, dest, sizePtr),  \
    uint16_t*: setupNvsGet(u16, nvs, key, dest, sizePtr),       \
)

typedef struct 
{
    SemaphoreHandle_t resetSem;
    bool connected;
} reset_t;


static void resetISR (void* resetStruct){
    reset_t* reset = (reset_t*)resetStruct;
    if(reset->connected)
        xSemaphoreGiveFromISR(reset->resetSem, NULL);
}


//Load already saved configuration data from NVS
static bool loadConfig(nvs_handle_t nvs, configData_t* configData){
    size_t size = sizeof(configData->ssid);
    ESP_LOGI(TAG, "Loading config");
    esp_err_t err = nvs_get_str(nvs, NVS_SSID_KEY, (char*)configData->ssid, &size);
    if(err)
        goto loadFail;

    size = sizeof(configData->password);
    err = nvs_get_str(nvs, NVS_PASS_KEY, (char*)configData->password, &size);
    if(err)
        goto loadFail;

    size = sizeof(configData->mqttURL);
    err = nvs_get_str(nvs, NVS_BROKER_ADRS_KEY, (char*)configData->mqttURL, &size);
    if(err)
        goto loadFail;

    err = nvs_get_u16(nvs, NVS_BROKER_PORT_KEY, &configData->mqttPort);
    if(err)
        goto loadFail;

    ESP_LOGI(TAG, "Loaded config");
    return true;

loadFail:
    ESP_LOGI(TAG, "Failed to load config");
    return false;
}

static bool saveConfig(nvs_handle_t nvs, configData_t* configData){
    ESP_LOGI(TAG, "Saving config");
    esp_err_t err = nvs_set_str(nvs, NVS_SSID_KEY, (char*)configData->ssid);
    if(err)
        goto saveFail;

    err = nvs_set_str(nvs, NVS_PASS_KEY, (char*)configData->password);
    if(err)
        goto saveFail;

    err = nvs_set_str(nvs, NVS_BROKER_ADRS_KEY, (char*)configData->mqttURL);
    if(err)
        goto saveFail;

    err = nvs_set_u16(nvs, NVS_BROKER_PORT_KEY, configData->mqttPort);
    if(err)
        goto saveFail;

    ESP_LOGI(TAG, "Save config");
    return true;
saveFail:
    ESP_LOGI(TAG, "Failed to save config");
    return false;
}

static void configTask(void* args){
    esp_mqtt_client_handle_t mqttClient = (esp_mqtt_client_handle_t)args;
    reset_t reset = {
        .resetSem = xSemaphoreCreateBinary(),
        .connected = false
    };

    //Semaphore is created in an empty state
    SemaphoreHandle_t configFinished = xSemaphoreCreateBinary();
    configData_t configData;

    gpio_isr_handler_add(RESET_PIN, resetISR, &reset);

    nvs_handle_t nvs;
    nvs_open(NVS_CONFIG_NS, NVS_READWRITE, &nvs);

    if(loadConfig(nvs, &configData))
        goto configReceived;

    while(true){
        while(!reset.connected){
            startWifiAP();
            startConfigServer(configFinished, &configData);

            ESP_LOGI(TAG, "Waiting for config");
            xSemaphoreTake(configFinished, portMAX_DELAY);
            ESP_LOGI(TAG, "Config is finished");

            stopWifi();
configReceived:
            reset.connected = startWifiSTA(configData.ssid, configData.password) && 
                startMqtt(mqttClient, configData.mqttURL, configData.mqttPort);

            if(!reset.connected)
                stopWifi();
        }

        saveConfig(nvs, &configData);
        xSemaphoreTake(reset.resetSem, portMAX_DELAY);
        reset.connected = false;
        stopMqtt(mqttClient);
        stopWifi();
    }
}


void app_main(void)
{
    //Initialize the non-volatile storage, which for some reason is needed for the WIFI
    ESP_ERROR_CHECK(nvs_flash_init());
    //Initialize the TCP/IP stack 
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t initCFG = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&initCFG));
    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    //Setup reset button gpio pin
    gpio_config_t ioConfig = {
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = false,
        .pull_up_en = true,
        .intr_type = GPIO_INTR_NEGEDGE,
        .pin_bit_mask = RESET_PIN_MASK
    };

    gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
    initSensors();
    gpio_config(&ioConfig);

    esp_mqtt_client_handle_t mqttClient = createMqttClient(NULL);
    xTaskCreate(configTask, "confTask", CONFIG_MAIN_TASK_STACK_SIZE, mqttClient, 1, NULL);


    uint8_t level = 0;
    gpio_reset_pin(GPIO_NUM_5);
    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
    while(1){
        gpio_set_level(GPIO_NUM_5, level);
        level = !level;
        vTaskDelay(100);
    }
}
