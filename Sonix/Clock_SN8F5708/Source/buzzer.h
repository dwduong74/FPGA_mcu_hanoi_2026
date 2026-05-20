#ifndef _BUZZER_H_
#define _BUZZER_H_
#include "mcu.h"

/*
 * Buzzer - Timer2 PWM output (T2PWM = P1.0)
 *   Timer2 clock = fexto/32 = 1MHz
 *   Tone frequency = 1MHz / (2 * (65536 - reload))
 */

void buzzer_init(void);
void buzzer_on(void);
void buzzer_off(void);

#endif
