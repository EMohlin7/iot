#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <inttypes.h>
#include "driver/gpio.h"
#include "nvs_flash.h" 
#include "esp_wifi.h"

#include "wifiController.h"
#include "mqttHandler.h"
#include "configServer.h"

#define TAG "Main"
#define RESET_PIN 32
#define RESET_PIN_MASK (1ull << RESET_PIN)

typedef struct 
{
    SemaphoreHandle_t resetSem;
    bool connected;
} reset_t;


void resetISR (void* resetStruct){
    reset_t* reset = (reset_t*)resetStruct;
    if(reset->connected)
        xSemaphoreGiveFromISR(reset->resetSem, NULL);
}


void config(void* args){
    reset_t reset = {
        .resetSem = xSemaphoreCreateBinary(),
        .connected = false
    };

    gpio_isr_handler_add(RESET_PIN, resetISR, &reset);

    esp_mqtt_client_handle_t mqttClient = createMqttClient(NULL);

    while(true){
        while(!reset.connected){
            startWifiAP();

            //Semaphore is created in an empty state
            SemaphoreHandle_t configFinished = xSemaphoreCreateBinary();
            configData_t configData;

            startConfigServer(configFinished, &configData);
            ESP_LOGI(TAG, "Waiting for config");
            xSemaphoreTake(configFinished, portMAX_DELAY);
            ESP_LOGI(TAG, "Config is finished");

            stopWifi();
            reset.connected = startWifiSTA(configData.ssid, configData.password) && 
                startMqtt(mqttClient, configData.mqttURL, configData.mqttPort);

            if(!reset.connected)
                stopWifi();
        }

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

    gpio_config_t ioConfig = {
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = false,
        .pull_up_en = true,
        .intr_type = GPIO_INTR_NEGEDGE,
        .pin_bit_mask = RESET_PIN_MASK
    };

    gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
    gpio_config(&ioConfig);

    xTaskCreate(config, "confTask", CONFIG_MAIN_TASK_STACK_SIZE, NULL, 1, NULL);


    uint8_t level = 0;
    gpio_reset_pin(GPIO_NUM_5);
    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
    while(1){
        gpio_set_level(GPIO_NUM_5, level);
        level = !level;
        vTaskDelay(100);
    }
}
