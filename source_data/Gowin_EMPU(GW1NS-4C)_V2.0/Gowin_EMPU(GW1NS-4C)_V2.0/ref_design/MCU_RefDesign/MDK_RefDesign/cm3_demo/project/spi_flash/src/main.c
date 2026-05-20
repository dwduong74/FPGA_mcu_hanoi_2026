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
#include "uart.h"
#include "spi_flash.h"
#include <stdio.h>


/* Definitions ---------------------------------------------------------------*/

#define BYTES_NUM 1055

uint8_t tx_buffer[BYTES_NUM] = {0};	//Write data
uint8_t rx_buffer[BYTES_NUM] = {0};	//Read data


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
  spi_flash_init();   //Initializes SPI Flash
	delay_init();       //Initializes delay functions
	
	//Delay some times to wait stable
	delay_ms(100);

  //Change SPI Flash from download mode to memory mode
  //change_mode_spi_flash();

  int i = 0;
  int rest = 0;

  //****** Write data to SPI Flash ******//
  for(i = 0;i < BYTES_NUM;i++)
  {
    tx_buffer[i] = i % 256;
  }

  printf("Write data into SPI Flash...\r\n");

  //Must erase before write.
  spi_flash_4ksector_erase(0x600000);

  //Write data
  if(BYTES_NUM <= 256)
  {
    spi_flash_write(BYTES_NUM, PROGRAM_CMD, 0x600000, tx_buffer);
  }
  else
  {
    for(i = 0;i < BYTES_NUM/256;i++)
    {
      spi_flash_write(256, PROGRAM_CMD, 0x600000+i*0x100, tx_buffer+i*256);
    }
    rest = BYTES_NUM - i*256;
    if(rest != 0)
    {
      spi_flash_write(rest, PROGRAM_CMD, 0x600000+i*0x100, tx_buffer+i*256);
    }
  }

  printf("Write done!\r\n\r\n");

  //****** Read data from SPI Flash ******//
  printf("Read data from SPI Flash...\r\n");

  //Read data
  if(BYTES_NUM <= 256)
  {
    spi_flash_read(BYTES_NUM, READ_CMD, 0x600000, rx_buffer);
  }
  else
  {
    for(i = 0;i < BYTES_NUM/256;i++)
    {
      spi_flash_read(256, READ_CMD, 0x600000+i*0x100, rx_buffer+i*256);
    }
    rest = BYTES_NUM - i*256;
    if(rest != 0)
    {
      spi_flash_read(rest, READ_CMD, 0x600000+i*0x100, rx_buffer+i*256);
    }
  }
	
	printf("Read done!\r\n\r\n");
	
  //Print data
  for(i = 0;i < BYTES_NUM;i ++ )
  {
    printf("%d ",rx_buffer[i]);

    if(((i+1)%256) == 0)
    {
      printf("\r\n");
    }
  }
  printf("\r\n");
	
	//Compare
	for(i = 0;i < BYTES_NUM;i++)
	{
		if(tx_buffer[i] != rx_buffer[i])
		{
			printf("FAILED!\r\n");
			break;
		}
	}
	
	printf("\r\nPASS!\r\n");

  while(1);
}
