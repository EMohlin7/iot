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
static StaticTimer_t moveInitTimerBuffer;

static portMUX_TYPE readLock = portMUX_INITIALIZER_UNLOCKED;

static void movementISR(void* args){
    QueueHandle_t sensorQ = (QueueHandle_t)args;
    bool high = gpio_get_level(MOVE_PIN);
    sensorReading_t reading = {
        .dht = false,
        .movementReading = {
            .move = high
        }
    };
    xQueueSendToBackFromISR(sensorQ, &reading, NULL);
}

static void onMoveInitFin(TimerHandle_t arg){
    //The pointer to the queue is stored as the timers id
    QueueHandle_t sensorQ = (QueueHandle_t)pvTimerGetTimerID(arg);
    gpio_isr_handler_add(MOVE_PIN, movementISR, sensorQ);
    bool high = gpio_get_level(MOVE_PIN);
    sensorReading_t reading = {
        .dht = false,
        .movementReading = {
            .move = high
        }
    };
    xQueueSendToBack(sensorQ, &reading, 0);
    ESP_LOGI(TAG, "Movement sensor initialized");
}

void initSensors(QueueHandle_t sensorQ){
    gpio_config_t ioConfig = {
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = false,
        .pull_up_en = false,
        .intr_type = GPIO_INTR_ANYEDGE,
        .pin_bit_mask = MOVE_PIN_MASK
    };
    gpio_config(&ioConfig);

    //Movement sensor need 1 minute to initialize
    TimerHandle_t timerHandle = xTimerCreateStatic(
        "moveInit", pdMS_TO_TICKS(1000*60), false, sensorQ, onMoveInitFin, &moveInitTimerBuffer); 

    xTimerStart(timerHandle, 0);
    DHT11_init(DHT_PIN);
}

void sensorsTask(void* args){
    QueueHandle_t sensorQ = (QueueHandle_t)args;
    while(true){
        sensorReading_t reading;
    
        //Read humidifier
        taskENTER_CRITICAL(&readLock);
        struct dht11_reading dhtReading = DHT11_read();
        taskEXIT_CRITICAL(&readLock);
        if(dhtReading.status == DHT11_OK){
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
