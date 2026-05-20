/*
 ******************************************************************************************
 * @file      main.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Main program body.
 ******************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "delay.h"
#include "gpio.h"


/* Definitions ---------------------------------------------------------------*/

//Application entry function
int main(void)
{
	SystemInit();		//Initializes system clock
	gpio_init();		//Initializes GPIO
	delay_init();		//Initializes delay functions
	
	//Set low level to trigger input interrupt
	GPIO_SetIntLowLevel(GPIO0, GPIO_Pin_1);
	
	//Enable interrupt
	nvic_irq_enable(PORT0_1_IRQn, 0, 0);
	
	while(1);
}

//GPIO pin 1 interrupt handler
void PORT0_1_Handler(void)
{
	GPIO_ResetBit(GPIO0, GPIO_Pin_0);
	delay_ms(2000);
	
	GPIO_SetBit(GPIO0, GPIO_Pin_0);
	delay_ms(2000);
	
	GPIO_IntClear(GPIO0, GPIO_Pin_1);
}
