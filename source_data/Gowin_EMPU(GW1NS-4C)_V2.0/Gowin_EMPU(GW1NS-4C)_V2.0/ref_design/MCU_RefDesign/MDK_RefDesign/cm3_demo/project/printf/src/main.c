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
#include <stdio.h>


/* Definitions ---------------------------------------------------------------*/

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
	
  char *ptr = "Hello world!";
  int i = 5;
	unsigned int bs = sizeof(int)*8;
	int mi;
	
	mi = (1 << (bs-1)) + 1;

	printf("%s\r\n", ptr);
	printf("%d = 5\r\n", i);
	printf("%d = - max integer\r\n", mi);
	printf("char %c = 'a'\r\n", 'a');
	printf("hex %x = ff\r\n", 0xff);
	printf("hex %02x = 00\r\n", 0);
	printf("signed %d = unsigned %u = hex %x\r\n", -3, -3, -3);
	printf("%d %s(s)", 0, "message");
	printf("\r\n");
	printf("%d %s(s) with %%\r\n", 0, "message");
	
	while(1);
}
