#ifndef LOGGING
#define LOGGING

//System libraries
#include "esp_log.h"
#include "esp_task_wdt.h"

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

//Watchdog timers and macro
#define TWDT_TIMEOUT_S          3
#define TASK_RESET_PERIOD_S     2
#define CHECK_ERROR_CODE(returned, expected) ({                        \
            if(returned != expected){                                  \
                ESP_LOGI(watchdog,"Watchdog timer ERROR");             \
                abort();                                               \
            }                                                          \
})

#endif /* LOGGING */
