/*==========================================================================
 * DIGITAL CLOCK - SN8F5708 EVK
 *==========================================================================
 * Board  : SN8F5708_EVK
 * MCU    : SN8F5708 (8051 core, 32MHz/4 = 8MHz)
 * Display: 4-digit 7-SEG, format HH.MM
 *
 * CONTROLS:
 *   SW3  (key 3)  : SETUP   - cycle through set modes
 *   SW6  (key 6)  : PLUS(+) - increment hour/minute
 *   SW10 (key 10) : MINUS(-) - decrement hour/minute
 *   SW16 (key 16) : ALARM   - enter alarm set mode
 *
 * STATE MACHINE:
 *   NORMAL -> [SW3] -> SET_HOUR -> [SW3] -> SET_MIN -> [SW3] -> NORMAL
 *   NORMAL -> [SW16]-> SET_ALARM_HOUR -> [SW16] -> SET_ALARM_MIN
 *                                                       -> [SW16] -> NORMAL (save EEPROM)
 *   Any set mode: 30s no key press -> auto return NORMAL
 *
 * BUZZER:
 *   Key press : pip 0.3s
 *   Alarm hit : pip-pip 5s (0.5s ON / 0.5s OFF)
 *   Timeout   : pip 0.3s
 *
 * LED D4 (P0.3):
 *   Blinks 1s (0.5s ON/OFF) only in SET_ALARM_HOUR / SET_ALARM_MIN
 *   OFF otherwise
 *
 * EEPROM (I2C, 0xA0):
 *   Addr 0x00 = alarm hour
 *   Addr 0x01 = alarm minute
 *==========================================================================*/

#define __XRAM_SFR_H__
#include "mcu.h"
#include "timer0.h"
#include "segment.h"
#include "keyscan.h"
#include "buzzer.h"
#include "i2c.h"

/*--------------------------------------------------------------------------
 *  State machine definitions
 *--------------------------------------------------------------------------*/
#define STATE_NORMAL          0
#define STATE_SET_HOUR        1
#define STATE_SET_MIN         2
#define STATE_SET_ALARM_HOUR  3
#define STATE_SET_ALARM_MIN   4

/*--------------------------------------------------------------------------
 *  Key definitions  (SW number = key_value from INT0 scan)
 *--------------------------------------------------------------------------*/
#define KEY_SETUP    3    /* SW3  - enter/cycle set modes        */
#define KEY_PLUS     6    /* SW6  - increment value              */
#define KEY_MINUS   10    /* SW10 - decrement value              */
#define KEY_ALARM   16    /* SW16 - enter alarm set / save alarm */

/*--------------------------------------------------------------------------
 *  EEPROM
 *--------------------------------------------------------------------------*/
#define EEPROM_DEV       0xA0
#define EEPROM_AL_HOUR   0x00
#define EEPROM_AL_MIN    0x01

/*--------------------------------------------------------------------------
 *  Timing constants (unit: ms)
 *--------------------------------------------------------------------------*/
#define BLINK_HALF_MS    500U   /* 500ms = half blink period            */
#define TIMEOUT_MS      30000U  /* 30s timeout in set modes             */
#define PIP_MS           300U   /* 0.3s short beep                      */
#define ALARM_HALF_MS    500U   /* 0.5s half period for alarm pip-pip   */
#define ALARM_TOTAL_MS  5000U   /* 5s total alarm duration              */

/*--------------------------------------------------------------------------
 *  LED D4 (P0.3)  Active-LOW
 *--------------------------------------------------------------------------*/
#define LED_D4_ON()   P03 = 0
#define LED_D4_OFF()  P03 = 1

/*--------------------------------------------------------------------------
 *  Global state variables
 *--------------------------------------------------------------------------*/
static uint8_t g_state     = STATE_NORMAL;

/* Current clock time */
static uint8_t g_hour = 0;
static uint8_t g_min  = 0;
static uint8_t g_sec  = 0;

/* Alarm time */
static uint8_t g_alarm_hour = 0;
static uint8_t g_alarm_min  = 0;

/* Timing counters (incremented every 1ms inside main loop) */
static uint16_t g_ms_cnt      = 0;   /* 0..999 -> 1s tick          */
static uint16_t g_blink_cnt   = 0;   /* 0..499 -> toggle blink     */
static uint16_t g_timeout_cnt = 0;   /* 0..29999 -> 30s timeout    */

/* Display blink state (1=digits visible, 0=digits hidden) */
static bit g_blink_on;

/* Buzzer state */
static uint16_t g_pip_ms           = 0;    /* remaining ms for short pip    */
static bit      g_alarm_active;            /* 1 = alarm pip-pip in progress */
static uint16_t g_alarm_remain_ms  = 0;   /* remaining alarm time          */
static uint16_t g_alarm_phase_ms   = 0;   /* remaining ms for current phase*/
static bit      g_alarm_phase_on;          /* 1=ON phase, 0=OFF phase       */

/*--------------------------------------------------------------------------
 *  Forward declarations
 *--------------------------------------------------------------------------*/
