#ifndef GREENHOUSE_SYSTEM
#define GREENHOUSE_SYSTEM

#include <stdint.h>
#include "debugging.h"

//OS
#define WIFI_COMMUNICATIONS_CORE 0
#define APPLICATION_CORE 1
#define TASK_STACK_MIN_SIZE 10000

//Timer
#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds

typedef enum _Buttons{NONE, BTN_SELECT=0, BTN_BACK=1, BTN_UP=2, BTN_DOWN=3} Buttons;

typedef enum _Window_state{Window_state_Open, Window_state_Closed} Window_state;
typedef struct {
    float humidity;
    float temperature;
    uint32_t luminosity;
    int window_state;
} sensor_data_t;

typedef enum _Window_action{Window_action_Open, Window_action_Close} Window_action;
typedef enum _Mode{Mode_Auto, Mode_Manual} Mode;
typedef struct {
    float temperature_max;
    float temperature_min;
    int window_action;
    int mode;
} control_data_t;

#endif /* GREENHOUSE_SYSTEM */
