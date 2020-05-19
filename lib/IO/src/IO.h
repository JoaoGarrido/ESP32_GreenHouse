#ifndef IO
#define IO

//System libraries
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
//Project libraries
#include "dht.h"
#include "logging.h"
#include "greenhouse_system.h"

void initialize_ports();
void update_motor_status(void *args);
void write_display(void *args);
void read_DHT22(void *args);
void read_ldr(void *args);

#endif /* IO */
