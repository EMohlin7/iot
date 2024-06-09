#ifndef SENSORS_H
#define SENSORS_H
#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(SENSOR_EVENT);

typedef struct 
{
    bool dht;
    union 
    {
        struct dhtReading{
            int temperature;
            int humidity;
        } dhtReading;

        struct moveReading{
            bool move;
        } movementReading;
    };
    
} sensorReading_t;

typedef enum {
    SENSOR_EVENT_DHT,
    SENSOR_EVENT_MOVE
} sensorEventId_t;

void initSensors(void);
void sensorsTask(void* args);


#endif