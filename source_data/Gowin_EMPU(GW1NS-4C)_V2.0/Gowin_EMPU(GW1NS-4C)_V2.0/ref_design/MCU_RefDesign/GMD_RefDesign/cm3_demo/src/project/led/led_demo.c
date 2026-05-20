/*
 ******************************************************************************************
 * @file      led_demo.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Demo led.
 ******************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "demo.h"


/* Definitions ---------------------------------------------------------------*/

#if LED_DEMO

/* Includes: */
#include "gpio.h"
#include "delay.h"


/* Definitions: */

//Application entry function
int led_demo(void)
{
  SystemInit();  //Initializes system clock
  gpio_init();   //Initializes GPIO
  delay_init();  //Initializes delay functions
	
  while(1)
  {
    GPIO_ResetBit(GPIO0,GPIO_Pin_0);	//LED1 on
    delay_sec(1);                       //1 sec

    GPIO_SetBit(GPIO0,GPIO_Pin_0);		//LED1 off
    delay_sec(1);
  }
}

#endif
