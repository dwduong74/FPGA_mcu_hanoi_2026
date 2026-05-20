#define __XRAM_SFR_H__
#include "mcu.h"
#include "led.h"
#include "key.h"

void delay_ms(uint16_t ms);
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
  led_config();
	while(1)
  {
		WDTR = 0x5A;                //CLR WDTR
    delay_ms(1);
    Mnkey();
  }
}
/***********************************************************
**Function name   :delay_ms
**
**Input parameter : uint16_t ms
**
**Input parameter :
**
**Description     : Software delay ms
***********************************************************/
void delay_ms(uint16_t ms)
{
  uint16_t i,j;
  for(i = 0; i < 1000;i++)
  {
    WDTR = 0x5A;
    for(j=0;j<ms;j++);
  }
}
  