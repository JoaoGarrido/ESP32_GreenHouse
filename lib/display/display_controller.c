#include "display_controller.h"

/**Public Variables**/
extern UI UserInterface;
extern control_data_t control_data;
extern sensor_data_t sensor_data;

/**Private Functions**/
static void main_menu_handler(uint32_t current_button){
    switch(current_button){
        case BTN_UP:
            UserInterface.main_menu.index = (UserInterface.main_menu.index + UserInterface.main_menu.size - 1)%UserInterface.main_menu.size;
            break;
        case BTN_DOWN:
            UserInterface.main_menu.index = (UserInterface.main_menu.index + 1)%UserInterface.main_menu.size;
            break;
        case BTN_SELECT:
            if(UserInterface.main_menu.index == 0){
                UserInterface.current_menu = DATA_MENU; 
            }
            else if(UserInterface.main_menu.index == 1){
                UserInterface.current_menu = CONTROL_MENU;
            }
            else if(UserInterface.main_menu.index == 2){
                UserInterface.current_menu = SETTINGS_MENU;
            }
            break;
        case BTN_BACK:
            break;
    }
}

static void data_menu_handler(uint32_t current_button){
    switch(current_button){
        case BTN_UP:
            UserInterface.main_menu.data_menu.index = (UserInterface.main_menu.data_menu.index + UserInterface.main_menu.data_menu.size - 1)%UserInterface.main_menu.data_menu.size;
            break;
        case BTN_DOWN:
            UserInterface.main_menu.data_menu.index = (UserInterface.main_menu.data_menu.index + 1)%UserInterface.main_menu.data_menu.size;
            break;
        case BTN_SELECT:
            if(UserInterface.main_menu.data_menu.index == 0){
                UserInterface.current_menu = DATA_TEMP_MENU;
                UserInterface.main_menu.data_menu.temp_menu.current = sensor_data.temperature;
            }
            else if(UserInterface.main_menu.data_menu.index == 1){
                UserInterface.current_menu = DATA_HUMID_MENU;
                UserInterface.main_menu.data_menu.humid_menu.current = sensor_data.humidity;
            }
            else if(UserInterface.main_menu.data_menu.index == 2){
                UserInterface.current_menu = DATA_LUMI_MENU;
                UserInterface.main_menu.data_menu.lumi_menu.current = sensor_data.luminosity;
            }
            else if(UserInterface.main_menu.data_menu.index == 3){
                UserInterface.current_menu = DATA_OTHER_MENU;
            }
            break;
        case BTN_BACK:
            UserInterface.current_menu = MAIN_MENU;
            break;
    } 
}

static void control_menu_handler(uint32_t current_button){
    switch(current_button){
        case BTN_UP:
            UserInterface.main_menu.control_menu.index = (UserInterface.main_menu.control_menu.index + UserInterface.main_menu.control_menu.size - 1)%UserInterface.main_menu.control_menu.size;
            break;
        case BTN_DOWN:
            UserInterface.main_menu.control_menu.index = (UserInterface.main_menu.control_menu.index + 1)%UserInterface.main_menu.control_menu.size;
            break;
        case BTN_SELECT:
            if(UserInterface.main_menu.settings_menu.mode_menu.current_mode == Auto_Mode){
                if(UserInterface.main_menu.control_menu.index == 0){
                    UserInterface.current_menu = CONTROL_TEMP_MAX_MENU; 
                    UserInterface.main_menu.control_menu.temp_max_menu.temperature = control_data.temperature_max;
                }
                else if(UserInterface.main_menu.control_menu.index == 1){
                    UserInterface.current_menu = CONTROL_TEMP_MIN_MENU;
                    UserInterface.main_menu.control_menu.temp_min_menu.temperature = control_data.temperature_min;
                }
            }
            else{
                if(UserInterface.main_menu.control_menu.index == 0){
                    control_data.window_action = Window_action_Open; 
                }
                else if(UserInterface.main_menu.control_menu.index == 1){
                    control_data.window_action = Window_action_Close; 
                }
            }
            break;
        case BTN_BACK:
            UserInterface.current_menu = MAIN_MENU;
            break;
    } 
}

