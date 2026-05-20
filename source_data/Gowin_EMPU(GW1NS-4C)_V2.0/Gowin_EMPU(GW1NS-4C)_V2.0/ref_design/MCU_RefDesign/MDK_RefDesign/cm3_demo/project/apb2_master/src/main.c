/*
 ******************************************************************************************
 * @file          main.c
 * @author        GowinSemicoductor
 * @device        Gowin_EMPU(GW1NS-4C)
 * @brief         Main function.
 ******************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "uart.h"
#include "multiple.h"
#include <stdio.h>


/* Definitions ---------------------------------------------------------------*/

//Application entry function
int main(void)
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

	printf("---------------------------------------------------\r\n");
	printf("Extended APB2 master peripherals in FPGA Fabric.\r\n");
	printf("---------------------------------------------------\r\n");
	printf("\r\n");
	
  printf("Initialization Status : \r\n");
  printf("--MULTIPLIER = %d\r\n",getMultiplier());
  printf("--MULTIPLICAND = %d\r\n",getMultiplicand());
  printf("--CMD = %d\r\n",getMultipleCmd());
  printf("--RESULT = %d\r\n",getMultipleResult());
	printf("\r\n");

  printf("Start first multiple\r\n");
  setMultiplier(20);
  setMultiplicand(40);
  startMultiple();
  printf("Compute Status : \r\n");
  printf("--Multiplier = %d\r\n",getMultiplier());
  printf("--Multiplicand = %d\r\n",getMultiplicand());
  printf("--CMD = %d\r\n",getMultipleCmd());

  while(getFinishStatus()==FINISHED_STATUS);
  finishMultiple();

  printf("Finished Status : \r\n");
  printf("--Multiplier = %d\r\n",getMultiplier());
  printf("--Multiplicand = %d\r\n",getMultiplicand());
  printf("--CMD = %d\r\n",getMultipleCmd());
  printf("--RESULT = %d\r\n",getMultipleResult());
  printf("Multiple first finished.\r\n");
	printf("\r\n");

  printf("Start second multiple\r\n");
  setMultiplier(30);
  setMultiplicand(50);
  startMultiple();
  printf("Compute Status : \r\n");
  printf("--Multiplier = %d\r\n",getMultiplier());
  printf("--Multiplicand = %d\r\n",getMultiplicand());
  printf("--CMD = %d\r\n",getMultipleCmd());

  while(getFinishStatus()==FINISHED_STATUS);
  finishMultiple();

  printf("Finished Status : \r\n");
  printf("--Multiplier = %d\r\n",getMultiplier());
  printf("--Multiplicand = %d\r\n",getMultiplicand());
  printf("--CMD = %d\r\n",getMultipleCmd());
  printf("--RESULT = %d\r\n",getMultipleResult());
  printf("Multiple second finished.\r\n");
	
	printf("\r\nEND!\r\n");

  while(1);
}
