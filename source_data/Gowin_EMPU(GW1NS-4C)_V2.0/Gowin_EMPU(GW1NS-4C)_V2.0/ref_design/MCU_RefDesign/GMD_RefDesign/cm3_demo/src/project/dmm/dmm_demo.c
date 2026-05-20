/*
 ******************************************************************************************
 * @file      dmm_demo.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Demo dynamic memory management.
 ******************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "demo.h"


/* Definitions ---------------------------------------------------------------*/

#if DMM_DEMO

/* Includes: */
#include "malloc.h"
#include "uart.h"
#include <stdio.h>


/* Definitions: */

//Application entry function
int dmm_demo(void)
{
	SystemInit();      //Initializes system clock
	uart_init(UART0,   //Initializes UART0
	          38400,   //Baudrate
	          1,       //Tx
	          1,       //Rx
	          0,       //Tx interrupt
	          0,       //Rx interrupt
	          0,       //Tx overflow interrupt
	          0);      //Rx overflow interrupt
	mem_init();        //Initializes dynamic memory management functions

	char* buff;

	printf("Allocate dynamic memory...\r\n");
	printf("Initializes value 'Gowin: Hello World'...\r\n");

	//Allocate memory
	buff = (char*)mymalloc(100);
	buff = "Gowin: Hello World!\r\n";

	printf("\r\n%s\r\n", buff);

	//Free memory
	myfree(buff);

	printf("Free dynamic memory...\r\n");
	printf("\r\nPASS!\r\n");

	while(1);
}

#endif