static void process_key(uint8_t key);
static void update_display(void);
static void tick_second(void);
static void start_pip(void);
static void start_alarm_beep(void);
static void update_buzzer(void);
static void load_alarm_from_eeprom(void);
static void save_alarm_to_eeprom(void);

/*==========================================================================
 *  main
 *==========================================================================*/
int main(void)
{
    /* --- System clock: 32MHz / 4 = 8MHz --- */
    CKCON  = 0x70;
    CLKSEL = 0x05;
    CLKCMD = 0x69;
    CKCON  = 0x00;

    /* --- Peripheral init --- */
    timer0_init();      /* 1ms interrupt                */
    segment_config();   /* 7-SEG GPIO                   */
    keyscan_init();     /* INT0 + row pin config        */
    buzzer_init();      /* Timer2 PWM, starts silent    */
    I2C_Init();         /* I2C for EEPROM               */

    /* LED D4 (P0.3) - push-pull output, starts OFF */
    P0  |= 0x08;
    P0M |= 0x08;
    LED_D4_OFF();

    /* Load alarm from EEPROM */
    load_alarm_from_eeprom();

    /* Init display */
    g_blink_on = 1;
    update_display();

    /*----------------------------------------------------------------------
     *  Main loop
     *----------------------------------------------------------------------*/
    while(1)
    {
        WDTR = 0x5A;    /* Feed watchdog */

        /* ---- 1ms tick tasks ---- */
        if(timer_1ms_flag)
        {
            timer_1ms_flag = 0;

            /* 1. Drive 7-SEG display (4-digit multiplexing) */
            segment_scan();

            /* 2. 1-second counter */
            g_ms_cnt++;
            if(g_ms_cnt >= 1000U)
            {
                g_ms_cnt = 0;
                tick_second();
            }

            /* 3. Blink and timeout (only active in set modes) */
            if(g_state != STATE_NORMAL)
            {
                /* Blink: toggle every 500ms */
                g_blink_cnt++;
                if(g_blink_cnt >= BLINK_HALF_MS)
                {
                    g_blink_cnt = 0;
                    g_blink_on  = !g_blink_on;
                    update_display();   /* Immediate refresh on blink toggle */
                }

                /* 30s timeout -> return to normal with pip */
                g_timeout_cnt++;
                if(g_timeout_cnt >= TIMEOUT_MS)
                {
                    g_timeout_cnt = 0;
                    g_state       = STATE_NORMAL;
                    g_blink_on    = 1;
                    start_pip();
                    update_display();
                }
            }
            else
            {
                g_blink_on = 1;   /* Always show digits in normal mode */
            }

            /* 4. Buzzer duration control */
            update_buzzer();
        }

        /* ---- Key event (from INT0 ISR) ---- */
        if(g_key_event)
        {
            uint8_t k    = g_key_event;
            g_key_event  = 0;
            process_key(k);
        }
    }
}

/*==========================================================================
 *  tick_second - called every 1 second
 *    Clock only advances when in STATE_NORMAL (frozen during editing)
 *==========================================================================*/
static void tick_second(void)
{
    if(g_state != STATE_NORMAL) return;

    g_sec++;
    if(g_sec >= 60)
    {
        g_sec = 0;
        g_min++;

        update_display();   /* Update display on minute change */

        if(g_min >= 60)
        {
            g_min = 0;
            g_hour++;
            if(g_hour >= 24) g_hour = 0;
        }

        /* Check alarm: trigger at exact HH:MM:00 */
        if(g_hour == g_alarm_hour && g_min == g_alarm_min)
        {
            start_alarm_beep();
        }
    }
}

/*==========================================================================
 *  process_key - state machine key handler
 *==========================================================================*/
static void process_key(uint8_t key)
{
    /* Every key press: short pip + reset timeout */
    start_pip();
    g_timeout_cnt = 0;
    g_blink_cnt   = 0;
    g_blink_on    = 1;

    switch(g_state)
    {
        /*------ NORMAL MODE ------*/
        case STATE_NORMAL:
            if(key == KEY_SETUP)
            {
                g_state = STATE_SET_HOUR;
            }
            else if(key == KEY_ALARM)
            {
                g_state = STATE_SET_ALARM_HOUR;
            }
            break;

        /*------ SET HOUR ------*/
        case STATE_SET_HOUR:
            if(key == KEY_SETUP)
            {
                g_state = STATE_SET_MIN;
            }
            else if(key == KEY_PLUS)
            {
                g_hour = (g_hour + 1) % 24;
            }
            else if(key == KEY_MINUS)
            {
                g_hour = (g_hour == 0) ? 23 : (g_hour - 1);
            }
            break;

        /*------ SET MINUTE ------*/
        case STATE_SET_MIN:
            if(key == KEY_SETUP)
            {
                /* Confirm: reset seconds, return to normal */
                g_sec    = 0;
                g_ms_cnt = 0;
                g_state  = STATE_NORMAL;
            }
            else if(key == KEY_PLUS)
            {
                g_min    = (g_min + 1) % 60;
                g_sec    = 0;    /* Reset seconds on minute change */
                g_ms_cnt = 0;
            }
            else if(key == KEY_MINUS)
            {
                g_min    = (g_min == 0) ? 59 : (g_min - 1);
                g_sec    = 0;
                g_ms_cnt = 0;
            }
            break;

        /*------ SET ALARM HOUR ------*/
        case STATE_SET_ALARM_HOUR:
            if(key == KEY_ALARM)
            {
                g_state = STATE_SET_ALARM_MIN;
            }
            else if(key == KEY_PLUS)
            {
                g_alarm_hour = (g_alarm_hour + 1) % 24;
            }
            else if(key == KEY_MINUS)
            {
                g_alarm_hour = (g_alarm_hour == 0) ? 23 : (g_alarm_hour - 1);
            }
            break;

        /*------ SET ALARM MINUTE ------*/
        case STATE_SET_ALARM_MIN:
            if(key == KEY_ALARM)
            {
                /* Save to EEPROM and return to normal */
                save_alarm_to_eeprom();
                g_state = STATE_NORMAL;
            }
            else if(key == KEY_PLUS)
            {
                g_alarm_min = (g_alarm_min + 1) % 60;
            }
            else if(key == KEY_MINUS)
            {
                g_alarm_min = (g_alarm_min == 0) ? 59 : (g_alarm_min - 1);
            }
            break;
    }

    update_display();
}

