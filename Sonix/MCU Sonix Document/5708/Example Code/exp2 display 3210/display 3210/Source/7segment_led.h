#ifndef _7segment_
#define _7segment_
#include "mcu.h"

#define     SEG_A       P30
#define     SEG_B       P31
#define     SEG_C       P32
#define     SEG_D       P33
#define     SEG_E       P34
#define     SEG_F       P35
#define     SEG_G       P36
#define     SEG_P       P37

#define     SEG_COM0    P50
#define     SEG_COM1    P51
#define     SEG_COM2    P52
#define     SEG_COM3    P53

void segment_display(uint16_t tempdata);
void segment_led_config(void);
void segment_led_scan(void);
extern uint16_t sys_count ;
void segment_displayhex(uint16_t tempdata);
#endif