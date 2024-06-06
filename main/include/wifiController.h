#ifndef WIFICONTROLLER_H
#define WIFICONTROLLER_H
#include "stdbool.h"

bool startWifiSTA(const unsigned char* ssid, const unsigned char* pass);

void startWifiAP(void);

void stopWifi(void);

#endif