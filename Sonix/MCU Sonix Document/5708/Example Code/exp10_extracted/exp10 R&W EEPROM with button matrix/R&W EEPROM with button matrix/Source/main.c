#define __XRAM_SFR_H__
#include "mcu.h"
#include "exint.h"
#include "i2c.h"
#include "7segment_led.h"

uint32_t sys_running_timer;
uint8_t epprom_addr;
/***********************************************************
**函数名     :
**功能描述   :
**
**输入参数   : 
**
**返回参数   :
**
**使用说明   :主循环
***********************************************************/
int main(void)
{ 
	uint8_t i = 0x80;
	CKCON 	=	0X70;
	CLKSEL 	=	0X05;								//Fcpu = 32M/4
	CLKCMD	=	0X69;								
	CKCON 	=	0X00;	  
  I2C_Init();
  int0_config(); 
	segment_led_config();
	segment_displayhex(0);
	
	while(1)
  {
		WDTR = 0x5A;                //CLR WDTR   
		key_value_app();		
    delay_ms(10);
    segment_led_scan();  		
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