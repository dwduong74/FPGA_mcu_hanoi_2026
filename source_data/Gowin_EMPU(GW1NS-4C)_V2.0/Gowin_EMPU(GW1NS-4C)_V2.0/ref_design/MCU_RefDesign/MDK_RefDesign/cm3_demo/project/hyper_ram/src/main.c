/*
 ******************************************************************************************
 * @file      main.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Main program body.
 ******************************************************************************************
 */

/* Includes ---------------------------------------------------------------------*/
#include <stdio.h>
#include "hyper_ram.h"
#include "uart.h"


/* Definitions ------------------------------------------------------------------*/

#define TEST_NUM 0x2000

uint32_t rec_temp[4];
uint32_t temp[4] = { 0x00};


//Application entry function
int main(void)
{
	SystemInit();	       //Initializes system clock
	uart_init(UART0,     //Initializes UART0
	          38400,     //Baudrate
	          1,         //Tx
	          1,         //Rx
	          0,         //Tx interrupt
	          0,         //Rx interrupt
	          0,         //Tx overflow interrupt
	          0);        //Rx overflow interrupt
	
	printf("---------------------------------------------------------------------\r\n");
	printf("AHB2 master HyperRAM peripherals in FPGA Fabric                      \r\n");
	printf("---------------------------------------------------------------------\r\n");
	printf("\r\n");
	
	printf("System Initialized Successfully!\r\n");
	printf("UART0 Initialized Successfully!\r\n");
	
	//Check and wait HyperRAM initialization finished
	while(HPRAM_Check_Init_Status()!=1);
	
	printf("HyperRAM Initialized Successfully!\r\n");
	
	//Write data into HyperRAM
	printf("\r\nWrite Data Into HyprRAM : \r\n");
	for(uint32_t i = 0; i < TEST_NUM; i = i+8)
	{
		for(uint16_t j = 0; j < 4; j++)
		{
			temp[j] = i;
			
			printf("The Address 0x%x : Write Data %x.\r\n",i,temp[j]);
		}
		
		HPRAM_Write_Data_Buff(temp,i);
	}
	
	//Read Data From HyperRAM
	printf("\r\nRead Data From HyperRAM : \r\n");
	for(uint32_t i = 0; i < TEST_NUM; i = i+8)
	{
		HPRAM_Read_Data_Buff(rec_temp,i);
		
		for(uint16_t j = 0; j < 4; j++)
		{
			printf("The Address 0x%x : Read Data %x.\r\n",i,rec_temp[j]);
		}
	}
	
	printf("\r\nHyperRAM PASS.\r\n");

	while(1);
}
