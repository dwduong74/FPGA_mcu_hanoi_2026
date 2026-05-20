#include "timer0.h"

#define T0_RELOAD  (65536 - 1000)   /* 1ms at 1MHz clock */

bit timer_1ms_flag = 0;

/*-----------------------------------------------------------
 * Timer0 ISR - fires every 1ms
 *-----------------------------------------------------------*/
void timer0_isr(void) interrupt ISRTimer0
{
    TH0 = T0_RELOAD >> 8;
    TL0 = (uint8_t)T0_RELOAD;
    timer_1ms_flag = 1;
}

/*-----------------------------------------------------------
 * timer0_init - configure Timer0 for 1ms interrupt
 *-----------------------------------------------------------*/
void timer0_init(void)
{
    TMOD  = 0x05;                   /* Timer0 Mode 1 (16-bit) */
    TCON0 = 0x20;                   /* Clock: fexto/32 = 1MHz  */
    TH0   = T0_RELOAD >> 8;
    TL0   = (uint8_t)T0_RELOAD;
    TR0   = 1;                      /* Start Timer0            */
    ET0   = 1;                      /* Enable Timer0 interrupt */
    EAL   = 1;                      /* Enable global interrupt */
}
