#ifndef TAGS
#define TAGS

//OS
#define WIFI_COMMUNICATIONS_CORE 0
#define APPLICATION_CORE 1
#define TASK_STACK_MIN_SIZE 10000

//Logging
#define startup_tag "[Startup]"
#define memory_tag "[Memory]"
#define ldr_tag "[LDR]"
#define wifi_tag "[Wifi]"
#define mqtt_tag "[MQTT]"
#define dht22_tag "[DHT22]"
#define task_logging "[Task_logging]"
#define watchdog "[Watchdog]"


//Watchdog timers and macro
#define TWDT_TIMEOUT_S          3
#define TASK_RESET_PERIOD_S     2
#define CHECK_ERROR_CODE(returned, expected) ({                        \
            if(returned != expected){                                  \
                ESP_LOGI(watchdog,"Watchdog timer ERROR");             \
                abort();                                               \
            }                                                          \
})

#endif /* TAGS */