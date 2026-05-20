#define __XRAM_SFR_H__
#include "mcu.h"
#include "7segment_led.h"
#include "adc.h"
void delay_ms(uint16_t ms);
uint16_t sys_count;
uint16_t ad_value;
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
  segment_led_config();
  adc_config();
	while(1)
  {
		WDTR = 0x5A;                //CLR WDTR
    delay_ms(10);
    segment_led_scan();
    segment_display(ad_value);  // 
    if(++sys_count > 40)
    {
      sys_count = 0;
      ad_value = GetAD(1);
      
    }
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
  for(i = 0; i < 100;i++)
  {
    WDTR = 0x5A;
    for(j=0;j<ms;j++);
  }
}
  