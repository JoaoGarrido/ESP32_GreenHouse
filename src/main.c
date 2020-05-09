#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
<<<<<<< HEAD
#include "mqtt_client.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/dac.h"
#include "esp_system.h"
#include "esp_adc_cal.h"
=======
#include "esp_task_wdt.h"
#include "dht.h"
//#include "dht11.h"
//#include "driver/gpio.h"
//#include "sdkconfig.h"
>>>>>>> Eman_branch

//Cores
#define WIFI_COMMUNICATIONS_CORE 0
#define APPLICATION_CORE 1

//SSID config
<<<<<<< HEAD
#define WIFI_SSID ""
#define WIFI_PASS ""
=======
#define EXAMPLE_WIFI_SSID "TheMarley"
#define EXAMPLE_WIFI_PASS "7EC2A4F115"
>>>>>>> Eman_branch
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

//MQTT config
#define BROKER_URL "mqtts://mqtt.eclipse.org"
#define BROKER_PORT 8883
//extern const uint8_t mqtt_eclipse_org_pem_start[]   asm("_binary_mqtt_eclipse_org_pem_start");
//extern const uint8_t mqtt_eclipse_org_pem_end[]   asm("_binary_mqtt_eclipse_org_pem_end");

//Logging
<<<<<<< HEAD
#define wifi_tag "Wifi"
#define mqtt_tag "MQTT"
#define dht22_tag "DHT22"
#define task_logging "Task_logging"
#define startup_tag "Startup"
#define memory_tag "Memory"
#define ldr_tag "LDR"
=======
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
static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
//#if defined(CONFIG_IDF_TARGET_ESP8266)
//static const gpio_num_t dht_gpio = 4;
//#else
static const gpio_num_t dht_gpio = 4;
//#endif
>>>>>>> Eman_branch


#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling


static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;

static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

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
static void check_efuse(void);
static void print_char_val_type(esp_adc_cal_value_t val_type);

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
<<<<<<< HEAD
    check_efuse();
     //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(width);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }
=======
   //DHT11_init(GPIO_NUM_4);
>>>>>>> Eman_branch
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
    //ESP_LOGI(wifi_tag,"Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static void initialize_mqtt_app(){
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = BROKER_URL,
        .port = BROKER_PORT,
        //.cert_pem = (const char *)mqtt_eclipse_org_pem_start,
        .task_prio = 4,
    };
    ESP_LOGI(memory_tag, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

static void check_efuse(void){
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        ESP_LOGI(ldr_tag, "eFuse Two Point: Supported\n");
    } else {
        ESP_LOGI(ldr_tag, "eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        ESP_LOGI(ldr_tag, "eFuse Vref: Supported\n");
    } else {
        ESP_LOGI(ldr_tag, "eFuse Vref: NOT supported\n");
    }
}
static void print_char_val_type(esp_adc_cal_value_t val_type){
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        ESP_LOGI(ldr_tag, "Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        ESP_LOGI(ldr_tag, "Characterized using eFuse Vref\n");
    } else {
        ESP_LOGI(ldr_tag, "Characterized using Default Vref\n");
    }
}

//Loop task functions
void read_DHT22(void *args){
    //Subscribe this task to TWDT, then check if it is subscribed
    CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
    CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

    for(;;){
        
          //Comment this line to trigger a TWDT timeout
        float humidity = 0.0;
        float  temperature = 0.0;
        if (dht_read_float_data(sensor_type, dht_gpio, &humidity, &temperature) == ESP_OK){
            CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);
            ESP_LOGI(task_logging,"Task running: %s", "read_DHT22");
            ESP_LOGI(dht22_tag,"Temperature: %fºC || Humidity %f%%", temperature, humidity);
        }
        //humidity = DHT11_read().humidity;
        //temperature = DHT11_read().temperature;
        

        vTaskDelay(2000 / portTICK_RATE_MS);
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

void read_ldr(void *args) {
    

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
    //Continuously sample ADC1
    while (1) {
        uint32_t adc_reading = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit == ADC_UNIT_1) {
                adc_reading += adc1_get_raw((adc1_channel_t)channel);
            } else {
                int raw;
                adc2_get_raw((adc2_channel_t)channel, width, &raw);
                adc_reading += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        ESP_LOGI(ldr_tag, "Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
        ESP_LOGI(ldr_tag, "ADC%d CH%d Raw: %d\t\n", unit, channel, adc_reading);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

static void mqtt_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    ESP_LOGD(mqtt_tag, "Event dispatched from event loop base=%s, event_id=%d", event_base, event_id);
    mqtt_event_handler_callback(event_data);
}

void wifi_send_data_to_broker(void *args){
    for(;;){
       // ESP_LOGI(task_logging,"Task running: %s", "wifi_send_data_to_broker");
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
    //initialize_mqtt_app();

    //Application Tasks  
    xTaskCreatePinnedToCore(read_DHT22, "read_DHT22", TASK_STACK_MIN_SIZE, NULL, 5, NULL, APPLICATION_CORE);
<<<<<<< HEAD
    xTaskCreatePinnedToCore(update_motor_status, "update_motor_status", TASK_STACK_MIN_SIZE, NULL, 4, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(control_greenhouse, "control_greenhouse", TASK_STACK_MIN_SIZE, NULL, 3, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(write_display, "write_display", TASK_STACK_MIN_SIZE, NULL, 2, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(logging, "logging", TASK_STACK_MIN_SIZE, NULL, 1, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(read_ldr, "read_ldr", TASK_STACK_MIN_SIZE, NULL, 6, NULL, APPLICATION_CORE);

    //Wifi Tasks
    xTaskCreatePinnedToCore(wifi_send_data_to_broker, "wifi_send_data_to_broker", TASK_STACK_MIN_SIZE, NULL, 4, NULL, WIFI_COMMUNICATIONS_CORE);
}