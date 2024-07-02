#ifndef UTILS_H
#define UTILS_H
#include "config.h"

#define DEBOUNCE {static TickType_t lastTick = 0;               \
    TickType_t tick = xTaskGetTickCountFromISR();               \
    if(tick < lastTick) /*Check for overflow*/                  \
        lastTick = 0;                                           \
    if(lastTick + pdMS_TO_TICKS(DEBOUNCE_TIME_MS) > tick)       \
        return;                                                 \
    lastTick = tick;                                            \
}

#define MIN(a,b) (((a)<(b))?(a):(b))


int intToAscii(int num, char* a, int len);

#endif