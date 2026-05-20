#ifndef _SEGMENT_H_
#define _SEGMENT_H_
#include "mcu.h"

/*
 * 7-Segment display - SN8F5708 EVK
 *   Segments : P3.0-P3.7  (SEG_A..SEG_G, SEG_DP)
 *   COM0-COM3: P5.0-P5.3
 *
 * Display format: HH.MM  (4 digits)
 *   led_buff[0] = HH tens
 *   led_buff[1] = HH units  (bit7 = decimal point)
 *   led_buff[2] = MM tens
 *   led_buff[3] = MM units
 */

extern uint8_t led_buff[4];

void segment_config(void);
void segment_scan(void);
void segment_update(uint8_t hour, uint8_t min, bit show_hh, bit show_mm);

#endif
