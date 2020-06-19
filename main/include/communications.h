#ifndef COMMUNICATIONS
#define COMMUNICATIONS

/*System libraries*/
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "mqtt_client.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
/*Project libraries*/
#include "greenhouse_system.h"

/*Definitions*/
#define WIFI_SSID ""
#define WIFI_PASS ""

/**Functions**/
//Wifi
void initialize_wifi_sta_mode();
esp_err_t wifi_event_handler(void *ctx, system_event_t *event);
//MQTT
void initialize_mqtt_app();
void mqtt_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void publish_sensor_data_handler(void *args);
void publish_control_data_handler(void *args);

#endif /* COMMUNICATIONS */
