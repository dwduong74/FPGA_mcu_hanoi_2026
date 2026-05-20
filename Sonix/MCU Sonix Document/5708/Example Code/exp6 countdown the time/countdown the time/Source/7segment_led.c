#include "mcu.h"
#include "7segment_led.h"

uint8_t led_buff[4];  //Display cache register

code  uint8_t numble_tab[16]= {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};  //0~F


/***********************************************************
**Function name   :segment_led_config()
**
**Input parameter : 
**
**Input parameter :
**
**Description     :Configure io for display nixie tube
***********************************************************/
void segment_led_config()
{
  P3 = 0X00;
  P3M |= 0XFF;
  P5M &= 0XF0;
  P5M |= 0X0F;
}
/***********************************************************
**Function name   :segment_led_scan()
**
**Input parameter : 
**
**Input parameter :
**
**Description     :Dynamic scanning display nixie tube
***********************************************************/
void segment_led_scan()
{
  static uint8_t cnt = 0;
  SEG_COM0 = 0;
  SEG_COM1 = 0;
  SEG_COM2 = 0;
  SEG_COM3 = 0;  
  switch(cnt)
  {
    case 0:SEG_COM0 = 1; break;
    case 1:SEG_COM1 = 1; break;
    case 2:SEG_COM2 = 1; break;
    case 3:SEG_COM3 = 1; break;    
  }
  P3  = led_buff[cnt];

  
  if(++cnt > 3 )
    cnt = 0;
}

/***********************************************************
**Function name   :segment_display(uint16_t tempdata)
**
**Input parameter : uint16_t tempdata
**
**Input parameter : null
**
**Description     :Displays a number less than 9999 on the nixie tube
***********************************************************/
void segment_display(uint16_t tempdata)
{
  if(tempdata > 9999){
    led_buff[0] = 0x40;
    led_buff[1] = 0x40;
    led_buff[2] = 0x40;
    led_buff[3] = 0x40;   //数据太大 显示 "----"
  }else{
    led_buff[3] = numble_tab[tempdata%10];
    tempdata /= 10;
    led_buff[2] = numble_tab[tempdata%10];
    tempdata /= 10;
    led_buff[1] = numble_tab[tempdata%10];
    tempdata /= 10;    
    led_buff[0] = numble_tab[tempdata];
  } 
}
/***********************************************************
**Function name   :segment_displayhex(uint16_t tempdata)
**
**Input parameter : uint16_t tempdata
**
**Input parameter : null
**
**Description     :Displays a HEX
***********************************************************/
void segment_displayhex(uint16_t tempdata)
{
    led_buff[3] = numble_tab[tempdata&0x000F];
    tempdata = tempdata>>4;
    led_buff[2] = numble_tab[tempdata&0x000F];
    tempdata = tempdata>>4;
    led_buff[1] = numble_tab[tempdata&0x000F];
    tempdata = tempdata>>4;   
    led_buff[0] = numble_tab[tempdata&0x000F];
}