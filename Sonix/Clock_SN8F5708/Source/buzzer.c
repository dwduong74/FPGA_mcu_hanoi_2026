#include "buzzer.h"

/*
 * Timer2 reload table - 10 frequency levels
 * freq = 1MHz / (2 * (65536 - reload))
 * Level 5 (reload=64701) -> 1MHz / (2*835) = ~599Hz (suitable for beep)
 */
code uint16_t musical_tab[10] = {
    63866,  /* Level 0: ~299 Hz */
    64033,  /* Level 1: ~350 Hz */
    64200,  /* Level 2: ~416 Hz */
    64367,  /* Level 3: ~499 Hz */
    64534,  /* Level 4: ~617 Hz */
    64701,  /* Level 5: ~599 Hz  <- default pip frequency */
    64848,  /* Level 6: ~1000 Hz */
    65035,  /* Level 7: ~1587 Hz */
    65202,  /* Level 8: ~2500 Hz */
    65369   /* Level 9: ~5000 Hz */
};

static uint16_t t2_reload;

/*-----------------------------------------------------------
 * buzzer_init - configure Timer2 for PWM, buzzer starts OFF
 *-----------------------------------------------------------*/
void buzzer_init(void)
{
    t2_reload = musical_tab[5];   /* default: level 5 ~600Hz */

    T2CON = 0x41;                  /* configure Timer2 */
    CCEN  = 0x00;                  /* compare output DISABLED (buzzer off) */

    TH2   = t2_reload >> 8;
    TL2   = (uint8_t)t2_reload;

    /* Set CRC to midpoint for 50% duty cycle */
    CRCH  = (uint8_t)((t2_reload + (65536 - t2_reload) / 2) >> 8);
    CRCL  = (uint8_t) (t2_reload + (65536 - t2_reload) / 2);

    ET2   = 1;                     /* Enable Timer2 interrupt */
}

/*-----------------------------------------------------------
 * buzzer_on - enable buzzer (PWM output active)
 *-----------------------------------------------------------*/
void buzzer_on(void)
{
    TH2  = t2_reload >> 8;         /* Reset Timer2 counter */
    TL2  = (uint8_t)t2_reload;
    CCEN |= 0x02;                  /* Enable compare output -> PWM tone */
}

/*-----------------------------------------------------------
 * buzzer_off - disable buzzer (pin goes quiet)
 *-----------------------------------------------------------*/
void buzzer_off(void)
{
    CCEN &= ~0x02;                 /* Disable compare output */
    /* Piezo buzzer stops when PWM stops */
}

/*-----------------------------------------------------------
 * Timer2 ISR - reload timer counter every period
 *-----------------------------------------------------------*/
void timer2_isr(void) interrupt ISRTimer2
{
    TH2 = t2_reload >> 8;
    TL2 = (uint8_t)t2_reload;
    TF2 = 0;                       /* Clear Timer2 overflow flag */
}
