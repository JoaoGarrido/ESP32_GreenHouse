#include "IO.h"

/**Global variables**/
extern sensor_data_t sensor_data;
extern control_data_t control_data;
extern SemaphoreHandle_t read_DHT_Signal;
extern SemaphoreHandle_t read_LDR_Signal;
extern SemaphoreHandle_t x_Sem_C_Greenhouse;
extern void button_handler(void *args);
extern QueueHandle_t xButtonQueue;

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
static const dht_sensor_type_t sensor_type = CONFIG_TEMP_HUMID_SENSOR;
static const gpio_num_t GPIO_DHT = 5;
//Window
static const gpio_num_t GPIO_WINDOW = 27;
//For task time test
static const gpio_num_t GPIO_TEST = 14;

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
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&window_config);

    const uint64_t GPIO_TEST_MASK = (1ULL << GPIO_TEST) | (1ULL << 13) | (1ULL << 12) | (1ULL << 15) | (1ULL << 2) | (1ULL << 4);
    gpio_config_t test_config = {
        .pin_bit_mask = GPIO_TEST_MASK,
        .intr_type = GPIO_PIN_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&test_config);
}

/*DHT*/
void read_DHT(void *args){
    //Subscribe this task to TWDT, then check if it is subscribed
    //CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
    //CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);
    float humidity = 0.0, temperature = 0.0;
    for(;;){
        xSemaphoreTake(read_DHT_Signal, portMAX_DELAY);
        DEBUG_GPIO(GPIO_CHANNEL_2,
            ESP_LOGI(dht_tag,"Task running: %s", "read_DHT");
            if (dht_read_float_data(sensor_type, GPIO_DHT, &humidity, &temperature) == ESP_OK){
                //CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);
                ESP_LOGI(dht_tag,"Temperature: %fºC || Humidity %f%%", sensor_data.temperature, sensor_data.humidity);
                sensor_data.temperature = temperature;
                sensor_data.humidity = humidity;
            }
        )
        xSemaphoreGive(x_Sem_C_Greenhouse);
    }  
}   

/*LDR*/
void read_ldr(void *args) {
    for(;;){
        xSemaphoreTake(read_LDR_Signal, portMAX_DELAY);
        DEBUG_GPIO(GPIO_CHANNEL_3,
            ESP_LOGI(ldr_tag,"Task running: %s", "read_ldr");
            uint32_t adc_reading = 0;
            //Multisampling
            for (int i = 0; i < NO_OF_SAMPLES; i++){
                adc_reading += adc1_get_raw((adc1_channel_t)channel);
            }
            adc_reading /= NO_OF_SAMPLES;
            //Convert adc_reading to voltage in mV
            uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
            //ESP_LOGI(ldr_tag, "Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
            ESP_LOGI(ldr_tag, "ADC1 CH%d Raw: %d\n", channel, adc_reading);
            //NEED TO: CONVERT TO LUMENS OR %
            sensor_data.luminosity = adc_reading;
        )
        xSemaphoreGive(x_Sem_C_Greenhouse);
    }
}

/*Buttons*/
static uint32_t button_debounce(uint32_t button_name, uint64_t button_gpio){
    static uint16_t button_state[N_BUTTONS] = {0,0,0,0};
    volatile uint8_t button_read = 0;
    //ets_printf("reading_gpio\n");

    button_read = gpio_get_level(button_gpio);
    if(button_read) ets_printf("Button %u from GPIO %lu Pressed\n");;
    button_state[button_name] = ( (button_state[button_name] << 1) | button_read | 0xE000);

    if(button_state[button_name] == 0xF000){
        ets_printf("Button %u from GPIO %lu Active\n");
        return 1;
    }
    return 0;
}

void IRAM_ATTR timer_button_isr(void *args){
    TIMERG0.hw_timer[0].update = 1;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint8_t button_to_queue;
    /*Queues vs xTaskNotifyFromISR:
    With queues we can handle two button presses at the same time 
    While if we use taskNotify and 2 buttons are pressed before the button handler ends, one of the presses will be ignored
    */
    DEBUG_GPIO(GPIO_CHANNEL_5,
        if(button_debounce(BTN_UP, GPIO_BTN_UP)){
            button_to_queue = BTN_UP;
            xQueueSendFromISR( xButtonQueue, &button_to_queue, &xHigherPriorityTaskWoken);
            //gpio_set_level(GPIO_TEST, 1);
        }
        if(button_debounce(BTN_DOWN, GPIO_BTN_DOWN)){
            button_to_queue = BTN_DOWN;
            xQueueSendFromISR( xButtonQueue, &button_to_queue, &xHigherPriorityTaskWoken);    
        }
        if(button_debounce(BTN_SELECT, GPIO_BTN_SEL)){
            button_to_queue = BTN_SELECT;
            xQueueSendFromISR( xButtonQueue, &button_to_queue, &xHigherPriorityTaskWoken);    
            gpio_set_level(4, 0);
        }
        if(button_debounce(BTN_BACK, GPIO_BTN_BACK)){
            button_to_queue = BTN_BACK;
            xQueueSendFromISR( xButtonQueue, &button_to_queue, &xHigherPriorityTaskWoken);
        }
    )
    //Clear intr flag
    //TIMERG0.int_clr_timers.t0 = 1; //ESP-IDF 3.xx
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0); //ESP-IDF version 4.xx
    //Re-enable interrupt
    TIMERG0.hw_timer[0].config.alarm_en = TIMER_ALARM_EN;

}

/*Window*/
void write_motor_state(void *args){
    for(;;){
        uint32_t output_level = 3;
        sensor_data.window_state = gpio_get_level(GPIO_WINDOW);
        xTaskNotifyWait(0x00, 0xffffffff, &output_level, portMAX_DELAY);
        DEBUG_GPIO(GPIO_CHANNEL_4,
            ESP_LOGI(motor_tag,"Task running: %s", "update_motor_status");
            ets_printf("%d", output_level);
            if(output_level < 3){            
                gpio_set_level(GPIO_WINDOW, output_level);
                //gpio_set_level(GPIO_TEST, 0);
            }
            else{
                ESP_LOGI(motor_tag,"ERROR invalid output level");
                //gpio_set_level(GPIO_TEST, 0);
            }
        )
    }  
}

void write_stats(void *args) {
    //ESP_LOGI(memory_tag, "***********************[APP] ESTA A EXECUTAR O FUCKING RUN TIME STATS ******************************************************");
    char str[500];
    for(;;) {
        vTaskGetRunTimeStats(str);
        ets_printf(str);
        vTaskList(str);
        ets_printf(str);
        vTaskDelay(10000 / portTICK_RATE_MS);
    }
}