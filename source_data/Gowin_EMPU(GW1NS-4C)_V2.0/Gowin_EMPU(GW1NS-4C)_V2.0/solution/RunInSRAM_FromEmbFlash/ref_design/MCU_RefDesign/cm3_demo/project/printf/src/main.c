/*
 ******************************************************************************************
 * @file      main.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Main program body.
 ******************************************************************************************
 */

/* Includes ----------------------------------------------------------------------*/
#include "uart.h"
#include <stdio.h>


/* Definitions  ------------------------------------------------------------------*/

extern void ram_func(void);


//Application entry function
int main(void)
{ 
  SystemInit();        //Initializes system clock
  uart_init(UART0,     //Initializes UART0
	          38400,     //Baudrate
	          1,         //Tx
	          1,         //Rx
	          0,         //Tx interrupt
	          0,         //Rx interrupt
	          0,         //Tx overflow interrupt
	          0);        //Rx overflow interrupt
	

	printf("Running instrunctions in userflash!\r\n");
	printf("Running instrunctions in userflash!\r\n");
	
	//Running in SRAM
	ram_func();

  while(1);
}
