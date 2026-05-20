#include "segment.h"

/* --- Pin definitions --- */
#define SEG_COM0   P50
#define SEG_COM1   P51
#define SEG_COM2   P52
#define SEG_COM3   P53

/* Segment encoding: bit0=a, bit1=b, ..., bit6=g, bit7=DP */
code uint8_t seg_tab[10] = {
    0x3F,  /* 0: a b c d e f   */
    0x06,  /* 1:   b c         */
    0x5B,  /* 2: a b   d e   g */
    0x4F,  /* 3: a b c d     g */
    0x66,  /* 4:   b c     f g */
    0x6D,  /* 5: a   c d   f g */
    0x7D,  /* 6: a   c d e f g */
    0x07,  /* 7: a b c         */
    0x7F,  /* 8: a b c d e f g */
    0x6F   /* 9: a b c d   f g */
};

uint8_t led_buff[4] = {0, 0, 0, 0};
static uint8_t com_cnt = 0;

/*-----------------------------------------------------------
 * segment_config - set GPIO modes for 7-SEG pins
 *-----------------------------------------------------------*/
void segment_config(void)
{
    P3  = 0x00;
    P3M |= 0xFF;   /* P3.0-P3.7: push-pull output */
    P5M &= 0xF0;
    P5M |= 0x0F;   /* P5.0-P5.3: push-pull output (COM0-COM3) */
}

/*-----------------------------------------------------------
 * segment_scan - multiplex 4 digits; call every 1ms
 *-----------------------------------------------------------*/
void segment_scan(void)
{
    /* Blank all segments before switching COM to avoid ghosting */
    P3 = 0x00;
    SEG_COM0 = 0;
    SEG_COM1 = 0;
    SEG_COM2 = 0;
    SEG_COM3 = 0;

    /* Activate current COM digit */
    switch(com_cnt)
    {
        case 0: SEG_COM0 = 1; break;
        case 1: SEG_COM1 = 1; break;
        case 2: SEG_COM2 = 1; break;
        case 3: SEG_COM3 = 1; break;
    }

    /* Output segment data for current digit */
    P3 = led_buff[com_cnt];

    if(++com_cnt > 3) com_cnt = 0;
}

/*-----------------------------------------------------------
 * segment_update - refresh led_buff with clock time
 *   show_hh: 1 = display HH, 0 = blank HH
 *   show_mm: 1 = display MM, 0 = blank MM
 *-----------------------------------------------------------*/
void segment_update(uint8_t hour, uint8_t min, bit show_hh, bit show_mm)
{
    /* HH digits */
    led_buff[0] = show_hh ? seg_tab[hour / 10]       : 0x00;
    led_buff[1] = show_hh ? (seg_tab[hour % 10] | 0x80) : 0x80; /* keep DP */

    /* MM digits */
    led_buff[2] = show_mm ? seg_tab[min / 10]  : 0x00;
    led_buff[3] = show_mm ? seg_tab[min % 10]  : 0x00;
}
