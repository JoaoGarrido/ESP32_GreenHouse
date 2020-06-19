#ifndef IO
#define IO

/*System libraries*/
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/timer.h"
#include "driver/periph_ctrl.h"
/*Project libraries*/
#include "greenhouse_system.h"
#include "dht.h"

/*Definitions*/
//Buttons
#define N_BUTTONS 4
//ADC
#define DEFAULT_VREF    1100
#define NO_OF_SAMPLES   64          //Multisampling

/*Functions*/
void initialize_ports();
void write_motor_state(void *args);
void write_display(void *args);
void read_DHT(void *args);
void read_ldr(void *args);
void IRAM_ATTR timer_button_isr(void *args);
void write_stats(void *args); 

#endif /* IO */
