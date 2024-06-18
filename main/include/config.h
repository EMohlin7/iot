#ifndef CONFIG_H
#define CONFIG_H

#define SSID_MAX_LEN 32
#define WIFI_PASS_MAX_LEN 63
#define URL_MAX_LEN 64

#define AP_IP "192.168.4.1"

#define TEMP_THRESHOLD 24

#define RESET_PIN 32
#define RESET_PIN_MASK (1ull << RESET_PIN)

#define MODE_PIN 26
#define MODE_PIN_MASK (1ull << MODE_PIN)

#define POWER_BUTTON_PIN 12
#define POWER_BUTTON_PIN_MASK (1ull << POWER_BUTTON_PIN)

#define POWER_PIN 33 
#define POWER_PIN_MASK (1ull << POWER_PIN)

#define MOVE_PIN 27
#define MOVE_PIN_MASK (1ull << MOVE_PIN)

#define DHT_PIN 25
#define DHT_PIN_MASK (1ull << DHT_PIN)
#define DHT_READ_WAIT_MS 10000
#define TEMP_OFFSET -2

#define MODE_DIODE_PIN 14
#define MODE_DIODE_PIN_MASK (1ull << MODE_DIODE_PIN)

#define CONFIG_DIODE_PIN 13
#define CONFIG_DIODE_PIN_MASK (1ull << CONFIG_DIODE_PIN)

#define NVS_CONFIG_NS "config"
#define NVS_SSID_KEY "ssid"
#define NVS_PASS_KEY "pass"
#define NVS_BROKER_ADRS_KEY "broker"
#define NVS_BROKER_PORT_KEY "brokerPort" 

#define DEBOUNCE_TIME_MS 300

#endif 