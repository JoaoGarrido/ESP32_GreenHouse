#ifndef DISPLAY
#define DISPLAY

/*System libraries*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/*Project libraries*/
#include "greenhouse_system.h"
#include "u8g2_esp32_hal.h"
#include "../u8g2/csrc/u8g2.h"

/*Definitions*/
enum All_Menu{
    MAIN_MENU, 
    DATA_MENU, CONTROL_MENU, SETTINGS_MENU, 
    DATA_TEMP_MENU, DATA_HUMID_MENU, DATA_LUMI_MENU, DATA_OTHER_MENU, 
    CONTROL_TEMP_MAX_MENU, CONTROL_TEMP_MIN_MENU, 
    SETTINGS_MODE_MENU
};

enum selected_mode_type{Auto_Mode, Manual_Mode};

typedef struct {
    uint8_t index;
    uint8_t size;
    uint8_t current_mode;
} Mode_menu;

typedef struct {
    uint8_t index;
    uint8_t size;    
    Mode_menu mode_menu;
} Settings_menu;

//Data_menu
typedef struct {
    uint8_t name;
} Other_menu;

typedef struct {  
    float current;
    float daily_max;
    float daily_min;
    float week_max;
    float week_min;
} Sensor_data_menu;

typedef struct {
    uint8_t index;
    uint8_t size;    
    Sensor_data_menu temp_menu;
    Sensor_data_menu humid_menu;
    Sensor_data_menu lumi_menu;
    Other_menu other_menu;
} Data_menu;

//Control_menu
typedef struct {
    float temperature;
} Temp_limit_menu;

typedef struct {
    uint8_t index;
    uint8_t size;
    Temp_limit_menu temp_max_menu;
    Temp_limit_menu temp_min_menu;
    uint8_t current_window_state;
} Control_menu;

//Main_menu
typedef struct _Main_menu{
    uint8_t index;
    uint8_t size;
    Data_menu data_menu;
    Control_menu control_menu;
    Settings_menu settings_menu;
} Main_menu;

typedef struct _UI{
    Main_menu main_menu;
    uint8_t current_menu;
} UI;

/*Functions*/
void init_display(gpio_num_t PIN_SDA, gpio_num_t PIN_SCL);
void update_display(void *args);

#endif /* DISPLAY */
