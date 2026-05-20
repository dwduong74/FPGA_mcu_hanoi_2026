#ifndef _KEYSCAN_H_
#define _KEYSCAN_H_
#include "mcu.h"

/*
 * Key matrix - SN8F5708 EVK
 *   Rows (output):  P4.4-P4.7  (keyout0-keyout3)
 *   Cols (input) :  P2.4-P2.7  (keyin0-keyin3)
 *
 * Physical layout and key_value mapping:
 *
 *          P27(Col0) P26(Col1) P25(Col2) P24(Col3)
 *  P44(R0):  SW1=1    SW5=5    SW9=9    SW13=13
 *  P45(R1):  SW2=2    SW6=6    SW10=10  SW14=14
 *  P46(R2):  SW3=3    SW7=7    SW11=11  SW15=15
 *  P47(R3):  SW4=4    SW8=8    SW12=12  SW16=16
 *
 * Buttons used in this project:
 *   SW3  (key_value 3)  = SETUP   - cycle through set modes
 *   SW6  (key_value 6)  = PLUS(+) - increment value
 *   SW10 (key_value 10) = MINUS(-) - decrement value
 *   SW16 (key_value 16) = ALARM   - set alarm mode
 */

extern uint8_t g_key_event;  /* 0 = no event; 1-16 = key pressed */

void keyscan_init(void);

#endif
