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
	SystemInit();       //Initializes system clock
  uart_init(UART0,    //Initializes UART0
	          38400,    //Baudrate
	          1,        //Tx
	          1,        //Rx
	          0,        //Tx interrupt
	          0,        //Rx interrupt
	          0,        //Tx overflow interrupt
	          0);       //Rx overflow interrupt
	I2C_Init(I2C,400);  //Initializes I2C master
	
	unsigned char tx[100];			//Tx data
	unsigned char rx[100];			//Rx data
	
	//Initializes Tx data
	for(int i = 0; i < 100; i++)
	{
		tx[i] = i;
	}
	
	printf("I2C master will send these data...\r\n");
	for(int i = 0; i < 30; i++)
	{
		printf("%d ", tx[i]);
	}
	printf("\r\n\r\n");
	
	//I2C master send data
	I2C_SendBytes(I2C, 0x50, 0x00, tx, 30);
	
	//I2C master receive data
	I2C_ReadBytes(I2C, 0x50, 0x00, rx, 30);
	
	printf("I2C master has received these data...\r\n");
	for(int i = 0; i < 30; i++)
	{
		printf("%d ",rx[i]);
	}
	printf("\r\n");
	
	while(1);
}
