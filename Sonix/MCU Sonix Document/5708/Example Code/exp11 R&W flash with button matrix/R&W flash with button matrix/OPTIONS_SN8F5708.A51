$NOMOD51
;------------------------------------------------------------------------------
;
;  Copyright (c) 2015 SONiX Technology Co., Ltd.
;  Version 1.0 - SN8F5708, SN8F5707, SN8F5705
;
;  *** <<< Use Configuration Wizard in Context Menu >>> ***
;------------------------------------------------------------------------------
;
;  This preference, such as watchdog, external reset pin, and clock source, is preloaded 
;  during the microcontroller's power-on. It is strongly recommanded to use configuration 
;  wizard to set these parameters up appropriately.
;
;------------------------------------------------------------------------------
;
;   <o> Program Memory Security <0x00=> Enable <0x01=> Disable
;   <i> The debug interface cannot read program memory if this security option is enable.
;   <i> Erase All Program Memory Action can be proformmed.
    SECURITY_SET    EQU     0x01
;
;   <o.1..3> CPU Clock Source <0x00=> X'tal 4 MHz <0x01=> X'tal 12 MHz <0x02=> External Clock <0x03=> IHRC 32 MHz with RTC <0x07=> IHRC 32 MHz
;   <i> X'tal 4 MHz: off-chip crystal between 1 MHz and 8 MHz.
;   <i> X'tal 12 MHz: off-chip crystal between 8 MHz and 16 MHz.
;   <i> External Clock: external clock input from XIN pin.
;   <i> IHRC 32 MHz: on-chip internal clock with or without Timer 0 real time clock.
    CLOCKSRC_SET    EQU     0x0E
;
;   <o> Noise Filter <0x01=> Disable <0x00=> Enable
    NOISEFILT_SET   EQU     0x01
;   <h> Reset Sources
;       <o.1..2> VDD Voltage (Low Voltage Detection) <0x00=> LVD_Max <0x01=> LVD_H <0x02=> LVD_M <0x03=> LVD_L
;       <i> LVD_Max: 3.3 V or below triggers reset.
;       <i> LVD_H: 2.4 V or below triggers reset with LVD33 indicator.
;       <i> LVD_M: 1.8 V or below triggers reset with LVD24 indicator.
;       <i> LVD_L: 1.8 V or below triggers reset.
        LVDMODE_SET     EQU     0x02
;
;       <o.4> External Reset / GPIO Shared Pin  <0x00=> Reset  <0x01=> GPIO
;       <i> Reset: Triggers reset if this pin is pulled low.
;       <i> GPIO: The shared pin is reserved for general purpose input/output.
        RESETPIN_SET    EQU     0x10
;
;       <o.4..7> Watchdog <0x00=> Always <0x05=> Enable <0x0A=> Disable
;       <i> Always: Trun on watchdog function including Normal, IDLE, and SLEEP mode.
;       <i> Enable: Turn on watchdog function only in Normal mode.
;       <i> Disable: Turn off watchdog function.
        WATCHDOG_SET    EQU     0x50
;
;       <o.6..7> Watchdog Overflow Period <0x00=> 64 ms <0x01=> 128 ms <0x02=> 256 ms <0x03=> 512 ms
;       <i> The watchdog overflow period is based on Internal Low R-C Clock which has a gentle inaccuracy.
        WATCHCLK_SET    EQU     0xC0
;   </h>


    CSEG    AT      0x3FFB
    DB      0xFC + NOISEFILT_SET + (NOISEFILT_SET << 1)
    DB      WATCHCLK_SET + RESETPIN_SET + LVDMODE_SET + NOISEFILT_SET
    DB      0x5A
    DB      0xA5
    DB      WATCHDOG_SET + CLOCKSRC_SET + SECURITY_SET
    END
