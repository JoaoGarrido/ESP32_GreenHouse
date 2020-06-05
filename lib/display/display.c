#include "display.h"

extern void refresh_data();
/**Public Variables**/
UI UserInterface;
/** Basic UI Logic
Main_menu:
-Control
--Auto Mode*
---Temp Max Limit
---Temp Min Limit
--Manual Mode*
---Open
---Close
-Data
--Temp
--Humid
--Lumi
--Other
-Settings
--Mode
---Auto
---Manual
**/

/**Private Variables**/
static const int I2CDisplayAddress = 0x3C;
static const int I2CDisplayWidth = 128;
static const int I2CDisplayHeight = 32;
static const int I2CResetPin = -1;
static struct SSD1306_Device I2CDisplay;

/**Private Functions**/

//Main menu
static void show_main_menu(){
    char Text_list[3][30] = {" Data", " Control", " Settings"};
    Text_list[UserInterface.main_menu.index][0] = '*';
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_list[0], SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_Center, Text_list[1], SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_South, Text_list[2], SSD_COLOR_WHITE);
}

//Data menu
static void show_data_menu(){
    char Text_list[4][30] = {" Temperature", " Humidity", " Luminosity", " Other"};
    Text_list[UserInterface.main_menu.data_menu.index][0] = '*';
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_Center, Text_list[UserInterface.main_menu.data_menu.index], SSD_COLOR_WHITE);
}

static void show_data(Sensor_data_menu sensor_data){
    //Current value
    char Text_Current[30];
    sprintf(Text_Current, "Current: %.2f", UserInterface.main_menu.data_menu.temp_menu.current);
    //Daily max value
    char Text_Daily[30];
    sprintf(Text_Daily, "Day|Max: %.2f Min: %.2f", UserInterface.main_menu.data_menu.temp_menu.daily_max, UserInterface.main_menu.data_menu.temp_menu.daily_min);
    //Week max value
    char Text_Week[30];
    sprintf(Text_Week, "Week|Max: %.2f Min: %.2f", UserInterface.main_menu.data_menu.temp_menu.week_max, UserInterface.main_menu.data_menu.temp_menu.week_min);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_Current, SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_Center, Text_Daily, SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_South, Text_Week, SSD_COLOR_WHITE);
}

static void show_data_other(){

}

//Control menu
static void show_control_menu(){
    if(UserInterface.main_menu.settings_menu.mode_menu.current_mode == Auto_Mode){
        char Text_list[2][30] = {" Max Temperature Limit", " Min Temperature Limit"};
        Text_list[UserInterface.main_menu.control_menu.index][0] = '*';
        SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_list[0], SSD_COLOR_WHITE);
        SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_list[1], SSD_COLOR_WHITE);
    }
    else{
        char Text_list[2][30] = {" Open", " Close"};
        Text_list[UserInterface.main_menu.control_menu.index][0] = '*';
        SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_list[0], SSD_COLOR_WHITE);
        SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_list[1], SSD_COLOR_WHITE);
    }
}

static void show_control_temp_limit(const char temp_limit_type[5], Temp_limit_menu temp_limit_menu){
    char Text_limit[30] = "    Temperature";
    for(int i = 0; i < 3; i++){
        Text_limit[i] = temp_limit_type[i];
    }
    char Text_Temp[10];
    sprintf(Text_Temp, "%.2f", temp_limit_menu.temperature);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_limit, SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_Center, Text_Temp, SSD_COLOR_WHITE);
}

//Settings menu
static void show_settings_menu(){
    char Text_list[1][30] = {" Mode"};
    Text_list[UserInterface.main_menu.control_menu.index][0] = '*';
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_list[0], SSD_COLOR_WHITE);
}

static void show_settings_mode(){
    char Text_list[2][30] = {" Auto Mode", " Manual Mode"};
    Text_list[UserInterface.main_menu.control_menu.index][0] = '*';
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_list[0], SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_list[1], SSD_COLOR_WHITE);
}

//Update GUI state
static void update_menu(void){
    switch(UserInterface.current_menu){
        case MAIN_MENU:
            show_main_menu();
            break;
        case DATA_MENU:
            show_data_menu();
            break;
        case CONTROL_MENU:
            show_control_menu();
            break;
        case SETTINGS_MENU:
            show_settings_menu();
            break;
        case DATA_TEMP_MENU:
            show_data(UserInterface.main_menu.data_menu.temp_menu);
            break;
        case DATA_HUMID_MENU:
            show_data(UserInterface.main_menu.data_menu.humid_menu);
            break;
        case DATA_LUMI_MENU:
            show_data(UserInterface.main_menu.data_menu.lumi_menu);
            break;
        case DATA_OTHER_MENU:
            show_data_other();
            break;
        case CONTROL_TEMP_MAX_MENU:
            show_control_temp_limit("Max", UserInterface.main_menu.control_menu.temp_max_menu);
            break;
        case CONTROL_TEMP_MIN_MENU:
            show_control_temp_limit("Min", UserInterface.main_menu.control_menu.temp_min_menu);
            break;
        case SETTINGS_MODE_MENU:
            show_settings_mode();
            break;
        default:
            break;
    }
}

/**Public Functions**/

void init_display(void){
    assert( SSD1306_I2CMasterInitDefault( ) == true );
    assert( SSD1306_I2CMasterAttachDisplayDefault( &I2CDisplay, I2CDisplayWidth, I2CDisplayHeight, I2CDisplayAddress, I2CResetPin ) == true );
    SSD1306_Clear( &I2CDisplay, SSD_COLOR_BLACK);
    SSD1306_SetFont( &I2CDisplay, &Font_liberation_mono_9x15);
}

void update_display(void* args){
    for(;;){
        refresh_data();
        //if(UserInterface.current_menu != previous_menu){
        SSD1306_Clear( &I2CDisplay, SSD_COLOR_BLACK);
        update_menu();   
        //}
        SSD1306_Update( &I2CDisplay);
        vTaskDelay(10);
    }
}