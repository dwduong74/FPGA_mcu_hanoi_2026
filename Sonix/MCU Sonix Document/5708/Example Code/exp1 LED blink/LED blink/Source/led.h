#ifndef _LED_
#define _LED_
#include "mcu.h"

#define     LED0      0
#define     LED1      1

#define     LED0_PIN      P03
#define     LED1_PIN      P04

void led_config(void);
void led_ctr(uint8_t led,uint8_t cmd);

#endif 