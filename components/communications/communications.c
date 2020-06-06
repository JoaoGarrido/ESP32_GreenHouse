#include "communications.h"

/**Global variables**/
extern sensor_data_t sensor_data;
extern control_data_t control_data;
extern SemaphoreHandle_t publish_sensor_signal;
extern SemaphoreHandle_t publish_control_signal;
/**Private variables**/
//Wifi
static EventGroupHandle_t wifi_event_group;
static const int CONNECTED_BIT = BIT0; //Event group
//Mqtt
static esp_mqtt_client_handle_t client_g;
static const char* BROKER_URL = "mqtt://test.mosquitto.org";
static const int BROKER_PORT = 1883;
//Publish
static const char* temp_current_topic = "/GreenHouse/Temperature/current";
static const char* humid_current_topic = "/GreenHouse/Humidity/current";
static const char* lumi_current_topic = "/GreenHouse/Luminosity/current";
static const char* temp_max_get_topic = "/GreenHouse/Temperature_MaxLimit/get";
static const char* temp_min_get_topic = "/GreenHouse/Temperature_MinLimit/get";
static const char* window_get_topic = "/GreenHouse/Window/get";
static const char* mode_get_topic = "/GreenHouse/Mode/get";
//Subscribe
static const char* temp_max_set_topic = "/GreenHouse/Temperature_MaxLimit/set";
static const char* temp_min_set_topic = "/GreenHouse/Temperature_MinLimit/set";
static const char* window_set_topic = "/GreenHouse/Window/set";
static const char* mode_set_topic = "/GreenHouse/Mode/set";


/**Static functions**/
static void initialize_nvs();
static esp_err_t mqtt_event_handler_callback(esp_mqtt_event_handle_t event);
static void mqtt_subscribed_event_handler(esp_mqtt_event_handle_t event);

static void mqtt_subscribed_event_handler(esp_mqtt_event_handle_t event){
    if(strncmp(temp_min_set_topic, event->topic, event->topic_len) == 0){
        float rec_data = atof(event->data);
        if(rec_data > 0.0 && rec_data < 100.0){
            control_data.temperature_min = rec_data;
        }
        else{
            ESP_LOGI(mqtt_tag, "%s topic data is invalid", event->topic);
        }
    }
    else if(strncmp(temp_max_set_topic, event->topic, event->topic_len) == 0){
        float rec_data = atof(event->data);
        if(rec_data > 0.0 && rec_data < 100.0){
            control_data.temperature_max = rec_data;
        }
        else{
            ESP_LOGI(mqtt_tag, "%s topic data is invalid", event->topic);
        }
    }
    else if(strncmp(mode_set_topic, event->topic, event->topic_len) == 0){
        int rec_data = atoi(event->data);
        if(rec_data >= 0 && rec_data < 2){
            control_data.mode = rec_data;
        }
        else{
            ESP_LOGI(mqtt_tag, "%s topic data is invalid", event->topic);
        }            
    }
    else if(strncmp(window_set_topic, event->topic, event->topic_len) == 0){
        int rec_data = atoi(event->data);
        if(rec_data >= 0 && rec_data < 2 && control_data.mode == Mode_Manual){
            control_data.window_action = rec_data;
        }
        else{
            ESP_LOGI(mqtt_tag, "%s topic data is invalid", event->topic);
        }
    }
}

