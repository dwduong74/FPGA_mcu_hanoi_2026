/*
 ******************************************************************************************
 * @file      main.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Main program body.
 ******************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <rtthread.h>
#include <stdio.h>
#include "gw1ns4c.h"


/* Definitions ---------------------------------------------------------------*/

int main(void)
{
	while(1)
	{
		GPIO_ResetBit(GPIO0, GPIO_Pin_0);	//LED1 on
		rt_thread_mdelay(500);
		
		GPIO_SetBit(GPIO0, GPIO_Pin_0);		//LED1 off
		rt_thread_mdelay(500);
	}
}
