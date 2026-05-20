#ifndef _TIMER0_H_
#define _TIMER0_H_
#include "mcu.h"

/* Timer0 - 1ms tick
 * Clock source: fexto/32 = 32MHz/32 = 1MHz
 * Reload: 65536 - 1000 = 64536 → overflow every 1ms
 */
extern bit timer_1ms_flag;

void timer0_init(void);

#endif