static esp_err_t mqtt_event_handler_callback(esp_mqtt_event_handle_t event){
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(mqtt_tag, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, temp_max_set_topic, 1);
            ESP_LOGI(mqtt_tag, "Sent topic %s subscribe successful, msg_id=%d", temp_max_set_topic, msg_id);
            msg_id = esp_mqtt_client_subscribe(client, temp_min_set_topic, 1);
            ESP_LOGI(mqtt_tag, "Sent topic %s subscribe successful, msg_id=%d", temp_min_set_topic, msg_id);
            msg_id = esp_mqtt_client_subscribe(client, window_set_topic, 1);
            ESP_LOGI(mqtt_tag, "Sent topic %s subscribe successful, msg_id=%d", window_set_topic, msg_id);
            msg_id = esp_mqtt_client_subscribe(client, mode_set_topic, 1);
            ESP_LOGI(mqtt_tag, "Sent topic %s subscribe successful, msg_id=%d", mode_set_topic, msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(mqtt_tag, "MQTT_EVENT_DISCONNECTED");
            break;  
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(mqtt_tag, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(mqtt_tag, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(mqtt_tag, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(mqtt_tag, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);

            //Handle subscribe topics
            /* Maybe a semaphore around control_data is necessary because of the GUI on the display*/
            mqtt_subscribed_event_handler(event);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(mqtt_tag, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_ESP_TLS) {
                ESP_LOGI(mqtt_tag, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
                ESP_LOGI(mqtt_tag, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                ESP_LOGI(mqtt_tag, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
            } else {
                ESP_LOGW(mqtt_tag, "Unknown error type: 0x%x", event->error_handle->error_type);
            }
            break;
        default:
            ESP_LOGI(mqtt_tag, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void initialize_nvs(){
    // Initialize Non volatile storage -> needs to be initialized because of the wifi
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
}

esp_err_t wifi_event_handler(void *ctx, system_event_t *event){
    //ESP_LOGI(wifi_tag,"Task running: %s", "wifi_event_handler");
    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            /* This is a workaround as ESP32 WiFi libs don't currently
            auto-reassociate. */
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

void initialize_wifi_sta_mode(){
    initialize_nvs();
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(wifi_event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.wifi_task_core_id = WIFI_COMMUNICATIONS_CORE;
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    //ESP_LOGI(wifi_tag,"Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

void initialize_mqtt_app(){
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = BROKER_URL,
        .port = BROKER_PORT,
        .task_prio = 4,
    };
    ESP_LOGI(memory_tag, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    client_g = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client_g, ESP_EVENT_ANY_ID, mqtt_event_handler, client_g);
    //xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY); //Waits for the wifi to connect to run the mqtt server
    esp_mqtt_client_start(client_g);
}


void mqtt_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    ESP_LOGD(mqtt_tag, "Event dispatched from event loop base=%s, event_id=%d", event_base, event_id);
    mqtt_event_handler_callback(event_data);
}

void publish_sensor_data_handler(void *args){
    char buff[21] = "";
    for(;;){
        xSemaphoreTake(publish_sensor_signal, portMAX_DELAY);
        ESP_LOGI(mqtt_tag,"Task running: %s", "publish_dht_handler");
        sprintf(buff, "%f", sensor_data.temperature);
        esp_mqtt_client_publish(client_g, temp_current_topic, buff, 0, 0, 0);
        sprintf(buff, "%f", sensor_data.humidity);
        esp_mqtt_client_publish(client_g, humid_current_topic, buff, 0, 0, 0);
        sprintf(buff, "%u", sensor_data.luminosity);
        esp_mqtt_client_publish(client_g, lumi_current_topic, buff, 0, 0, 0);
        sprintf(buff, "%d", sensor_data.window_state);
        esp_mqtt_client_publish(client_g, window_get_topic, buff, 0, 0, 0);
    }
}

void publish_control_data_handler(void *args){
    char buff[21] = "";
    for(;;){
        xSemaphoreTake(publish_control_signal, portMAX_DELAY);
        ESP_LOGI(mqtt_tag,"Task running: %s", "publish_control_data_handler");
        sprintf(buff, "%d", control_data.mode);
        esp_mqtt_client_publish(client_g, mode_get_topic, buff, 0, 0, 0);
        sprintf(buff, "%f", control_data.temperature_min);
        esp_mqtt_client_publish(client_g, temp_min_get_topic, buff, 0, 0, 0);
        sprintf(buff, "%f", control_data.temperature_max);
        esp_mqtt_client_publish(client_g, temp_max_get_topic, buff, 0, 0, 0);
    }
}