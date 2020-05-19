#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/dac.h"
#include "esp_system.h"
#include "esp_adc_cal.h"
#include "esp_task_wdt.h"
#include "dht.h"

//
#include "tags.h"
#include "communications.h"
#include "greenhouse_system.h"


sensor_data_t sensor_data = {0.0, 0.0, 0, 0};
//DHT
static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
//#if defined(CONFIG_IDF_TARGET_ESP8266)
//static const gpio_num_t dht_gpio = 4;
//#else
static const gpio_num_t dht_gpio = 4;
//#endif

//ADC variables
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
void read_DHT22(void *args);
void update_motor_status(void *args);
void control_greenhouse(void *args);
void write_display(void *args);
void logging(void *args);
static void check_efuse(void);
static void print_char_val_type(esp_adc_cal_value_t val_type);

//Semaphores
SemaphoreHandle_t DHT_Signal = NULL;
SemaphoreHandle_t LDR_Signal = NULL;
SemaphoreHandle_t Window_state_Signal = NULL;

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
    check_efuse();
     //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(width);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }
}

void init_sync_variables(){
    DHT_Signal = xSemaphoreCreateBinary();
    LDR_Signal = xSemaphoreCreateBinary();
    Window_state_Signal = xSemaphoreCreateBinary();
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
        if (dht_read_float_data(sensor_type, dht_gpio, &(sensor_data.humidity), &(sensor_data.temperature)) == ESP_OK){
            CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);
            ESP_LOGI(task_logging,"Task running: %s", "read_DHT22");
            ESP_LOGI(dht22_tag,"Temperature: %fºC || Humidity %f%%", sensor_data.temperature, sensor_data.humidity);
        }
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

void read_ldr(void *args) {
    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
    //Continuously sample ADC1
    for(;;){
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
    init_sync_variables();

    //Application Tasks  
    xTaskCreatePinnedToCore(read_DHT22, "read_DHT22", TASK_STACK_MIN_SIZE, NULL, 5, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(update_motor_status, "update_motor_status", TASK_STACK_MIN_SIZE, NULL, 4, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(control_greenhouse, "control_greenhouse", TASK_STACK_MIN_SIZE, NULL, 3, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(write_display, "write_display", TASK_STACK_MIN_SIZE, NULL, 2, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(logging, "logging", TASK_STACK_MIN_SIZE, NULL, 1, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(read_ldr, "read_ldr", TASK_STACK_MIN_SIZE, NULL, 6, NULL, APPLICATION_CORE);

    //MQTT Tasks
    xTaskCreatePinnedToCore(publish_dht_handler, "publish_dht_handler", TASK_STACK_MIN_SIZE, NULL, 2, NULL, WIFI_COMMUNICATIONS_CORE);
    xTaskCreatePinnedToCore(publish_ldr_handler, "publish_ldr_handler", TASK_STACK_MIN_SIZE, NULL, 2, NULL, WIFI_COMMUNICATIONS_CORE);
    xTaskCreatePinnedToCore(publish_window_state_handler, "publish_window_state_handler", TASK_STACK_MIN_SIZE, NULL, 2, NULL, WIFI_COMMUNICATIONS_CORE);

}