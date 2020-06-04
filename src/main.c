//System libraries
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/timer.h"
#include "driver/periph_ctrl.h"
//Project libraries
#include "logging.h"
#include "communications.h"
#include "greenhouse_system.h"
#include "IO.h"
#include "display_controller.h"
#include "display.h"
//Header
static void init_sync_variables();
static void init_button_timer(double timer_interval_msec);
static void control_greenhouse(void *args);
void app_main();

/**Global variables**/
//Data
sensor_data_t sensor_data = {0.0, 0.0, 0, Window_state_Closed};
control_data_t control_data = {45.0, 30.0, Window_action_Close, Mode_Auto};
//Semaphores
SemaphoreHandle_t publish_DHT_Signal = NULL;
SemaphoreHandle_t publish_LDR_Signal = NULL;
SemaphoreHandle_t publish_WindowState_Signal = NULL;
SemaphoreHandle_t publish_Control_data_Signal = NULL;
SemaphoreHandle_t read_DHT_Signal = NULL;
SemaphoreHandle_t read_LDR_Signal = NULL;
SemaphoreHandle_t x_Sem_C_Greenhouse = NULL;
//Task Handlers
TaskHandle_t th_button_handler;
TaskHandle_t th_write_motor_state;

//Init functions
static void init_sync_variables(){
    publish_DHT_Signal = xSemaphoreCreateBinary();
    publish_LDR_Signal = xSemaphoreCreateBinary();
    publish_WindowState_Signal = xSemaphoreCreateBinary();
    publish_Control_data_Signal = xSemaphoreCreateBinary();
    read_DHT_Signal = xSemaphoreCreateBinary();
    read_LDR_Signal = xSemaphoreCreateBinary();
    x_Sem_C_Greenhouse = xSemaphoreCreateCounting(2, 0);
}

static void init_button_timer(double timer_interval_msec){
    /* Select and initialize basic parameters of the timer */
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = 1,
    }; // default clock source is APB
    timer_init(TIMER_GROUP_0, TIMER_0, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, timer_interval_msec * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_register(TIMER_GROUP_0, TIMER_0, timer_button_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);
    timer_start(TIMER_GROUP_0, TIMER_0);
    ESP_LOGI(startup_tag, "Timer button init END");
}

//Loop task functions
static void control_greenhouse(void *args){
    for(;;){
        ESP_LOGI(task_logging,"Task running: %s", "control_greenhouse");
        //Trigger read tasks
        xSemaphoreGive(read_DHT_Signal);
        xSemaphoreGive(read_LDR_Signal);
        ESP_LOGI(task_logging,"Task running: %s", "control_greenhouse | SemaphoreGive to DHT and LDR");
        //Wait for read to finish 
        for(int i = 0; i < 2; i++){
            xSemaphoreTake(x_Sem_C_Greenhouse, portMAX_DELAY);
        }
        ESP_LOGI(task_logging,"Task running: %s", "control_greenhouse | Passed counting semaphore DHT and LDR");
        xQueueReset(x_Sem_C_Greenhouse); //Hack->Use reset queue to reset counting semaphore
        //Control Algorithm
        ESP_LOGI(task_logging,"Task running: %s | Mode: %d | Max Temp: %f | Min Temp: %f", "control_greenhouse", control_data.mode, control_data.temperature_max, control_data.temperature_min);
        
        if(control_data.mode == Mode_Auto){
            if(sensor_data.temperature > control_data.temperature_max){
                control_data.window_action = Window_action_Open;
            }
            else if(sensor_data.temperature < control_data.temperature_min){
                control_data.window_action = Window_action_Close;
            }
        }
        //Write Motor State
        if((int)control_data.window_action != (int)sensor_data.window_state){
            xTaskNotify(th_write_motor_state, control_data.window_action, eSetValueWithOverwrite);
        }
        //Trigger publish tasks on WiFi Core
        xSemaphoreGive(publish_DHT_Signal);
        xSemaphoreGive(publish_LDR_Signal);
        xSemaphoreGive(publish_WindowState_Signal);
        xSemaphoreGive(publish_Control_data_Signal);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void app_main(){
    //Startup info
    ESP_LOGI(startup_tag, "[APP] Startup..");
    ESP_LOGI(startup_tag, "[APP] IDF version: %s", esp_get_idf_version());
    ESP_LOGI(memory_tag, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    
    //Init
    init_sync_variables();
    initialize_wifi_sta_mode();
    initialize_ports();
    initialize_mqtt_app();
    init_display();
    
    init_button_timer(10);

    //Application Tasks  
    xTaskCreatePinnedToCore(write_motor_state, "write_motor_state", TASK_STACK_MIN_SIZE, NULL, 11, &th_write_motor_state, APPLICATION_CORE);
    xTaskCreatePinnedToCore(button_handler, "button_handler", TASK_STACK_MIN_SIZE, NULL, 10, &th_button_handler, APPLICATION_CORE);
    xTaskCreatePinnedToCore(control_greenhouse, "control_greenhouse", TASK_STACK_MIN_SIZE, NULL, 6, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(read_DHT, "read_DHT", TASK_STACK_MIN_SIZE, NULL, 5, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(read_ldr, "read_ldr", TASK_STACK_MIN_SIZE, NULL, 5, NULL, APPLICATION_CORE);
    //xTaskCreatePinnedToCore(write_display, "write_display", TASK_STACK_MIN_SIZE, NULL, 4, NULL, APPLICATION_CORE);

    //MQTT Tasks
    xTaskCreatePinnedToCore(publish_dht_handler, "publish_dht_handler", TASK_STACK_MIN_SIZE, NULL, 1, NULL, WIFI_COMMUNICATIONS_CORE);
    xTaskCreatePinnedToCore(publish_ldr_handler, "publish_ldr_handler", TASK_STACK_MIN_SIZE, NULL, 1, NULL, WIFI_COMMUNICATIONS_CORE);
    xTaskCreatePinnedToCore(publish_window_handler, "publish_window_handler", TASK_STACK_MIN_SIZE, NULL, 1, NULL, WIFI_COMMUNICATIONS_CORE);
    xTaskCreatePinnedToCore(publish_control_data_handler, "publish_control_data_handler", TASK_STACK_MIN_SIZE, NULL, 1, NULL, WIFI_COMMUNICATIONS_CORE);
}