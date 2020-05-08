#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_task_wdt.h"
//#include "dht.h"
#include "dht11.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

//Cores
#define WIFI_COMMUNICATIONS_CORE 0
#define APPLICATION_CORE 1

//SSID config
#define EXAMPLE_WIFI_SSID "The Marley"
#define EXAMPLE_WIFI_PASS "7EC2A4F115"
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

//Logging
#define wifi_tag "[Wifi]"
#define mqtt_tag "[MQTT]"
#define dht22_tag "[DHT22]"
#define task_logging "[Task_logging]"
#define watchdog "[Watchdog]"

#define TASK_STACK_MIN_SIZE 10000

//Watchdog timers and macro
#define TWDT_TIMEOUT_S          3
#define TASK_RESET_PERIOD_S     2


#define CHECK_ERROR_CODE(returned, expected) ({                        \
            if(returned != expected){                                  \
                ESP_LOGI(watchdog,"Watchdog timer ERROR");             \
                abort();                                               \
            }                                                          \
})


//DHT
//static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
//#if defined(CONFIG_IDF_TARGET_ESP8266)
//static const gpio_num_t dht_gpio = 4;
//#else
//static const gpio_num_t dht_gpio = 4;
//#endif


//Header
void initialize_nvs();
void initialize_ports();
void initialize_wifi_sta_mode();
void read_DHT22(void *args);
void update_motor_status(void *args);
void control_greenhouse(void *args);
void write_display(void *args);
void logging(void *args);
static esp_err_t wifi_event_handler(void *ctx, system_event_t *event);
void wifi_send_data_to_broker(void *args);

//Init functions
void initialize_nvs(){
    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
}

void initialize_ports(){
   DHT11_init(GPIO_NUM_4);
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
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    //ESP_LOGI(wifi_tag,"Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

//Loop task functions
void read_DHT22(void *args){

    //Subscribe this task to TWDT, then check if it is subscribed
    CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
    CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

    for(;;){
        CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);  //Comment this line to trigger a TWDT timeout
        float humidity = 0;
        float temperature = 0;
        //dht_read_float_data(sensor_type, dht_gpio, &humidity, &temperature);
        humidity = DHT11_read().humidity;
        temperature = DHT11_read().temperature;
        ESP_LOGI(task_logging,"Task running: %s", "read_DHT22");
        ESP_LOGI(dht22_tag,"Temperature: %f || Humidity %f", temperature, humidity);

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

void wifi_send_data_to_broker(void *args){
    for(;;){
       // ESP_LOGI(task_logging,"Task running: %s", "wifi_send_data_to_broker");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }    
}

void app_main(){
    //init wifi
    initialize_nvs();
    initialize_wifi_sta_mode();
    initialize_ports();

    //Application Tasks  
    xTaskCreatePinnedToCore(read_DHT22, "read_DHT22", TASK_STACK_MIN_SIZE*3, NULL, 5, NULL, APPLICATION_CORE);
    //xTaskCreatePinnedToCore(update_motor_status, "update_motor_status", TASK_STACK_MIN_SIZE, NULL, 4, NULL, APPLICATION_CORE);
    //xTaskCreatePinnedToCore(control_greenhouse, "control_greenhouse", TASK_STACK_MIN_SIZE, NULL, 3, NULL, APPLICATION_CORE);
    //xTaskCreatePinnedToCore(write_display, "write_display", TASK_STACK_MIN_SIZE, NULL, 2, NULL, APPLICATION_CORE);
    //xTaskCreatePinnedToCore(logging, "logging", TASK_STACK_MIN_SIZE, NULL, 1, NULL, APPLICATION_CORE);
    //Wifi Tasks
    //xTaskCreatePinnedToCore(wifi_send_data_to_broker, "wifi_send_data_to_broker", TASK_STACK_MIN_SIZE, NULL, 4, NULL, WIFI_COMMUNICATIONS_CORE);
    
}