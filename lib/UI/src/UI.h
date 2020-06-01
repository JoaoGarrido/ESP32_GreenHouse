#ifndef UI
#define UI

#include "IO.h"

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

void button_handler(void *args);

#endif /* UI */
