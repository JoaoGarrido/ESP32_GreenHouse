#include "IO.h"

/**Global variables**/
extern sensor_data_t sensor_data;
extern control_data_t control_data;
extern SemaphoreHandle_t read_DHT_Signal;
extern SemaphoreHandle_t read_LDR_Signal;
extern SemaphoreHandle_t x_Sem_C_Greenhouse;
/**Static variables**/
//Display variables
typedef enum _Menu_state{Menu_state_Data_menu, Menu_state_Main_menu, Menu_state_Max_temperature_menu, Menu_state_Min_temperature_menu, Menu_state_Config_menu} Menu_state;
typedef enum _Main_menu_select{Main_menu_select_Data_menu, Main_menu_select_Max_temperature_menu, Main_menu_select_Min_temperature_menu, Main_menu_select_Config_menu} Main_menu_select;
typedef enum _Config_menu_select{SOMETHING} Config_menu_select;
typedef struct{
    //Current menu
    Menu_state menu_state;
    Main_menu_select main_menu_select;
    const uint8_t main_menu_size;
    Config_menu_select config_menu_select;
    float temperature;
} display_data_t;
static display_data_t display_data = {Menu_state_Data_menu, Main_menu_select_Data_menu, 8, SOMETHING, 30.0};
static Buttons button_pressed = NONE;
enum Buttons_GPIO{GPIO_BTN_UP=35, GPIO_BTN_DOWN=34, GPIO_BTN_SEL=33, GPIO_BTN_BACK=32};
//ADC variables
#define DEFAULT_VREF    1100
#define NO_OF_SAMPLES   64          //Multisampling
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_0;   //GPIO36
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_0;
//DHT variables
static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
static const gpio_num_t GPIO_DHT = 5;
//Window
static const gpio_num_t GPIO_WINDOW = 1;
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
    //Checks
    check_efuse();
    /*Configure ADC*/
    adc1_config_width(width);
    adc1_config_channel_atten(channel, atten);
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
        .mode = GPIO_MODE_OUTPUT_OD,
        .pull_down_en = 0,
        .pull_up_en = 0,
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

void read_DHT(void *args){
    //Subscribe this task to TWDT, then check if it is subscribed
    CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
    CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

    for(;;){
        xSemaphoreTake(read_DHT_Signal, portMAX_DELAY);
        ESP_LOGI(dht_tag,"Task running: %s", "read_DHT");
        if (dht_read_float_data(sensor_type, GPIO_DHT, &(sensor_data.humidity), &(sensor_data.temperature)) == ESP_OK){
            CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);
            ESP_LOGI(dht_tag,"Temperature: %fºC || Humidity %f%%", sensor_data.temperature, sensor_data.humidity);
        }
        xSemaphoreGive(x_Sem_C_Greenhouse);
    }  
}   

void read_ldr(void *args) {
    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
    //Continuously sample ADC1
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
        xSemaphoreGive(x_Sem_C_Greenhouse);
    }
}

//Buttons
void IRAM_ATTR timer_button_isr(void *args){
    uint32_t button_pressed = 0;
    /*switch(){
        case BTN_UP:
        case BTN_DOWN:
        case BTN_SELECT:
        case BTN_BACK:
    }*/
    xTaskNotifyFromISR(read_buttons, button_pressed, eSetValueWithOverwrite, NULL);
    //portYIELD_FROM_ISR();
}

static void main_menu(){
    switch (button_pressed){
        case BTN_SELECT:
            switch(display_data.main_menu_select){
                case Main_menu_select_Data_menu:
                    display_data.menu_state = Menu_state_Data_menu;
                    break;
                case Main_menu_select_Max_temperature_menu:
                    display_data.menu_state = Menu_state_Max_temperature_menu;
                    display_data.temperature = control_data.temperature_max;
                    break;
                case Main_menu_select_Min_temperature_menu:
                    display_data.menu_state = Menu_state_Min_temperature_menu;
                    display_data.temperature = control_data.temperature_min;
                    break;
                case Main_menu_select_Config_menu:
                    display_data.menu_state = Menu_state_Config_menu;
                    break;
            }
            break;
        case BTN_UP:
            display_data.main_menu_select = (display_data.main_menu_size+display_data.main_menu_size+1)%display_data.main_menu_size;
            break;
        case BTN_DOWN:
            display_data.main_menu_select = (display_data.main_menu_size+display_data.main_menu_size-1)%display_data.main_menu_size;
            break;
        default:
            break;
    }
}

static void max_temperature_menu(){
    switch (button_pressed){
        case BTN_SELECT:
            display_data.menu_state = Menu_state_Main_menu;
            if(display_data.temperature < control_data.temperature_min){
                display_data.temperature = control_data.temperature_min + 1.0;
            }
            control_data.temperature_max = display_data.temperature;
            break;
        case BTN_UP:
            if(display_data.temperature <= 98.0){
                display_data.temperature += 1.0;
            }
            break;
        case BTN_DOWN:
            if(display_data.temperature >= 1.0){
                display_data.temperature -= 1.0;
            }            
            break;
        case BTN_BACK:
            display_data.menu_state = Menu_state_Main_menu;
            break;
        default:
            break;
    }
}

static void min_temperature_menu(){
    switch (button_pressed){
        case BTN_SELECT:
            display_data.menu_state = Menu_state_Main_menu;
            if(display_data.temperature > control_data.temperature_max){
                display_data.temperature = control_data.temperature_max - 1.0;
            }
            control_data.temperature_min = display_data.temperature;
            break;
        case BTN_UP:
            if(display_data.temperature <= 98.0){
                display_data.temperature += 1.0;
            }
            break;
        case BTN_DOWN:
            if(display_data.temperature >= 1.0){
                display_data.temperature -= 0.0;
            }            
            break;
        case BTN_BACK:
            display_data.menu_state = Menu_state_Main_menu;
            break;
        default:
            break;
    }
}

static void config_menu(){
    switch (button_pressed){
        case BTN_SELECT:
            display_data.menu_state = Menu_state_Main_menu;
            if(display_data.temperature > control_data.temperature_max){
                display_data.temperature = control_data.temperature_max - 1.0;
            }
            control_data.temperature_min = display_data.temperature;
            break;
        case BTN_UP:
            if(display_data.temperature <= 98.0){
                display_data.temperature += 1.0;
            }
            break;
        case BTN_DOWN:
            if(display_data.temperature >= 1.0){
                display_data.temperature -= 0.0;
            }            
            break;
        case BTN_BACK:
            display_data.menu_state = Menu_state_Main_menu;
            break;
        default:
            break;
    }
}

void read_buttons(void *args){
    uint32_t notification_value = 0;
    for(;;){
        xTaskNotifyWait(0x00, 0xffffffff, &notification_value, portMAX_DELAY);
        ESP_LOGI(buttons_tag,"Task running: %s", "read_buttons");
        button_pressed = notification_value;
        switch(display_data.menu_state){
            case Menu_state_Data_menu:
                //Move from data menu to main menu after any button is pressed
                display_data.menu_state = Menu_state_Main_menu;
                break;
            case Menu_state_Main_menu:
                main_menu();
                break;
            case Menu_state_Max_temperature_menu:
                max_temperature_menu();
                break;
            case Menu_state_Min_temperature_menu:
                min_temperature_menu();
                break;
            case Menu_state_Config_menu:
                config_menu();
                break;
        }
    }  
}

//Motor
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

void write_display(void *args){
    for(;;){
        ESP_LOGI(display_tag,"Task running: %s", "write_display");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}