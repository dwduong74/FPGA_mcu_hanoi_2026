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

//Initializes SPI
void spi_init(void)
{
  SPI_InitTypeDef init_spi;

  init_spi.CLKSEL= CLKSEL_CLK_DIV_8;  //50MHZ / 8
  init_spi.DIRECTION = DISABLE;       //MSB First
  init_spi.PHASE =DISABLE;            //posedge
  init_spi.POLARITY =DISABLE;         //polarity 0

  SPI_Init(&init_spi);								//Initialized

  printf("Initializes SPI master...\r\n\r\n");
}

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
  spi_init();        //Initializes SPI

  SPI_Select_Slave(0x01);  			//Select SPI slave
  SPI_WriteData(0x9F);     			//Send Jedec

  while(1)
  {
		//Write data
		printf("SPI master transmits data...\r\n");
    if(~SPI_GetToeStatus() && SPI_GetTrdyStatus() == 1)
    {
      SPI_WriteData(0x9F);		//Send Jedec
    }

		//Read data
		printf("SPI master receives data: ");
    if(~SPI_GetRoeStatus() && SPI_GetRrdyStatus() == 1)
    {
      printf("%x\r\n\r\n", SPI_ReadData());		
    }
  }
}
