/*
 ******************************************************************************************
 * @file      main.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Main program body.
 ******************************************************************************************
 */

/* Includes ----------------------------------------------------------------------*/
#include "spi_flash.h"
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
	spi_flash_init();    //Initializes SPI-Flash
	
	printf("Running instructions in userflash!\r\n");
	printf("Running instructions in userflash!\r\n");
	
	int i = 0;
	uint8_t* ram_ro = (uint8_t*)(0x20002000);	//Write into SRAM from SPI-Flash
	
	//Write into SRAM from SPI-Flash
	//8KB in SRAM
	for(i = 0;i < 64;i++)
	{
		//Read 128 byte once
		spi_flash_read(0x80, 0x03, 0x0+i*0x80, ram_ro);
		ram_ro += 0x80;
	}
	
	//Running in SRAM
	ram_func();

  while(1);
}
