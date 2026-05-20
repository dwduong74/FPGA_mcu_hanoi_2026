#include "mcu.h"
#include "led.h"

/***********************************************************
**Function name   :led_config()
**
**Input parameter : 
**
**Input parameter :
**
**Description     :P03 P04 Set it to output 1
***********************************************************/
void led_config()
{
  P0 |= 0X18;
  P0M |= 0X18;
}
/***********************************************************
**Function name   :led_ctr
**
**Input parameter : led:led Serial number  cmd: 0or 1  turn on or off
**
**Input parameter :
**
**Description     :Control  LED to turn on or off
***********************************************************/
void led_ctr(uint8_t led,uint8_t cmd)
{
  switch(led)
  {
    case 0:LED0_PIN = cmd;break;
    case 1:LED1_PIN = cmd;break;
  }
}