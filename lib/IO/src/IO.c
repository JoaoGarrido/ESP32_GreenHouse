#include "IO.h"

/**Global variables**/
extern sensor_data_t sensor_data;
extern control_data_t control_data;
extern SemaphoreHandle_t read_DHT_Signal;
extern SemaphoreHandle_t read_LDR_Signal;
extern SemaphoreHandle_t x_Sem_C_Greenhouse;
extern void button_handler(void *args);
extern TaskHandle_t th_button_handler;
/**Private variables**/
static const gpio_num_t GPIO_BTN_UP = 26;
static const gpio_num_t GPIO_BTN_DOWN = 25;
static const gpio_num_t GPIO_BTN_SEL = 33;
static const gpio_num_t GPIO_BTN_BACK = 32;
//ADC variables
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_0;   //GPIO36
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_0;
//DHT variables
static const dht_sensor_type_t sensor_type = DHT_TYPE_AM2301;
static const gpio_num_t GPIO_DHT = 5;
//Window
static const gpio_num_t GPIO_WINDOW = 27;

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

/*Init ports*/
void initialize_ports(){
    /*Configure ADC*/
    adc1_config_width(width);
    adc1_config_channel_atten(channel, atten);
    check_efuse();
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
    /*Configure GPIO*/
    //Buttons
    const uint64_t GPIO_BUTTON_MASK = (1ULL << GPIO_BTN_UP) | (1ULL << GPIO_BTN_DOWN) | (1ULL << GPIO_BTN_SEL) | (1ULL << GPIO_BTN_BACK);
    gpio_config_t button_config = {
        .pin_bit_mask = GPIO_BUTTON_MASK,
        .intr_type = GPIO_PIN_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = 1,
        .pull_up_en = 0,
    };
    gpio_config(&button_config);
    //DHT
    const uint64_t GPIO_DHT_MASK = (1ULL << GPIO_DHT);
    gpio_config_t dht_config = {
        .pin_bit_mask = GPIO_DHT_MASK,
        .intr_type = GPIO_PIN_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&dht_config);
    //Window
    const uint64_t GPIO_WINDOW_MASK = (1ULL << GPIO_WINDOW);
    gpio_config_t window_config = {
        .pin_bit_mask = GPIO_WINDOW_MASK,
        .intr_type = GPIO_PIN_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT_OD,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&window_config);
}

/*DHT*/
void read_DHT(void *args){
    //Subscribe this task to TWDT, then check if it is subscribed
    //CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
    //CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

    for(;;){
        xSemaphoreTake(read_DHT_Signal, portMAX_DELAY);
        ESP_LOGI(dht_tag,"Task running: %s", "read_DHT");
        if (dht_read_float_data(sensor_type, GPIO_DHT, &(sensor_data.humidity), &(sensor_data.temperature)) == ESP_OK){
            //CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);
            ESP_LOGI(dht_tag,"Temperature: %fºC || Humidity %f%%", sensor_data.temperature, sensor_data.humidity);
        }
    }  
}   

/*LDR*/
void read_ldr(void *args) {
    for(;;){
        xSemaphoreTake(read_LDR_Signal, portMAX_DELAY);
        ESP_LOGI(ldr_tag,"Task running: %s", "read_ldr");
        uint32_t adc_reading = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++){
            adc_reading += adc1_get_raw((adc1_channel_t)channel);
        }
        adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        ESP_LOGI(ldr_tag, "Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
        ESP_LOGI(ldr_tag, "ADC1 CH%d Raw: %d\t\n", channel, adc_reading);
    }
}

/*Buttons*/
static uint32_t button_debounce(uint32_t button_name, uint64_t button_gpio){
    static uint16_t button_state[N_BUTTONS] = {0,0,0,0};
    volatile uint8_t button_read = 0;
    //ets_printf("reading_gpio\n");

    button_read = gpio_get_level(button_gpio);
    if(button_read) ets_printf("GPIO %lu pressed\n", button_gpio);
    button_state[button_name] = ( (button_state[button_name] << 1) | button_read | 0xE000);

    if(button_state[button_name] == 0xF000){
        ets_printf("Button %u from GPIO %lu Active\n");
        return 1;
    }
    return 0;
}

void IRAM_ATTR timer_button_isr(void *args){
    TIMERG0.hw_timer[0].update = 1;
    
    /*Queues vs xTaskNotifyFromISR:
    With queues we can handle the button presses at the same time 
    While if we use taskNotify and 2 buttons are pressed at exactly the same time, one of the presses will probably be ignored
    This situation is highly improbable and xTaskNotify is faster and uses less RAM
    */
    if(button_debounce((uint32_t)BTN_UP, (uint64_t)GPIO_BTN_UP)){
        xTaskNotifyFromISR(th_button_handler, (uint32_t)BTN_UP, eSetValueWithoutOverwrite, NULL);
    }
    if(button_debounce(BTN_DOWN, GPIO_BTN_DOWN)){
        xTaskNotifyFromISR(th_button_handler, (uint32_t)BTN_DOWN, eSetValueWithoutOverwrite, NULL);
    }
    if(button_debounce(BTN_SELECT, GPIO_BTN_SEL)){
        xTaskNotifyFromISR(th_button_handler, (uint32_t)BTN_SELECT, eSetValueWithoutOverwrite, NULL);
    }
    if(button_debounce(BTN_BACK, GPIO_BTN_BACK)){
        xTaskNotifyFromISR(th_button_handler, (uint32_t)BTN_BACK, eSetValueWithoutOverwrite, NULL);
    }
    //
    TIMERG0.int_clr_timers.t0 = 1;
    //timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0); //new ESP-IDF version
    
    TIMERG0.hw_timer[0].config.alarm_en = TIMER_ALARM_EN;
    
}

/*Window*/
void write_motor_state(void *args){
    for(;;){
        uint32_t output_level = 3;
        xTaskNotifyWait(0x00, 0xffffffff, &output_level, portMAX_DELAY);
        ESP_LOGI(motor_tag,"Task running: %s", "update_motor_status");
        if(output_level < 3){
            gpio_set_level(GPIO_WINDOW, output_level);
        }
        else{
            ESP_LOGI(motor_tag,"ERROR invalid output level");
        }
    }  
}

/*Display*/
void write_display(void *args){
    for(;;){
        ESP_LOGI(display_tag,"Task running: %s", "write_display");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}