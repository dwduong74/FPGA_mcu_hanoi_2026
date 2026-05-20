/*
 ******************************************************************************************
 * @file      led.h
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     uC/OS-III task application
 ******************************************************************************************
 */

#ifndef	__LED_H_
#define	__LED_H_

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

/* Includes-------------------------------------------------------------------------------- */
#include <stdint.h>

/* Declarations----------------------------------------------------------------------------- */
extern void led_init(void);
extern void led_on(uint8_t led);
extern void led_off(uint8_t led);
extern void led_toggle(uint8_t led);


#ifdef __cplusplus
}  //extern "C"
#endif //__cplusplus

#endif    /* __LED_H_ */
