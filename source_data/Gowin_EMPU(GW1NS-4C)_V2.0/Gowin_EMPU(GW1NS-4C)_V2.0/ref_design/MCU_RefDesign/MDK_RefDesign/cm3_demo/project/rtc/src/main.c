/*
 ******************************************************************************************
 * @file      main.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Main program body.
 ******************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "uart.h"
#include "delay.h"
#include "gpio.h"
#include <stdio.h>


/* Definitions ---------------------------------------------------------------*/

//Initializes RTC
void rtc_init(void)
{
	Set_Match_Value(10);	//Match 10
	Set_Load_Value(0);		//0 Start
	RTC_Inter_Mask_Set();

	Start_RTC();					//Start
}

//Application entry function
int main(void)
{
  SystemInit();     //Initialzes system clock
	gpio_init();      //Initializes GPIO
	uart_init(UART0,  //Initializes UART0
	          38400,  //Baudrate
	          1,	    //Tx
	          1,      //Rx
	          0,      //Tx interrupt
	          0,      //Rx interrupt
	          0,      //Tx overflow interrupt
	          0);     //Rx overflow interrupt
  rtc_init();       //Initializes RTC
	delay_init();     //Initializes delay functions
	
	uint32_t rtc_current_val = 0;
	
  while(1)
	{
		rtc_current_val = Get_Current_Value();
		printf("Current RTC Value : 0x%X\r\n", rtc_current_val);
		delay_ms(1000);
	}
}
