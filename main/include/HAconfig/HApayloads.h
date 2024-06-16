#ifndef HAPAYLOADS_H
#define HAPAYLOADS_H
#include "HAconfig.h"
#include "HAtopics.h"

#define HA_DEVICE "{\"identifiers\": [\"" HAid "\"], \"name\": \"Fan\", \"model\": \"ESP32\"}"
#define PAYLOAD_AVAILABLE "online"
#define PAYLOAD_NOT_AVAILABLE "offline"

#define AVAILABILITY "[{\"topic\": \"" HA_AVAILABILITY_TOPIC "\"}]"

#define HA_DISCOVERY_POWER_PAYLOAD "{"                                       \
                            "\"unique_id\": \"" HAid "power\", "             \
                            "\"name\": \"Power\", "                          \
                            "\"state_topic\": \"" POWER_STATE_TOPIC "\", "   \
                            "\"command_topic\": \"" POWER_SET_TOPIC "\", "   \
                            "\"availability\": " AVAILABILITY ", "           \
                            "\"device\": " HA_DEVICE                         \
                            "}"

#define HA_DISCOVERY_AUTO_PAYLOAD "{"                                       \
                            "\"name\": \"Auto mode\","                      \
                            "\"state_topic\": \"" AUTO_STATE_TOPIC "\" ,"   \
                            "\"command_topic\": \"" AUTO_SET_TOPIC "\" ,"   \
                            "\"availability\": " AVAILABILITY ","           \
                            "\"unique_id\": \"" HAid "auto\","              \
                            "\"device\": " HA_DEVICE                        \
                            "}"

#define HA_DISCOVERY_MOVE_PAYLOAD "{"                                       \
                            "\"name\": \"Presence\","                       \
                            "\"device_class\": \"presence\", "              \
                            "\"state_topic\": \"" MOVE_STATE_TOPIC "\" ,"   \
                            "\"availability\": " AVAILABILITY ","           \
                            "\"unique_id\": \"" HAid "move\","              \
                            "\"device\": " HA_DEVICE                        \
                            "}"

#define HA_DISCOVERY_TEMP_PAYLOAD "{"                                       \
                            "\"name\": \"Temperature\","                    \
                            "\"device_class\": \"temperature\", "           \
                            "\"state_topic\": \"" TEMP_STATE_TOPIC "\" ,"   \
                            "\"availability\": " AVAILABILITY ","           \
                            "\"unique_id\": \"" HAid "temp\","              \
                            "\"unit_of_measurement\": \"°C\","              \
                            "\"device\": " HA_DEVICE                        \
                            "}"

#define HA_DISCOVERY_HUMID_PAYLOAD "{"                                      \
                            "\"name\": \"Humidity\","                       \
                            "\"device_class\": \"humidity\", "              \
                            "\"state_topic\": \"" HUMID_STATE_TOPIC "\" ,"  \
                            "\"availability\": " AVAILABILITY ","           \
                            "\"unique_id\": \"" HAid "humid\","             \
                            "\"unit_of_measurement\": \"%\","              \
                            "\"device\": " HA_DEVICE                        \
                            "}"

#define POWER_ON_PAYLOAD "ON"
#define POWER_OFF_PAYLOAD "OFF"

#endif