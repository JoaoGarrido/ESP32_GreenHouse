#include "UI.h"

/*Global variables*/
extern control_data_t control_data;
/*Static variables*/
static display_data_t display_data = {Menu_state_Data_menu, Main_menu_select_Data_menu, 8, SOMETHING, 30.0};

static void main_menu(uint32_t current_button){
    switch (current_button){
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

static void max_temperature_menu(uint32_t current_button){
    switch (current_button){
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

static void min_temperature_menu(uint32_t current_button){
    switch (current_button){
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

static void config_menu(uint32_t current_button){
    switch (current_button){
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

void button_handler(void *args){
    uint32_t current_button = 0;
    for(;;){
        ESP_LOGI(buttons_tag, "Task running: %s", "button_handler blocked");
        xTaskNotifyWait(0x00, 0xffffffff, &current_button, portMAX_DELAY);
        ESP_LOGI(buttons_tag,"Task running: %s%d", "button_handler unblocked from button ", current_button);
        switch(display_data.menu_state){
            case Menu_state_Data_menu:
                //Move from data menu to main menu after any button is pressed
                display_data.menu_state = Menu_state_Main_menu;
                break;
            case Menu_state_Main_menu:
                main_menu(current_button);
                break;
            case Menu_state_Max_temperature_menu:
                max_temperature_menu(current_button);
                break;
            case Menu_state_Min_temperature_menu:
                min_temperature_menu(current_button);
                break;
            case Menu_state_Config_menu:
                config_menu(current_button);
                break;
        }
    }  
}