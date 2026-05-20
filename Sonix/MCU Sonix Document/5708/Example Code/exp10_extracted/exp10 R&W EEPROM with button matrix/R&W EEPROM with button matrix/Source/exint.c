#include "exint.h"
#include "i2c.h"
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

uint8_t ep_addr;
uint8_t  ep_data;
uint16_t data_temp;
uint8_t data_read;
 uint8_t key_value;
uint8_t input_flag;  // input_flag == 0 input addr  ;input_flag == 1 input data  ;
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
  

  PEDGE = 0x02;
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
void key_value_app()
{
	if(key_value == 0) 
		return ;
	
  switch(key_value)
  {
		case 1:	data_temp = 0;break;  //clear 
		
    case 2: data_temp = data_temp *10+ 0; break;
    case 5: data_temp = data_temp *10+ 1; break;
    case 6: data_temp = data_temp *10+ 2; break;   
    case 7: data_temp = data_temp *10+ 3; break;
    case 9: data_temp = data_temp *10+ 4; break;
    case 10:data_temp = data_temp *10+ 5; break;
    case 11:data_temp = data_temp *10+ 6; break;
    case 13:data_temp = data_temp *10+ 7; break;   
    case 14:data_temp = data_temp *10+ 8; break;   //Pay attention to Data overflow (addr < 65536)
    case 15:data_temp = data_temp *10+ 9; break;   // Pay attention to address overflow
		
		case 8: input_flag = 0;				//input addr
						ep_addr = 0;
						data_temp = 0;
						break;
		
		case 4: input_flag = 1;				//input data
						ep_data = 0;
						data_temp = 0;
						break;		
    case 16:   
            EP_WriteByte(0xA0,ep_addr,ep_data);//write data to addr
						segment_display(0);			
            break;  
    case 12:   
            data_read= EP_RdRead(0xA0,ep_addr);//read data from addr
						segment_display(data_read);						
          break;  
  }
  
	if(key_value != 12 && key_value!= 16)
	{
		if(input_flag == 1)
		{
			if(data_temp > 255)
				data_temp = 255;
			
			ep_data = data_temp;
		}
		else
		{
			if(data_temp > 9999)
				data_temp = 9999;			
			ep_addr = data_temp;
		}
		segment_display(data_temp);
	}
	
  PEDGE = 0X02;	
	key_value = 0;

}