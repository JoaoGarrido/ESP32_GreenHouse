#ifndef IO
#define IO

//System libraries
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
//Project libraries
#include "dht.h"
#include "logging.h"
#include "greenhouse_system.h"

void initialize_ports();
void write_motor_state(void *args);
void write_display(void *args);
void read_DHT(void *args);
void read_ldr(void *args);
void read_buttons(void *args);

#endif /* IO */
