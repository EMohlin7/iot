#include "sensors.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "config.h"
#include "driver/gpio.h"
#include "dht11.h"

#define TAG "Sensor"

ESP_EVENT_DEFINE_BASE(SENSOR_EVENT);
static StaticTimer_t moveInitTimer;

static void movementISR(void* args){
    
}

static void onMoveInitFin(TimerHandle_t arg){
    gpio_isr_handler_add(MOVE_PIN, movementISR, NULL);
    ESP_LOGI(TAG, "Movement sensor initialized");
}

void initSensors(){
    gpio_config_t ioConfig = {
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = false,
        .pull_up_en = true,
        .intr_type = GPIO_INTR_ANYEDGE,
        .pin_bit_mask = MOVE_PIN_MASK
    };
    gpio_config(&ioConfig);

    //Movement sensor need 1 minute to initialize
    TimerHandle_t timerHandle = xTimerCreateStatic(
        "moveInit", pdMS_TO_TICKS(1000*60), false, NULL, onMoveInitFin, &moveInitTimer);

    xTimerStart(timerHandle, pdMS_TO_TICKS(1000*60));

    DHT11_init(DHT_PIN);
}

void sensorsTask(void* args){
    while(true){
        //Read humidifier
        struct dht11_reading reading = DHT11_read();
        if(reading.status == DHT11_OK){
            ESP_LOGI(TAG, "Temp: %d", reading.temperature);
            ESP_LOGI(TAG, "Humid: %d", reading.humidity);
        }
        else{
            ESP_LOGI(TAG, "DHT11 reading failed");
        }
        vTaskDelay(pdMS_TO_TICKS(DHT_READ_WAIT_MS));
    }
}
