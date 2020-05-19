#include "IO.h"

/**Global variables**/
extern sensor_data_t sensor_data;
/**Static variables**/
//ADC variables
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;
//DHT variables
static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
static const gpio_num_t dht_gpio = 4;
/**Static functions**/
static void check_efuse(void);
static void print_char_val_type(esp_adc_cal_value_t val_type);


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

void update_motor_status(void *args){
    for(;;){
        ESP_LOGI(task_logging,"Task running: %s", "update_motor_status");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }  
}

void write_display(void *args){
    for(;;){
       ESP_LOGI(task_logging,"Task running: %s", "write_display");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }  
}