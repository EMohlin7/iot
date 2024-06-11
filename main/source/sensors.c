#include "sensors.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "config.h"
#include "driver/gpio.h"
#include "dht11.h"

#define TAG "Sensor"
#define MOVE_BIT 1
#define NO_MOVE_BIT 2
ESP_EVENT_DEFINE_BASE(SENSOR_EVENT);
static StaticTimer_t moveInitTimer;
static StaticEventGroup_t moveEventBuffer;
static EventGroupHandle_t moveEvent;

static void movementISR(void* args){
    EventGroupHandle_t moveEvent = (EventGroupHandle_t)args;
    bool high = gpio_get_level(MOVE_PIN);
    xEventGroupSetBitsFromISR(moveEvent, (MOVE_BIT*high) | (NO_MOVE_BIT * !high), NULL);
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
    moveEvent = xEventGroupCreateStatic(&moveEventBuffer);
}

void sensorsTask(void* args){
    QueueHandle_t sensorQ = (QueueHandle_t)args;
    EventBits_t bits = 0;
    while(true){
        sensorReading_t reading;
        if((bits = xEventGroupWaitBits(moveEvent, MOVE_BIT | NO_MOVE_BIT, true, false, 0))){
            reading.movementReading.move = bits & MOVE_BIT;
            reading.dht = false;
            
            xQueueSendToBack(sensorQ, &reading, pdMS_TO_TICKS(2000));
        }
    
        //Read humidifier
        struct dht11_reading dhtReading = DHT11_read();
        if(dhtReading.status == DHT11_OK){
            ESP_LOGI(TAG, "Temp: %d", dhtReading.temperature);
            ESP_LOGI(TAG, "Humid: %d",dhtReading.humidity);

            reading.dht = true;
            reading.dhtReading.humidity = dhtReading.humidity;
            reading.dhtReading.temperature = dhtReading.temperature + TEMP_OFFSET;
            xQueueSendToBack(sensorQ, &reading, pdMS_TO_TICKS(2000));
        }
        else{
            ESP_LOGI(TAG, "DHT11 reading failed");
        }
        vTaskDelay(pdMS_TO_TICKS(DHT_READ_WAIT_MS));
    }
}
