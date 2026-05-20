/*
 ******************************************************************************************
 * @file      led.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     uC/OS-III task application.
 ******************************************************************************************
 */

/* Include-------------------------------------------------------------------------------- */
#include "gpio.h"
#include "led.h"

/* Definitions---------------------------------------------------------------------------- */

//Initializes GPIO
void led_init(void)
{
  gpio_init();
}

//led on
void led_on(uint8_t led)
{
  switch (led)
  {
		case 0:
     	GPIO_ResetBit(GPIO0, GPIO_Pin_0);
     	break;
    case 1:
     	GPIO_ResetBit(GPIO0, GPIO_Pin_1);
     	break;
    case 2:
     	GPIO_ResetBit(GPIO0, GPIO_Pin_2);
     	break;
    case 3:
     	GPIO_ResetBit(GPIO0, GPIO_Pin_3);
     	break;
    default:
     	break;
  }
}

//led off
void led_off(uint8_t led)
{
	switch (led)
  {
    case 0:
     	GPIO_SetBit(GPIO0, GPIO_Pin_0);
     	break;
    case 1:
     	GPIO_SetBit(GPIO0, GPIO_Pin_1);
     	break;
    case 2:
     	GPIO_SetBit(GPIO0, GPIO_Pin_2);
     	break;
    case 3:
     	GPIO_SetBit(GPIO0, GPIO_Pin_3);
     	break;
    default:
     	break;
  }
}

//led toggle
void led_toggle(uint8_t led)
{
	uint32_t temp;

	temp = GPIO0->DATAOUT;

	switch (led)
  {
    case 0:
    	if (temp & GPIO_Pin_0)
     	{
     		GPIO_ResetBit(GPIO0, GPIO_Pin_0);
     	}
     	else
     	{
     		GPIO_SetBit(GPIO0, GPIO_Pin_0);
     	}
	    break;
    case 1:
     	if (temp & GPIO_Pin_1)
     	{
     		GPIO_ResetBit(GPIO0, GPIO_Pin_1);
     	}
    	else
     	{
     		GPIO_SetBit(GPIO0, GPIO_Pin_1);
     	}
		  break;
    case 2:
     	if (temp & GPIO_Pin_2)
     	{
     		GPIO_ResetBit(GPIO0, GPIO_Pin_2);
     	}
     	else
     	{
     		GPIO_SetBit(GPIO0, GPIO_Pin_2);
     	}
			break;
    case 3:
     	if (temp & GPIO_Pin_3)
     	{
     		GPIO_ResetBit(GPIO0, GPIO_Pin_3);
     	}
     	else
     	{
     		GPIO_SetBit(GPIO0, GPIO_Pin_3);
     	}
		  break;
    default:
     	break;
  }
}