static void settings_menu_handler(uint32_t current_button){
    switch(current_button){
        case BTN_UP:
            UserInterface.main_menu.settings_menu.index = (UserInterface.main_menu.settings_menu.index + UserInterface.main_menu.settings_menu.size - 1)%UserInterface.main_menu.settings_menu.size;
            break;
        case BTN_DOWN:
            UserInterface.main_menu.settings_menu.index = (UserInterface.main_menu.settings_menu.index + 1)%UserInterface.main_menu.settings_menu.size;
            break;
        case BTN_SELECT:
            if(UserInterface.main_menu.control_menu.index == 0){
                UserInterface.current_menu = SETTINGS_MODE_MENU; 
            }
            break;
        case BTN_BACK:
            UserInterface.current_menu = MAIN_MENU;
            break;
    } 
}

static void data_show_menu_handler(uint32_t current_button){
    switch(current_button){
        case BTN_BACK:
            UserInterface.current_menu = DATA_MENU;
            break;
        default:
            break;
    }
}

static void control_temp_menu_handler(uint32_t current_button){
    float *temp;
    if(UserInterface.current_menu == CONTROL_TEMP_MAX_MENU){
        temp = &(UserInterface.main_menu.control_menu.temp_max_menu.temperature);
    }
    else{
        temp = &(UserInterface.main_menu.control_menu.temp_min_menu.temperature);
    }
    switch(current_button){
        case BTN_UP:
            (*temp) = (*temp) + 1.0; 
            break;
        case BTN_DOWN:
            (*temp) = (*temp) - 1.0; 
            break;
        case BTN_SELECT:
            control_data.temperature_max = (*temp);
            break;
        case BTN_BACK:
            UserInterface.current_menu = CONTROL_MENU;
            break;
    } 
}

static void settings_mode_menu_handler(uint32_t current_button){
    switch(current_button){
        case BTN_UP:
            UserInterface.main_menu.settings_menu.mode_menu.index = (UserInterface.main_menu.settings_menu.mode_menu.index + UserInterface.main_menu.settings_menu.mode_menu.size - 1)%UserInterface.main_menu.settings_menu.mode_menu.size;
            break;
        case BTN_DOWN:
            UserInterface.main_menu.settings_menu.mode_menu.index = (UserInterface.main_menu.settings_menu.mode_menu.index + 1)%UserInterface.main_menu.settings_menu.mode_menu.size;
            break;
        case BTN_SELECT:
            UserInterface.main_menu.settings_menu.mode_menu.current_mode = UserInterface.main_menu.settings_menu.mode_menu.index;
            if(UserInterface.main_menu.settings_menu.mode_menu.index == Auto_Mode){
                control_data.mode = Mode_Auto; 
            }
            else{
                control_data.mode = Mode_Manual; 
            }
            break;
        case BTN_BACK:
            UserInterface.current_menu = SETTINGS_MENU;
            break;
    }
}

/**Public Functions**/
void refresh_gui(void *args){
    for(;;){

    }
}

void button_handler(void *args){
    uint32_t current_button = 0;
    for(;;){
        ESP_LOGI(buttons_tag, "Task running: %s", "button_handler blocked");
        xTaskNotifyWait(0x00, 0xffffffff, &current_button, portMAX_DELAY);
        ESP_LOGI(buttons_tag,"Task running: %s%d", "button_handler unblocked from button ", current_button);
        switch(UserInterface.current_menu){
            case MAIN_MENU:
                main_menu_handler(current_button);
                break;
            case DATA_MENU:
                data_menu_handler(current_button);
                break;
            case CONTROL_MENU:
                control_menu_handler(current_button);
                break;
            case SETTINGS_MENU:
                settings_menu_handler(current_button);
                break;
            case DATA_TEMP_MENU:
            case DATA_HUMID_MENU:
            case DATA_LUMI_MENU:
            case DATA_OTHER_MENU:
                data_show_menu_handler(current_button);
                break;
            case CONTROL_TEMP_MAX_MENU:
            case CONTROL_TEMP_MIN_MENU:
                control_temp_menu_handler(current_button);
                break;
            case SETTINGS_MODE_MENU:
                settings_mode_menu_handler(current_button);
                break;
            default:
                break;
        }
    }
}