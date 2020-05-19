#ifndef COMMUNICATIONS
#define COMMUNICATIONS

//System libraries
#include "mqtt_client.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
//Project libraries
#include "logging.h"
#include "greenhouse_system.h"

#define WIFI_SSID ""
#define WIFI_PASS ""

/**Wifi**/
//Functions
void initialize_wifi_sta_mode();
esp_err_t wifi_event_handler(void *ctx, system_event_t *event);

/**MQTT**/
void initialize_mqtt_app();
void mqtt_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void publish_dht_handler(void *args);
void publish_ldr_handler(void *args);
void publish_window_state_handler(void *args);

#endif /* COMMUNICATIONS */
