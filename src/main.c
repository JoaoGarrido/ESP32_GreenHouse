#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "cert.h"

//Cores
#define WIFI_COMMUNICATIONS_CORE 0
#define APPLICATION_CORE 1

//SSID config
#define WIFI_SSID "ExampleSSDI"
#define WIFI_PASS "ExamplePW"
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;


//MQTT config
#define BROKER_URL "mqtts://mqtt.eclipse.org"
#define BROKER_PORT 8883
static const uint8_t mqtt_eclipse_org_pem_start[]  = PEM_CERT;

//Logging
#define wifi_tag "Wifi"
#define mqtt_tag "MQTT"
#define dht22_tag "DHT22"
#define task_logging "Task_logging"
#define startup_tag "Startup"
#define memory_tag "Memory"

#define TASK_STACK_MIN_SIZE 8000

//Header
void initialize_nvs();
void initialize_ports();
void initialize_wifi_sta_mode();
static void initialize_mqtt_app();
void read_DHT22(void *args);
void update_motor_status(void *args);
void control_greenhouse(void *args);
void write_display(void *args);
void logging(void *args);
static esp_err_t wifi_event_handler(void *ctx, system_event_t *event);
static void mqtt_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static esp_err_t mqtt_event_handler_callback(esp_mqtt_event_handle_t event);
void wifi_send_data_to_broker(void *args);

//Init functions
void initialize_nvs(){
    // Initialize Non volatile storage -> needs to be initialized because of the wifi
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
}

void initialize_ports(){

}

void initialize_wifi_sta_mode(){
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
    ESP_LOGI(wifi_tag,"Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static void initialize_mqtt_app(){
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = BROKER_URL,
        .port = BROKER_PORT,
        .cert_pem = (const char *)mqtt_eclipse_org_pem_start,
        .task_prio = 4,
    };
    ESP_LOGI(memory_tag, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

//Loop task functions
void read_DHT22(void *args){
    for(;;){
        ESP_LOGI(task_logging,"Task running: %s", "read_DHT22");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }  
}   

void update_motor_status(void *args){
    for(;;){
        ESP_LOGI(task_logging,"Task running: %s", "update_motor_status");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }  
}

void control_greenhouse(void *args){
    for(;;){
        ESP_LOGI(task_logging,"Task running: %s", "control_greenhouse");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }  
}

void write_display(void *args){
    for(;;){
        ESP_LOGI(task_logging,"Task running: %s", "write_display");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }  
}

void logging(void *args){
    for(;;){
        ESP_LOGI(task_logging,"Task running: %s", "Logging");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }  
}

//WiFi Communications functions
static esp_err_t wifi_event_handler(void *ctx, system_event_t *event){
    ESP_LOGI(wifi_tag,"Task running: %s", "wifi_event_handler");
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

static esp_err_t mqtt_event_handler_callback(esp_mqtt_event_handle_t event){
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(mqtt_tag, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "/esp32_greenhouse/dht22", 1);
            ESP_LOGI(mqtt_tag, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/esp32_greenhouse/temperature_opening_limit", 1);
            ESP_LOGI(mqtt_tag, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/esp32_greenhouse/temperature_closing_limit", 1);
            ESP_LOGI(mqtt_tag, "sent subscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(mqtt_tag, "MQTT_EVENT_DISCONNECTED");
            break;  

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(mqtt_tag, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/esp32_greenhouse/temperature_opening_limit", "data", 0, 0, 0);
            ESP_LOGI(mqtt_tag, "sent publish successful, msg_id=%d", msg_id);
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

static void mqtt_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    ESP_LOGD(mqtt_tag, "Event dispatched from event loop base=%s, event_id=%d", event_base, event_id);
    mqtt_event_handler_callback(event_data);
}

void wifi_send_data_to_broker(void *args){
    for(;;){
        ESP_LOGI(task_logging,"Task running: %s", "wifi_send_data_to_broker");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }    
}

void app_main(){
    //Startup info
    ESP_LOGI(startup_tag, "[APP] Startup..");
    ESP_LOGI(startup_tag, "[APP] IDF version: %s", esp_get_idf_version());
    ESP_LOGI(memory_tag, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    //init
    initialize_nvs();
    initialize_wifi_sta_mode();
    initialize_ports();
    initialize_mqtt_app();

    //Application Tasks  
    xTaskCreatePinnedToCore(read_DHT22, "read_DHT22", TASK_STACK_MIN_SIZE, NULL, 5, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(update_motor_status, "update_motor_status", TASK_STACK_MIN_SIZE, NULL, 4, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(control_greenhouse, "control_greenhouse", TASK_STACK_MIN_SIZE, NULL, 3, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(write_display, "write_display", TASK_STACK_MIN_SIZE, NULL, 2, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(logging, "logging", TASK_STACK_MIN_SIZE, NULL, 1, NULL, APPLICATION_CORE);
    //Wifi Tasks
    xTaskCreatePinnedToCore(wifi_send_data_to_broker, "wifi_send_data_to_broker", TASK_STACK_MIN_SIZE, NULL, 4, NULL, WIFI_COMMUNICATIONS_CORE);
}