#include "exint.h"
#include "7segment_led.h"

#define	keyout0		P44																						//按键接口引脚定义
#define	keyout1		P45
#define	keyout2		P46
#define	keyout3		P47

#define	keyin0		P24
#define	keyin1		P25
#define	keyin2		P26
#define	keyin3		P27

uint8_t	keyinbuf1,keyinbuf2;			
/***********************************************************
**Function name   :int0_config()
**
**Input parameter : 
**
**Input parameter :
**
**Description     :INT0 is configured to trigger the falling edge
***********************************************************/
void int0_config()
{
  P4  &= 0X0F;
  P4M |= 0XF0;
  
  PEDGE = 0X02;
  EX0 = 1;
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
void int0_isr(void) interrupt ISRInt0
{
  
  uint8_t key_value;
  uint8_t i;
  PEDGE = 0;
  P44 = 0;
  P4 |= 0XEF; 
  for(i = 0;i<8;i++);
  if(keyin0 == 0)
    key_value = 13;
  if(keyin1 == 0)
    key_value = 9;
  if(keyin2 == 0)
    key_value = 5;
  if(keyin3 == 0)
    key_value = 1;  

  
  P4 |= 0XDF; 
  P45 = 0;
  for(i = 0;i<8;i++);
  if(keyin0 == 0)
    key_value = 14;
  if(keyin1 == 0)
    key_value = 10;
  if(keyin2 == 0)
    key_value = 6;
  if(keyin3 == 0)
    key_value = 2;  
  
  P4 |= 0XbF; 
  P46 = 0;
  for(i = 0;i<8;i++);
  if(keyin0 == 0)
    key_value = 15;
  if(keyin1 == 0)
    key_value = 11;
  if(keyin2 == 0)
    key_value = 7;
  if(keyin3 == 0)
    key_value = 3;  

  P4 |= 0X7F; 
  P47 = 0;
  for(i = 0;i<8;i++);
  if(keyin0 == 0)
    key_value = 16;
  if(keyin1 == 0)
    key_value = 12;
  if(keyin2 == 0)
    key_value = 8;
  if(keyin3 == 0)
    key_value = 4;    
  
  P4 &= 0X0F;
  segment_displayhex(key_value);   

  PEDGE = 0X02;
}