/*==========================================================================
 *  update_display - refresh 7-SEG buffer + LED D4
 *==========================================================================*/
static void update_display(void)
{
    uint8_t h, m;
    bit show_hh = 1;
    bit show_mm = 1;

    /* Choose time source based on state */
    if(g_state == STATE_SET_ALARM_HOUR || g_state == STATE_SET_ALARM_MIN)
    {
        h = g_alarm_hour;
        m = g_alarm_min;
    }
    else
    {
        h = g_hour;
        m = g_min;
    }

    /* Apply blink to the group being edited */
    if(!g_blink_on)
    {
        if(g_state == STATE_SET_HOUR || g_state == STATE_SET_ALARM_HOUR)
            show_hh = 0;
        else if(g_state == STATE_SET_MIN || g_state == STATE_SET_ALARM_MIN)
            show_mm = 0;
    }

    segment_update(h, m, show_hh, show_mm);

    /* LED D4: blinks only in alarm set modes */
    if(g_state == STATE_SET_ALARM_HOUR || g_state == STATE_SET_ALARM_MIN)
    {
        if(g_blink_on) LED_D4_ON(); else LED_D4_OFF();
    }
    else
    {
        LED_D4_OFF();
    }
}

/*==========================================================================
 *  Buzzer helpers
 *==========================================================================*/

/* start_pip - short pip 0.3s (skipped if alarm is active) */
static void start_pip(void)
{
    if(!g_alarm_active)
    {
        g_pip_ms = PIP_MS;
        buzzer_on();
    }
}

/* start_alarm_beep - 5s pip-pip pattern */
static void start_alarm_beep(void)
{
    g_pip_ms           = 0;          /* Cancel any ongoing pip  */
    g_alarm_active     = 1;
    g_alarm_remain_ms  = ALARM_TOTAL_MS;
    g_alarm_phase_ms   = ALARM_HALF_MS;
    g_alarm_phase_on   = 1;
    buzzer_on();
}

/* update_buzzer - call every 1ms */
static void update_buzzer(void)
{
    if(g_alarm_active)
    {
        /* Alarm pip-pip: count down total duration */
        if(g_alarm_remain_ms > 0)
        {
            g_alarm_remain_ms--;
            g_alarm_phase_ms--;

            /* Toggle buzzer every 500ms */
            if(g_alarm_phase_ms == 0)
            {
                g_alarm_phase_ms = ALARM_HALF_MS;
                g_alarm_phase_on = !g_alarm_phase_on;
                if(g_alarm_phase_on) buzzer_on();
                else                 buzzer_off();
            }
        }
        else
        {
            /* Alarm finished */
            g_alarm_active = 0;
            buzzer_off();
        }
    }
    else if(g_pip_ms > 0)
    {
        /* Short pip countdown */
        g_pip_ms--;
        if(g_pip_ms == 0) buzzer_off();
    }
}

/*==========================================================================
 *  EEPROM helpers
 *==========================================================================*/
static void load_alarm_from_eeprom(void)
{
    g_alarm_hour = EP_RdRead(EEPROM_DEV, EEPROM_AL_HOUR);
    g_alarm_min  = EP_RdRead(EEPROM_DEV, EEPROM_AL_MIN);

    /* Validate - EEPROM may be 0xFF on first use */
    if(g_alarm_hour >= 24) g_alarm_hour = 0;
    if(g_alarm_min  >= 60) g_alarm_min  = 0;
}

static void save_alarm_to_eeprom(void)
{
    EP_WriteByte(EEPROM_DEV, EEPROM_AL_HOUR, g_alarm_hour);
    EP_WriteByte(EEPROM_DEV, EEPROM_AL_MIN,  g_alarm_min);
}
