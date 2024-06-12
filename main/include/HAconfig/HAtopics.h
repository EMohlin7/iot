#ifndef HATOPICS_H
#define HATOPICS_H
#include "HAconfig.h"

#define HA_AVAILABILITY_TOPIC HAid "/availability"

#define HA_DISCOVERY_POWER_TOPIC "homeassistant/switch/" HAid "power/config"
#define POWER_STATE_TOPIC HAid "/power/state"
#define POWER_SET_TOPIC HAid "/power/set"

#define HA_DISCOVERY_AUTO_TOPIC "homeassistant/switch/" HAid "auto/config"
#define AUTO_STATE_TOPIC HAid "/auto/state"
#define AUTO_SET_TOPIC HAid "/auto/set"

#define HA_DISCOVERY_TEMP_TOPIC "homeassistant/sensor/" HAid "_temp/config"
#define TEMP_STATE_TOPIC HAid "/temp/state"

#define HA_DISCOVERY_MOVE_TOPIC "homeassistant/binary_sensor/" HAid "movement/config"
#define MOVE_STATE_TOPIC HAid "/movement/state"

#define HA_DISCOVERY_HUMID_TOPIC "homeassistant/sensor/" HAid "humid/config"
#define HUMID_STATE_TOPIC HAid "/humid/state"

#endif