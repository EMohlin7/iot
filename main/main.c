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

#define TAG "Main"

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


    esp_mqtt_client_handle_t mqttClient = createMqttClient(NULL);
    bool connected = false;
    while(!connected){
        startWifiAP();

        //Semaphore is created in an empty state
        SemaphoreHandle_t configFinished = xSemaphoreCreateBinary();
        configData_t configData;

        startConfigServer(configFinished, &configData);
        ESP_LOGI(TAG, "Waiting for config");
        xSemaphoreTake(configFinished, portMAX_DELAY);
        ESP_LOGI(TAG, "Config is finished");

        stopWifi();
        connected = startWifiSTA(configData.ssid, configData.password) && 
            startMqtt(mqttClient, configData.mqttURL, configData.mqttPort);

        if(!connected)
            stopWifi();
    }


    uint8_t level = 0;
    gpio_reset_pin(GPIO_NUM_5);
    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
    while(1){
        ESP_LOGI("Test", "Blink! %d", gpio_set_level(GPIO_NUM_5, level));
        level = !level;
        vTaskDelay(100);
    }
}
