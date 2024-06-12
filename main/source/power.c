#include "power.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

ESP_EVENT_DEFINE_BASE(POWER_EVENT);

typedef struct {
    SemaphoreHandle_t mutex;
    power_t power;
} monitor_t;

static StaticSemaphore_t mutexBuffer;
static monitor_t monitor;


power_t initPowerMonitor(bool power, bool autoMode, esp_event_handler_t onPowerHandler, esp_event_handler_t onAutoHandler){
    monitor.mutex = xSemaphoreCreateMutexStatic(&mutexBuffer);
    monitor.power.power = power;
    monitor.power.autoMode = autoMode;
    ESP_ERROR_CHECK(esp_event_handler_register(POWER_EVENT, SET_POWER_EVENT, onPowerHandler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(POWER_EVENT, SET_AUTO_EVENT, onAutoHandler, NULL));

    return monitor.power;
}

power_t getPower(TickType_t maxWait){
    power_t power;
    if(xSemaphoreTake(monitor.mutex, maxWait)){
        power = monitor.power;
        xSemaphoreGive(monitor.mutex);
        return power;
    }

    power.autoMode = -1;
    power.power = -1;
    return power;
}
void setPower(bool power, bool autoMode, TickType_t maxWait){
    if(xSemaphoreTake(monitor.mutex, maxWait)){
        monitor.power.power = power;
        monitor.power.autoMode = autoMode;
        xSemaphoreGive(monitor.mutex);
        esp_event_post(POWER_EVENT, SET_POWER_EVENT, &power, sizeof(bool), maxWait);
        esp_event_post(POWER_EVENT, SET_AUTO_EVENT, &autoMode, sizeof(bool), maxWait);
    }
}
power_t switchPower(TickType_t maxWait){
    power_t power;
    if(xSemaphoreTake(monitor.mutex, maxWait)){
        monitor.power.power = !monitor.power.power;
        power.power = monitor.power.power;
        power.autoMode = monitor.power.autoMode;
        xSemaphoreGive(monitor.mutex);
        esp_event_post(POWER_EVENT, SET_POWER_EVENT, &power.power, sizeof(bool), maxWait);
        return power;
    }
    power.autoMode = -1;
    power.power = -1;
    return power;
}
power_t switchAutoMode(TickType_t maxWait){
    power_t power;
    if(xSemaphoreTake(monitor.mutex, maxWait)){
        monitor.power.autoMode = !monitor.power.autoMode;
        power.power = monitor.power.power;
        power.autoMode = monitor.power.autoMode;
        xSemaphoreGive(monitor.mutex);
        esp_event_post(POWER_EVENT, SET_AUTO_EVENT, &power.autoMode, sizeof(bool), maxWait);
        return power;
    }
    power.autoMode = -1;
    power.power = -1;
    return power;
}