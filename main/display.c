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
#if SSD1306
    u8g2_t u8g2;
    #define NORTH 2,17
    #define CENTER 2, 32
    #define SOUTH 2, 63
    #define SOUTH_EAST 80, 63
#elif LCD16x02

#endif

/**Private Functions**/

//Main menu
static void show_main_menu(){
    char Text_list[3][30] = {" Data", " Control", " Settings"};
    char Text_position[10];
    sprintf( Text_position, "[%d/%d]", UserInterface.main_menu.index+1, UserInterface.main_menu.size);

#if SSD1306
    u8g2_DrawStr( &u8g2, CENTER, Text_list[UserInterface.main_menu.index]);
    u8g2_DrawStr( &u8g2, SOUTH_EAST, Text_position);
#elif LCD16x02

#endif
}

//Data menu
static void show_data_menu(){
    char Text_list[4][30] = {" Temperature", " Humidity", " Luminosity", " Other"};
    char Text_position[10];
    sprintf( Text_position, "[%d/%d]", UserInterface.main_menu.data_menu.index+1, UserInterface.main_menu.data_menu.size);
    //Text_list[UserInterface.main_menu.data_menu.index][0] = '*';
#if SSD1306
    u8g2_DrawStr( &u8g2, CENTER, Text_list[UserInterface.main_menu.data_menu.index]);
    u8g2_DrawStr( &u8g2, SOUTH_EAST, Text_position);
#elif LCD16x02

#endif
}

static void show_data(Sensor_data_menu sensor_data){
    //Current value
    char Text_Current[30];
    sprintf(Text_Current, "Curr: %.2f", sensor_data.current);
    //Daily max value
    char Text_Daily[30];
    sprintf(Text_Daily, "Day|Max: %.2f\nMin: %.2f", sensor_data.daily_max, sensor_data.daily_min);
    //Week max value
    char Text_Week[30];
    sprintf(Text_Week, "Week|Max: %.2f\nMin: %.2f", sensor_data.week_max, sensor_data.week_min);
#if SSD1306    
    u8g2_DrawStr( &u8g2, NORTH, Text_Current);
    u8g2_DrawStr( &u8g2, CENTER, Text_Daily);
    u8g2_DrawStr( &u8g2, SOUTH, Text_Week);
#elif LCD16x02

#endif
}

static void show_data_other(){

}

//Control menu
static void show_control_menu(){
    if(UserInterface.main_menu.settings_menu.mode_menu.current_mode == Auto_Mode){
        char Text_list[2][30] = {" Max Temp Limit", " Min Temp Limit"};
        char Text_position[10];
        sprintf( Text_position, "[%d/%d]", UserInterface.main_menu.control_menu.index+1, UserInterface.main_menu.control_menu.size);
#if SSD1306    
        u8g2_DrawStr( &u8g2, CENTER, Text_list[UserInterface.main_menu.control_menu.index]);
        u8g2_DrawStr( &u8g2, SOUTH_EAST, Text_position);
#elif LCD16x02

#endif
    }
    else{
        char Text_list[2][30] = {" Open", " Close"};
        Text_list[UserInterface.main_menu.control_menu.current_window_state][0] = '*';
        char Text_position[10];
        sprintf( Text_position, "[%d/%d]", UserInterface.main_menu.control_menu.index+1, UserInterface.main_menu.control_menu.size);
#if SSD1306    
        u8g2_DrawStr( &u8g2, CENTER, Text_list[UserInterface.main_menu.control_menu.index]);
        u8g2_DrawStr( &u8g2, SOUTH_EAST, Text_position);
#elif LCD16x02

#endif    
    }
}

static void show_control_temp_limit(const char temp_limit_type[5], Temp_limit_menu temp_limit_menu){
    char Text_limit[30] = "    Temp";
    for(int i = 0; i < 3; i++){
        Text_limit[i] = temp_limit_type[i];
    }
    char Text_Temp[10];
    sprintf(Text_Temp, "%.2f", temp_limit_menu.temperature);
#if SSD1306    
    u8g2_DrawStr( &u8g2, NORTH, Text_limit);
    u8g2_DrawStr( &u8g2, CENTER, Text_Temp);
#elif LCD16x02

#endif
}

