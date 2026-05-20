#include "mcu.h"
#include "string.h"
#include "uart.h"

/***********************************************************
**Function name   :uart0_config
**
**Input parameter : 
**
**Input parameter :
**
**Description     :Configure UART0 MODE 1 9600 baud rate and io
***********************************************************/
void uart0_config(void)
{	

  P05 = 1;
	P0M |= 0x20; 																  
	P0M &= 0xbf; 					//						
	S0CON = 0X40 | 0X10; //enabel uart and Configure mode 1

  PCON &= 0x7f;       													
  S0CON2 |= 0X80;    //
	S0RELH = 0x03;
  S0RELL = 0xF3;     // 9600
  																							
  ES0 = 1;	        //enable uart interrupt
  EAL = 1;          //enable  General interrupt
}
/***********************************************************
**Function name   :
**
**Input parameter : 
**
**Input parameter :
**
**Description     :
***********************************************************/
void UART0rxISR(void) interrupt ISRUart
{
	if((S0CON&0x01) == 1)
	{
		S0CON &= 0xfe; 
    S0BUF = S0BUF;    // Send the data received by RX through TX
	}
}
