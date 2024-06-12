#ifndef POWER_H
#define POWER_H
#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(POWER_EVENT);

typedef enum{
    SET_POWER_EVENT,
    SET_AUTO_EVENT
} powerEvent_t;

typedef struct {
    int8_t autoMode;
    int8_t power;
} power_t;

power_t initPowerMonitor(bool power, bool autoMode, esp_event_handler_t onPowerHandler, esp_event_handler_t onAutoHandler);

power_t getPower(TickType_t maxWait);
void setPower(bool power, bool autoMode, TickType_t maxWait);
power_t switchPower(TickType_t maxWait);
power_t switchAutoMode(TickType_t maxWait);

#endif