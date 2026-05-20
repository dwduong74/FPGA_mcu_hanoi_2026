/*
 ******************************************************************************************
 * @file      keyscan_demo.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Demo key scan.
 ******************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "demo.h"


/* Definitions ---------------------------------------------------------------*/

#if KEYSCAN_DEMO

/* Includes: */
#include "gpio.h"
#include "delay.h"


/* Definitions: */

#define KEY_ON	1
#define KEY_OFF	0

#define N_BLINK 5


uint8_t GPIO_ReadInputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  uint8_t bitstatus = 0x00;

  if ((GPIOx->DATA & GPIO_Pin) != 0)
  {
    bitstatus = (uint8_t)1;
  }
  else
  {
    bitstatus = (uint8_t)0;
  }

  return bitstatus;
}

//Scan key status
uint8_t Key_Scan(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin)
{
	//whether key is press
	if(GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON)
	{
		//wait free key
		while(GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON);

		return 	KEY_ON;
	}
	else
	{
		return KEY_OFF;
	}
}

//Application entry function
int keyscan_demo(void)
{
	SystemInit();		//Initializes system clock
	gpio_init();		//Initializes GPIO
	delay_init();		//Initializes delay functions

	while(1)
	{
		if(Key_Scan(GPIO0, GPIO_Pin_1) == KEY_ON)
		{
			for(int i = 0;i < N_BLINK;i++)
			{
				GPIO_ResetBit(GPIO0, GPIO_Pin_0);
				delay_sec(1);

				GPIO_SetBit(GPIO0, GPIO_Pin_0);
				delay_sec(1);
			}
		}
	}
}

#endif
