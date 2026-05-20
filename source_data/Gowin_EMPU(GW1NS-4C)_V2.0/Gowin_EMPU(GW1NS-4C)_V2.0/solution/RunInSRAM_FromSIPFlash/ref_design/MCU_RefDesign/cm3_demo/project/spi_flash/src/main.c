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
#include "spi_flash.h"
#include <stdio.h>


/* Definitions ---------------------------------------------------------------*/

uint8_t ER_ROM1[1024*8] = 
{
  0x40, 0xF2, 0xA1, 0x1C, 0xC0, 0xF2, 0x00, 0x0C, 0x60, 0x47, 0x00, 0x00, 0x10, 0xB5, 0x06, 0xA0, 
  0xFF, 0xF7, 0xF6, 0xFF, 0x05, 0xA0, 0xFF, 0xF7, 0xF3, 0xFF, 0x04, 0xA0, 0xFF, 0xF7, 0xF0, 0xFF, 
  0x0E, 0xA0, 0xFF, 0xF7, 0xED, 0xFF, 0x10, 0xBD, 0x0D, 0x0A, 0x00, 0x00, 0x52, 0x75, 0x6E, 0x6E, 
  0x69, 0x6E, 0x67, 0x20, 0x69, 0x6E, 0x73, 0x74, 0x72, 0x75, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x73, 
  0x20, 0x69, 0x6E, 0x20, 0x53, 0x52, 0x41, 0x4D, 0x20, 0x66, 0x72, 0x6F, 0x6D, 0x20, 0x53, 0x50, 
  0x49, 0x2D, 0x46, 0x6C, 0x61, 0x73, 0x68, 0x21, 0x0D, 0x0A, 0x00, 0x00, 0x48, 0x65, 0x6C, 0x6C, 
  0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64, 0x2E, 0x0D, 0x0A, 0x00, 0x00
};


void delay_ms(__IO uint32_t delay_ms)
{
	for(delay_ms=(SystemCoreClock>>13)*delay_ms; delay_ms != 0; delay_ms--);
}

//Application entry function
int main(void)
{
	int i, flag = 0;
	uint8_t* p_rom = ER_ROM1;
	uint8_t rx_buffer[0x80] = {0};		//Only used to verify
																		//Change length as actual size
	
  SystemInit();        //Initializes system clock
  uart_init(UART0,     //Initializes UART0
	          38400,     //Baudrate
	          1,         //Tx
	          1,         //Rx
	          0,         //Tx interrupt
	          0,         //Rx interrupt
	          0,         //Tx overflow interrupt
	          0);        //Rx overflow interrupt
	spi_flash_init();	   //Initializes SPI-Flash
	
	delay_ms(10);				 //Wait SPI-Flash initialization
	
	//Erase SPI-Flash
	spi_flash_4ksector_erase(0x000000);
	
	//Write ER_ROM1 into SPI-Flash
	for(i = 0;i < 64;i++)
	{
		spi_flash_page_program(0x80, 0x000000+i*0x80, p_rom);
		p_rom += 0x80;
	}
	
	//Verification
	//Read data from SPI-Flash
	spi_flash_read(0x80, READ_CMD, 0x000000, rx_buffer);
	
	//Compare
	for(i = 0;i < 0x80;i++)
	{
		if(rx_buffer[i]!=ER_ROM1[i])
		{
			flag = 1;
			break;
		}
	}
	
	if(flag)
	{
		printf("Program SPI-Flash Failed.\r\n");
	}
	else
	{
		printf("Program SPI-Flash Successfully.\r\n");
	}
	
	while(1);
}
