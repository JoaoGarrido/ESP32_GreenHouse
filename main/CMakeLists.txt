# Warning! This code was automatically generated for projects
# without default 'CMakeLists.txt' file.

set(app_sources "app_main.c" 
                "IO.c" 
                "display.c" 
                "display_controller.c"
                "communications.c"
)

idf_component_register(SRCS ${app_sources} INCLUDE_DIRS "include" REQUIRES esp_adc_cal mqtt nvs_flash PRIV_REQUIRES dht u8g2 u8g2_esp32_hal)