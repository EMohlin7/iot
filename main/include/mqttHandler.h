#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H
#include "mqtt_client.h"

/// @brief Connect client to broker
/// @param mqttClient Client to connect
/// @param brokerUri URI of the broker the client will conenct to
/// @param brokerPort Port of the broker the client will connect to
bool startMqtt(esp_mqtt_client_handle_t mqttClient, const char* brokerUri, uint16_t brokerPort);

void stopMqtt(esp_mqtt_client_handle_t mqttClient);

/// @brief Create a client used to communicate with the MQTT broker 
/// @param clientId Name of the client. If NULL will get a default name
/// @return Client handle
esp_mqtt_client_handle_t createMqttClient(const char* clientId);


#endif