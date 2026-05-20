#define __XRAM_SFR_H__
#include "mcu.h"
#include "uart.h"
#include "stdio.h"
void delay_ms(uint16_t ms);
uint32_t sys_running_timer;
/***********************************************************
**Function name   :
**
**Input parameter : 
**
**Input parameter :
**
**Description     :
***********************************************************/
int main(void)
{ 
	CKCON 	=	0X70;
	CLKSEL 	=	0X05;								//Fcpu = 32M/4
	CLKCMD	=	0X69;								
	CKCON 	=	0X00;	  
  uart0_config();
	while(1)
  {
		WDTR = 0x5A;                //CLR WDTR
  }
}
