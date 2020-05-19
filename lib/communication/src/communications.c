#include "communications.h"

/** Definitions**/
//Config
static const char* BROKER_URL = "mqtt://test.mosquitto.org";
static const int BROKER_PORT = 1883;
static const int CONNECTED_BIT = BIT0; //Event group
//Topics
static const char* temp_max_topic = "Temperature_max";
static const char* temp_min_topic = "Temperature_min";
static const char* temp_topic = "Temperature";
static const char* humid_topic = "Humidity";
static const char* lumi_topic = "Luminosity";
static const char* window_state_topic = "Window_state";
static const char* window_topic = "Window";

/** Variables**/
//Static data
static EventGroupHandle_t wifi_event_group;
static esp_mqtt_client_handle_t client_g;
//Global data
extern sensor_data_t sensor_data;
extern SemaphoreHandle_t DHT_Signal;
extern SemaphoreHandle_t LDR_Signal;
extern SemaphoreHandle_t Window_state_Signal;

//Functions
//Static functions
static esp_err_t mqtt_event_handler_callback(esp_mqtt_event_handle_t event){
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(mqtt_tag, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, temp_min_topic, 1);
            ESP_LOGI(mqtt_tag, "sent subscribe successful, msg_id=%d", msg_id);
            msg_id = esp_mqtt_client_subscribe(client, temp_max_topic, 1);
            ESP_LOGI(mqtt_tag, "sent subscribe successful, msg_id=%d", msg_id);
            msg_id = esp_mqtt_client_subscribe(client, window_topic, 1);
            ESP_LOGI(mqtt_tag, "sent subscribe successful, msg_id=%d", msg_id);
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
            if (strncmp(event->data, "send binary please", event->data_len) == 0) {
                ESP_LOGI(mqtt_tag, "Sending the binary");
            }
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

//Global functions
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
    esp_mqtt_client_start(client_g);
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

void mqtt_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    ESP_LOGD(mqtt_tag, "Event dispatched from event loop base=%s, event_id=%d", event_base, event_id);
    mqtt_event_handler_callback(event_data);
}

void publish_dht_handler(void *args){
    char buff[21] = "";
    for(;;){
        xSemaphoreTake(DHT_Signal, portMAX_DELAY);
        ESP_LOGI(task_logging,"Task running: %s", "publish_dht_handler");
        sprintf(buff, "%f", sensor_data.temperature);
        esp_mqtt_client_publish(client_g, temp_topic, buff, 0, 0, 0);
        sprintf(buff, "%f", sensor_data.humidity);
        esp_mqtt_client_publish(client_g, humid_topic, buff, 0, 0, 0);
    }
}

void publish_ldr_handler(void *args){
    char buff[21] = "";
    for(;;){
        xSemaphoreTake(LDR_Signal, portMAX_DELAY);
        ESP_LOGI(task_logging,"Task running: %s", "publish_ldr_handler");
        sprintf(buff, "%d", sensor_data.luminosity);
        esp_mqtt_client_publish(client_g, lumi_topic, buff, 0, 0, 0);
    }
}

void publish_window_state_handler(void *args){
    char buff[21] = "";
    for(;;){
        xSemaphoreTake(Window_state_Signal, portMAX_DELAY);
        ESP_LOGI(task_logging,"Task running: %s", "publish_window_state_handler");
        sprintf(buff, "%u", sensor_data.window_state);
        esp_mqtt_client_publish(client_g, window_state_topic, buff, 0, 0, 0);
    }
}