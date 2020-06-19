#ifndef DEBUGGING
#define DEBUGGING

/*System libraries*/
#include "esp_log.h"
#include "esp_task_wdt.h"

/*Definitions*/
//Debugging
enum debug{
    No_debug = 0,
    Logging_debug,
    GPIO_debug,
    All_debug
};

enum ENUM_DEBUG_GPIO{GPIO_CHANNEL_0=14, GPIO_CHANNEL_1=12, GPIO_CHANNEL_2=13, GPIO_CHANNEL_3=2, GPIO_CHANNEL_4=15 , GPIO_CHANNEL_5=4};

#if CONFIG_USE_DEBUG >= 2
#define DEBUG_GPIO(GPIO_PIN, CODE) \
    do{ \
        gpio_set_level(GPIO_PIN, 1);\
        CODE\
        gpio_set_level(GPIO_PIN, 0);\
    }while(0); 
#else
#define DEBUG_GPIO(GPIO_PIN, CODE)\
    do{ \
        CODE\
    }while(0);
#endif

//Logging
#define startup_tag "[Startup]"
#define memory_tag "[Memory]"
#define ldr_tag "[LDR]"
#define wifi_tag "[Wifi]"
#define mqtt_tag "[MQTT]"
#define dht_tag "[DHT]"
#define task_logging "[Task_logging]"
#define watchdog "[Watchdog]"
#define buttons_tag "[User Input]"
#define motor_tag "[Motor Output]"
#define display_tag "[Display]"
#define stats_tag "[Stats]"

//Watchdog timers and macro
#define TWDT_TIMEOUT_S          3
#define TASK_RESET_PERIOD_S     2
#define CHECK_ERROR_CODE(returned, expected) ({                        \
            if(returned != expected){                                  \
                ESP_LOGI(watchdog,"Watchdog timer ERROR");             \
                abort();                                               \
            }                                                          \
})

#endif