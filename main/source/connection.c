#include "connection.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "nvs_flash.h" 
#include "wifiController.h"
#include "mqttHandler.h"
#include "config.h"
#include "utils.h"
#include "sensors.h"
#include "configServer.h"

#define TAG "Connection task"


static void clearConfigISR (void* args){
    DEBOUNCE
    EventGroupHandle_t connectedHandle = (EventGroupHandle_t)args;
    
    xEventGroupClearBitsFromISR(connectedHandle, CONNECTED_BIT);
    xEventGroupSetBitsFromISR(connectedHandle, DISCONNECTED_BIT | RESET_BIT, NULL);
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


void connectionTask(void* args){
    configTaskArgs_t* arg = (configTaskArgs_t*)args;
    esp_mqtt_client_handle_t mqttClient = arg->mqttClient;
    EventGroupHandle_t connectedHandle = arg->connectedHandle;
    bool connected = false;
    bool configCleared = true;

    //Semaphore is created in an empty state
    SemaphoreHandle_t configFinished = xSemaphoreCreateBinary();
    configData_t configData;

    gpio_isr_handler_add(RESET_PIN, clearConfigISR, connectedHandle);

    //Needed for loading and saving config from Non-volatile storage
    nvs_handle_t nvs;
    nvs_open(NVS_CONFIG_NS, NVS_READWRITE, &nvs);

    if(loadConfig(nvs, &configData))
        configCleared = false;

    while(true){
        while(!connected){
            //If config is cleared, start a server to receive a new config
            if(configCleared){
                startWifiAP();
                startConfigServer(configFinished, &configData);

                ESP_LOGI(TAG, "Waiting for config");
                xSemaphoreTake(configFinished, portMAX_DELAY);
                ESP_LOGI(TAG, "Config is finished");

                stopWifi();
            }
            
            connected = startWifiSTA(configData.ssid, configData.password) && 
            startMqtt(mqttClient, configData.mqttURL, configData.mqttPort);

            if(!connected){
                stopWifi();
                //Make sure the config gets reset if the button is pressed when previous working config no longer has connection
                EventBits_t bits = xEventGroupWaitBits(connectedHandle, RESET_BIT, false, true, 0);
                configCleared = bits & RESET_BIT;
            }
        }

        saveConfig(nvs, &configData);
        xEventGroupClearBits(connectedHandle, DISCONNECTED_BIT | RESET_BIT);
        xEventGroupSetBits(connectedHandle, CONNECTED_BIT);

        //Wait until we are disconnected or until the network config is reset by the user
        EventBits_t bits = xEventGroupWaitBits(connectedHandle, DISCONNECTED_BIT | RESET_BIT, false, false, portMAX_DELAY);
        stopMqtt(mqttClient);
        stopWifi();
        connected = false;
        if(bits & RESET_BIT){
            configCleared = true; 
        }
        else{
            configCleared = false;
        }        
    }
}