#include "exint.h"
#include "7segment_led.h"
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
  if(++sys_count > 9999)
    sys_count = 0;
}