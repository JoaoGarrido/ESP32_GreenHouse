//System libraries
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
//Project libraries
#include "logging.h"
#include "communications.h"
#include "greenhouse_system.h"
#include "IO.h"

//Header
static void control_greenhouse(void *args);

/**Global variables**/
//Data
sensor_data_t sensor_data = {0.0, 0.0, 0, 0};
control_data_t control_data = {0.0, 0.0, 0};
//Semaphores
SemaphoreHandle_t publish_DHT_Signal = NULL;
SemaphoreHandle_t publish_LDR_Signal = NULL;
SemaphoreHandle_t publish_WindowState_Signal = NULL;
SemaphoreHandle_t read_DHT_Signal = NULL;
SemaphoreHandle_t read_LDR_Signal = NULL;
SemaphoreHandle_t x_Sem_C_Greenhouse = NULL;

//Init functions
void init_sync_variables(){
    publish_DHT_Signal = xSemaphoreCreateBinary();
    publish_LDR_Signal = xSemaphoreCreateBinary();
    publish_WindowState_Signal = xSemaphoreCreateBinary();
    read_DHT_Signal = xSemaphoreCreateBinary();
    read_LDR_Signal = xSemaphoreCreateBinary();
    x_Sem_C_Greenhouse = xSemaphoreCreateCounting(2, 0);
}
//Loop task functions
static void control_greenhouse(void *args){
    uint8_t motor_state;
    for(;;){
        ESP_LOGI(task_logging,"Task running: %s", "control_greenhouse");
        //Trigger read tasks
        xSemaphoreGive(read_DHT_Signal);
        xSemaphoreGive(read_LDR_Signal);
        //Wait for reads to finish 
        for(int i = 0; i < 2; i++){
            xSemaphoreTake(x_Sem_C_Greenhouse, portMAX_DELAY);
        }
        xQueueReset(x_Sem_C_Greenhouse); //Hack->Use reset queue for counting semaphore
        //Control Algorithm
        if(sensor_data.temperature > control_data.temperature_max){
            motor_state = 1;
        }
        else if(sensor_data.temperature < control_data.temperature_min){
            motor_state = 0;
        }
        //Trigger publish tasks on WiFi Core
        xSemaphoreGive(publish_DHT_Signal);
        xSemaphoreGive(publish_LDR_Signal);
        xSemaphoreGive(publish_WindowState_Signal);
        
        vTaskDelay(1000 / portTICK_RATE_MS);
    }  
}

void app_main(){
    //Startup info
    ESP_LOGI(startup_tag, "[APP] Startup..");
    ESP_LOGI(startup_tag, "[APP] IDF version: %s", esp_get_idf_version());
    ESP_LOGI(memory_tag, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    
    //Init
    initialize_wifi_sta_mode();
    initialize_ports();
    initialize_mqtt_app();
    init_sync_variables();

    //Application Tasks  
    xTaskCreatePinnedToCore(write_motor_state, "update_motor_status", TASK_STACK_MIN_SIZE, NULL, 11, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(control_greenhouse, "control_greenhouse", TASK_STACK_MIN_SIZE, NULL, 10, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(read_buttons, "read_buttons", TASK_STACK_MIN_SIZE, NULL, 6, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(read_DHT, "read_DHT", TASK_STACK_MIN_SIZE, NULL, 5, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(read_ldr, "read_ldr", TASK_STACK_MIN_SIZE, NULL, 5, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(write_display, "write_display", TASK_STACK_MIN_SIZE, NULL, 4, NULL, APPLICATION_CORE);

    //MQTT Tasks
    xTaskCreatePinnedToCore(publish_dht_handler, "publish_dht_handler", TASK_STACK_MIN_SIZE, NULL, 1, NULL, WIFI_COMMUNICATIONS_CORE);
    xTaskCreatePinnedToCore(publish_ldr_handler, "publish_ldr_handler", TASK_STACK_MIN_SIZE, NULL, 1, NULL, WIFI_COMMUNICATIONS_CORE);
    xTaskCreatePinnedToCore(publish_window_state_handler, "publish_window_state_handler", TASK_STACK_MIN_SIZE, NULL, 1, NULL, WIFI_COMMUNICATIONS_CORE);
}