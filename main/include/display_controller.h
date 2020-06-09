#ifndef DISPLAY_CONTROLLER
#define DISPLAY_CONTROLLER

#include "display.h"
#include "greenhouse_system.h"

void init_gui();
void button_handler(void *args);
void refresh_gui(void *args);

#endif /* DISPLAY_CONTROLLER */
