#ifndef SENSORS_H
#define SENSORS_H
#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(SENSOR_EVENT);

void initSensors(void);
void sensorsTask(void* args);


#endif