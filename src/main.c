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
//Semaphores
SemaphoreHandle_t DHT_Signal = NULL;
SemaphoreHandle_t LDR_Signal = NULL;
SemaphoreHandle_t Window_state_Signal = NULL;

//Init functions
void init_sync_variables(){
    DHT_Signal = xSemaphoreCreateBinary();
    LDR_Signal = xSemaphoreCreateBinary();
    Window_state_Signal = xSemaphoreCreateBinary();
}

//Loop task functions
static void control_greenhouse(void *args){
    for(;;){
        ESP_LOGI(task_logging,"Task running: %s", "control_greenhouse");
        
        vTaskDelay(1000 / portTICK_RATE_MS);
    }  
}

void app_main(){
    //Startup info
    ESP_LOGI(startup_tag, "[APP] Startup..");
    ESP_LOGI(startup_tag, "[APP] IDF version: %s", esp_get_idf_version());
    ESP_LOGI(memory_tag, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    //init
    initialize_wifi_sta_mode();
    initialize_ports();
    initialize_mqtt_app();
    init_sync_variables();

    //Application Tasks  
    xTaskCreatePinnedToCore(read_DHT22, "read_DHT22", TASK_STACK_MIN_SIZE, NULL, 5, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(update_motor_status, "update_motor_status", TASK_STACK_MIN_SIZE, NULL, 4, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(control_greenhouse, "control_greenhouse", TASK_STACK_MIN_SIZE, NULL, 3, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(write_display, "write_display", TASK_STACK_MIN_SIZE, NULL, 2, NULL, APPLICATION_CORE);
    xTaskCreatePinnedToCore(read_ldr, "read_ldr", TASK_STACK_MIN_SIZE, NULL, 6, NULL, APPLICATION_CORE);

    //MQTT Tasks
    xTaskCreatePinnedToCore(publish_dht_handler, "publish_dht_handler", TASK_STACK_MIN_SIZE, NULL, 2, NULL, WIFI_COMMUNICATIONS_CORE);
    xTaskCreatePinnedToCore(publish_ldr_handler, "publish_ldr_handler", TASK_STACK_MIN_SIZE, NULL, 2, NULL, WIFI_COMMUNICATIONS_CORE);
    xTaskCreatePinnedToCore(publish_window_state_handler, "publish_window_state_handler", TASK_STACK_MIN_SIZE, NULL, 2, NULL, WIFI_COMMUNICATIONS_CORE);

}