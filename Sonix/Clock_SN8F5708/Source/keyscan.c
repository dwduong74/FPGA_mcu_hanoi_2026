#include "keyscan.h"

/* Row output pins */
#define ROW0  P44
#define ROW1  P45
#define ROW2  P46
#define ROW3  P47

/* Column input pins */
#define COL0  P27
#define COL1  P26
#define COL2  P25
#define COL3  P24

uint8_t g_key_event = 0;

/*-----------------------------------------------------------
 * keyscan_init - configure INT0 + row pin modes
 *   - All rows driven LOW so any key press pulls a column LOW
 *   - INT0 fires on falling edge of any column
 *-----------------------------------------------------------*/
void keyscan_init(void)
{
    P4  &= 0x0F;    /* Clear P4.4-P4.7 (rows = 0) */
    P4M |= 0xF0;    /* P4.4-P4.7 as push-pull output */

    PEDGE = 0x02;   /* INT0: falling edge trigger */
    EX0   = 1;      /* Enable External Interrupt 0 */
    EAL   = 1;      /* Enable global interrupt */
}

/*-----------------------------------------------------------
 * keyscan_isr - INT0 ISR, scans matrix to find pressed key
 *-----------------------------------------------------------*/
void keyscan_isr(void) interrupt ISRInt0
{
    uint8_t key = 0;
    uint8_t i;

    PEDGE = 0;      /* Disable edge detection during scan */

    /* --- Scan Row 0 (P44 low, P45/P46/P47 high) --- */
    P4 |= 0xF0;     /* All rows HIGH */
    ROW0 = 0;       /* Row0 LOW */
    for(i = 0; i < 8; i++);    /* Settle delay */
    if(COL0 == 0) key = 1;
    if(COL1 == 0) key = 5;
    if(COL2 == 0) key = 9;
    if(COL3 == 0) key = 13;

    /* --- Scan Row 1 (P45 low) --- */
    P4 |= 0xF0;
    ROW1 = 0;
    for(i = 0; i < 8; i++);
    if(COL0 == 0) key = 2;
    if(COL1 == 0) key = 6;
    if(COL2 == 0) key = 10;
    if(COL3 == 0) key = 14;

    /* --- Scan Row 2 (P46 low) --- */
    P4 |= 0xF0;
    ROW2 = 0;
    for(i = 0; i < 8; i++);
    if(COL0 == 0) key = 3;
    if(COL1 == 0) key = 7;
    if(COL2 == 0) key = 11;
    if(COL3 == 0) key = 15;

    /* --- Scan Row 3 (P47 low) --- */
    P4 |= 0xF0;
    ROW3 = 0;
    for(i = 0; i < 8; i++);
    if(COL0 == 0) key = 4;
    if(COL1 == 0) key = 8;
    if(COL2 == 0) key = 12;
    if(COL3 == 0) key = 16;

    P4 &= 0x0F;     /* Restore: all rows LOW (ready for next INT0) */

    if(key != 0)
        g_key_event = key;

    PEDGE = 0x02;   /* Re-enable falling edge detection */
}
