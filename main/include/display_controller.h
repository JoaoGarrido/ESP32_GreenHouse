#ifndef DISPLAY_CONTROLLER
#define DISPLAY_CONTROLLER

/*Project libraries*/
#include "display.h"
#include "greenhouse_system.h"

/*Functions*/
void init_gui();
void button_handler(void *args);
void refresh_gui(void *args);

#endif /* DISPLAY_CONTROLLER */

