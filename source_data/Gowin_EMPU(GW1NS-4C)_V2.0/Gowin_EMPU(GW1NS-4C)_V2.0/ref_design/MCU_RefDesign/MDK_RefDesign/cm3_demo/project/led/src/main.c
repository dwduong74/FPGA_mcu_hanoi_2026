/*
 ******************************************************************************************
 * @file      main.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Main program body.
 ******************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "delay.h"


/* Definitions ---------------------------------------------------------------*/

int main(void)
{
	SystemInit();  //Initializes system clock
	gpio_init();   //Initializes GPIO
	delay_init();  //Initializes delay functions
	
  while(1)
  {
    GPIO_ResetBit(GPIO0,GPIO_Pin_0);	//LED1 on
    delay_sec(1);                     //1 sec

    GPIO_SetBit(GPIO0,GPIO_Pin_0);		//LED1 off
    delay_sec(1);
  }
}
