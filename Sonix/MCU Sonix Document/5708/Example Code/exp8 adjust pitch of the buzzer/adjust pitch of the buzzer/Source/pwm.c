#include "mcu.h"
#include "pwm.h"


code uint16_t musical_tab[10]= {63866,64033,64200,64367,64534,64701,64848,65035,65202,65369};  //0~F


uint16_t t2_load_value;

/***********************************************************
**Function name   :
**
**Input parameter : 
**
**Input parameter :
**
**Description     :Configure io for display nixie tube
***********************************************************/
void buzzer_pwm_config()
{

  buzzer_fre_set(9);
  T2CON = 0x41;

  CCEN = 0X02;
  
  TH2 = t2_load_value>>8;
  TL2 = t2_load_value;
  ET2 = 1;
}
void T2COM0Interrupt(void) interrupt ISRTimer2 
{
  TH2 = t2_load_value>>8;
  TL2 = t2_load_value;
  TF2 = 0;
}
/***********************************************************
**Function name   :
**
**Input parameter : level 0-9
**
**Input parameter :
**
**Description     :buzzer Frequency setting
***********************************************************/
void buzzer_fre_set(uint8_t lv)
{

  t2_load_value = musical_tab[lv];
  CRCH =(t2_load_value +(65536-t2_load_value)/2) >> 8;
  CRCL =(t2_load_value +(65536-t2_load_value)/2);
}