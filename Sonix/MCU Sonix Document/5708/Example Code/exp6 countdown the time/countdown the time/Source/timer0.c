#include "timer0.h"
#include "7segment_led.h"
uint16_t count_1s;
/***********************************************************
**Function name   :timer0_config()
**
**Input parameter : 
**
**Input parameter :
**
**Description     :Configure T0 1mS
***********************************************************/
void timer0_config(void)
{
  TMOD = 0x05;          //mode 1 16 bit timer0 
	TCON0 = 0x20;         // fexto /32 = 32MHZ/32
  TH0 = (65536 - 1000)>>8;             
  TL0 = (65536 - 1000);						    	
	TR0 = 1;										  
  ET0 = 1;   
  EAL = 1;
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
void timer0_isr(void) interrupt ISRTimer0   // Vector @  0x0B
{
  TH0 = (65536 - 1000)>>8;    //        
  TL0 = (65536 - 1000);		 
  if(++count_1s > 1000) // 1000* 1MS = 1S
  {
    count_1s = 0;
    if(sys_count == 0)
      sys_count = 9999 ;
    else
      sys_count -- ;
  }
}

