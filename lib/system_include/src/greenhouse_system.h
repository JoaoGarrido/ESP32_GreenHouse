#ifndef GREENHOUSE_SYSTEM
#define GREENHOUSE_SYSTEM

#include <stdint.h>

typedef struct {
    float humidity;
    float temperature;
    uint32_t luminosity;
    uint8_t window_state;
} sensor_data_t;

typedef struct {
    float temperature_max;
    float temperature_min;
    uint8_t window_action;
} control_data_t;

#endif /* GREENHOUSE_SYSTEM */