//Settings menu
static void show_settings_menu(){
    char Text_list[1][30] = {" Mode"};
    char Text_position[10];
    sprintf( Text_position, "[%d/%d]", UserInterface.main_menu.settings_menu.index+1, UserInterface.main_menu.settings_menu.size);
#if SSD1306
    u8g2_DrawStr( &u8g2, CENTER, Text_list[UserInterface.main_menu.settings_menu.index]);
    u8g2_DrawStr( &u8g2, SOUTH_EAST, Text_position);
#elif LCD16x02

#endif
}

static void show_settings_mode(){
    char Text_list[2][30] = {" Auto Mode", " Manual Mode"};
    Text_list[UserInterface.main_menu.settings_menu.mode_menu.current_mode][0] = '*';
    char Text_position[10];
    sprintf( Text_position, "[%d/%d]", UserInterface.main_menu.settings_menu.mode_menu.index+1, UserInterface.main_menu.settings_menu.mode_menu.size);
#if SSD1306    
    u8g2_DrawStr( &u8g2, CENTER, Text_list[UserInterface.main_menu.settings_menu.mode_menu.index]);
    u8g2_DrawStr( &u8g2, SOUTH_EAST, Text_position);
#elif LCD16x02

#endif
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

static void init_SSD1306(gpio_num_t PIN_SDA, gpio_num_t PIN_SCL){
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.sda   = PIN_SDA;
	u8g2_esp32_hal.scl  = PIN_SCL;
	u8g2_esp32_hal_init(u8g2_esp32_hal);
	//init u8g2
    ets_printf("Starting I2C setup\n");
    u8g2_Setup_ssd1306_i2c_128x64_noname_f( &u8g2, U8G2_R0, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);
	u8x8_SetI2CAddress( &u8g2.u8x8, 0x78);
    u8g2_InitDisplay(&u8g2);
    ets_printf("Finished I2C setup\n");
    u8g2_SetPowerSave(&u8g2, 0);
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
    ets_printf("Finished display init\n");
}

static void init_LCD16x02(gpio_num_t PIN_SDA, gpio_num_t PIN_SCL){
    // Set up I2C
    i2c_master_init();
    i2c_port_t i2c_num = I2C_MASTER_NUM;
    uint8_t address = CONFIG_LCD1602_I2C_ADDRESS;

    // Set up the SMBus
    smbus_info_t * smbus_info = smbus_malloc();
    ESP_ERROR_CHECK(smbus_init(smbus_info, i2c_num, address));
    ESP_ERROR_CHECK(smbus_set_timeout(smbus_info, 1000 / portTICK_RATE_MS));
    
    // Set up the LCD1602 device with backlight off
    i2c_lcd1602_info_t * lcd_info = i2c_lcd1602_malloc();
    ESP_ERROR_CHECK(i2c_lcd1602_init(lcd_info, smbus_info, true,
                                     LCD_NUM_ROWS, LCD_NUM_COLUMNS, LCD_NUM_VISIBLE_COLUMNS));
    i2c_lcd1602_set_backlight(lcd_info, false);

    ESP_ERROR_CHECK(i2c_lcd1602_reset(lcd_info));
} 

/**Public Functions**/
void init_display(gpio_num_t PIN_SDA, gpio_num_t PIN_SCL){
#if SSD1306
    init_SSD1306(PIN_SDA, PIN_SCL);
#elif LCD16x02
    init_LCD16x02(PIN_SDA, PIN_SCL);
#endif
}

void update_display(void* args){
    for(;;){
        DEBUG_GPIO(GPIO_CHANNEL_0,
            refresh_data();
#if SSD1306
            u8g2_ClearBuffer(&u8g2);
#elif LCD16x02
#endif
            update_menu();
#if SSD1306
            u8g2_SendBuffer(&u8g2);
#elif LCD16x02
#endif
        );
        vTaskDelay(1/portTICK_RATE_MS);
    }
}