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
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_list[1], SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_list[2], SSD_COLOR_WHITE);
}

//Data menu
static void show_data_menu(){
    char Text_list[4][30] = {" Temperature", " Humidity", " Luminosity", " Other"};
    Text_list[UserInterface.main_menu.data_menu.index][0] = '*';
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_list[0], SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_list[1], SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_list[2], SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_list[3], SSD_COLOR_WHITE);
}

static void show_data_temp(){
    char Text_Temperature_Menu[30] = "Temperature Menu";
    //Current value
    char Text_Temp_Current[30];
    sprintf(Text_Temp_Current, "Current: %.2f", UserInterface.main_menu.data_menu.temp_menu.current);
    //Daily max value
    char Text_Temp_Daily[30];
    sprintf(Text_Temp_Daily, "Daily-> Max: %.2f Min: %.2f", UserInterface.main_menu.data_menu.temp_menu.daily_max, UserInterface.main_menu.data_menu.temp_menu.daily_min);
    //Week max value
    char Text_Temp_Week[30];
    sprintf(Text_Temp_Week, "Week-> Max: %.2f Min: %.2f", UserInterface.main_menu.data_menu.temp_menu.week_max, UserInterface.main_menu.data_menu.temp_menu.week_min);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_Temperature_Menu, SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_NorthEast, Text_Temp_Current, SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_Center, Text_Temp_Daily, SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_SouthEast, Text_Temp_Week, SSD_COLOR_WHITE);
}

static void show_data_humid(){
    char Text_Humidity_Menu[30] = "Humidity Menu";
    //Current value
    char Text_Humid_Current[30];
    sprintf(Text_Humid_Current, "Current: %.2f", UserInterface.main_menu.data_menu.humid_menu.current);
    //Daily max value
    char Text_Humid_Daily[30];
    sprintf(Text_Humid_Daily, "Daily-> Max: %.2f Min: %.2f", UserInterface.main_menu.data_menu.humid_menu.daily_max, UserInterface.main_menu.data_menu.humid_menu.daily_min);
    //Week max value
    char Text_Humid_Week[30];
    sprintf(Text_Humid_Week, "Week-> Max: %.2f Min: %.2f", UserInterface.main_menu.data_menu.humid_menu.week_max, UserInterface.main_menu.data_menu.humid_menu.week_min);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_Humidity_Menu, SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_NorthEast, Text_Humid_Current, SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_Center, Text_Humid_Daily, SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_SouthEast, Text_Humid_Week, SSD_COLOR_WHITE);
}

static void show_data_lumi(){
    char Text_Luminosity_Menu[30] = "Luminosity Menu";
    //Current value
    char Text_Lumi_Current[30];
    sprintf(Text_Lumi_Current, "Current: %.2f", UserInterface.main_menu.data_menu.lumi_menu.current);
    //Daily max value
    char Text_Lumi_Daily[30];
    sprintf(Text_Lumi_Daily, "Daily-> Max: %.2f Min: %.2f", UserInterface.main_menu.data_menu.lumi_menu.daily_max, UserInterface.main_menu.data_menu.lumi_menu.daily_min);
    //Week max value
    char Text_Lumi_Week[30];
    sprintf(Text_Lumi_Week, "Week-> Max: %.2f Min: %.2f", UserInterface.main_menu.data_menu.lumi_menu.week_max, UserInterface.main_menu.data_menu.lumi_menu.week_min);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_Luminosity_Menu, SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_NorthEast, Text_Lumi_Current, SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_Center, Text_Lumi_Daily, SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_SouthEast, Text_Lumi_Week, SSD_COLOR_WHITE);
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

static void show_control_temp_max(){
    char Text_Temp_max_limit[30] = "Max Temperature Limit";
    char Text_Temp[10];
    sprintf(Text_Temp, "%.2f", UserInterface.main_menu.control_menu.temp_max_menu.temperature);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_Temp_max_limit, SSD_COLOR_WHITE);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_Center, Text_Temp, SSD_COLOR_WHITE);
}

static void show_control_temp_min(){
    char Text_Temp_min_limit[30] = "Min Temperature Limit";
    char Text_Temp[10];
    sprintf(Text_Temp, "%.2f", UserInterface.main_menu.control_menu.temp_min_menu.temperature);
    SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_North, Text_Temp_min_limit, SSD_COLOR_WHITE);
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
            show_data_temp();
            break;
        case DATA_HUMID_MENU:
            show_data_humid();
            break;
        case DATA_LUMI_MENU:
            show_data_lumi();
            break;
        case DATA_OTHER_MENU:
            show_data_other();
            break;
        case CONTROL_TEMP_MAX_MENU:
            show_control_temp_max();
            break;
        case CONTROL_TEMP_MIN_MENU:
            show_control_temp_min();
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
    SSD1306_SetFont( &I2CDisplay, &Font_liberation_mono_17x30);
}

void update_display(void* args){
    for(;;){
        static int previous_menu = 0;
        refresh_data();
        if(UserInterface.current_menu != previous_menu){
            SSD1306_Clear( &I2CDisplay, SSD_COLOR_BLACK);
            update_menu();   
        }
        previous_menu = UserInterface.current_menu;
        SSD1306_Update( &I2CDisplay);
        vTaskDelay(10);
    }
}
