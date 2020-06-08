#ifndef DEBUGGING
#define DEBUGGING

#define CONFIG_USE_DEBUG 0

#if CONFIG_USE_DEBUG
#define DEBUG_GPIO(GPIO_PIN, CODE) \
    do{ \
        gpio_set_level(GPIO_PIN, 1);\
        CODE\
        gpio_set_level(GPIO_PIN, 0);\
    }while(0); 
#else
#define DEBUG_GPIO(GPIO_PIN, CODE)\
    do{ \
        CODE\
    }while(0);
#endif

enum ENUM_DEBUG_GPIO{GPIO_CHANNEL_0=14, GPIO_CHANNEL_1=12, GPIO_CHANNEL_2=13, GPIO_CHANNEL_3=2, GPIO_CHANNEL_4=15 , GPIO_CHANNEL_5=4};

#endif