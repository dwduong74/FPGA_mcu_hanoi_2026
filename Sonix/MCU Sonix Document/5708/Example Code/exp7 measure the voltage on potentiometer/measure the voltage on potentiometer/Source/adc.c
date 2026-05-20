#include "adc.h"
/***********************************************************
**Function name   :adc_config
**
**Input parameter :NULL
**
**Input parameter : NULL
**
**Description     : Configure the ADC and corresponding I/O ports
***********************************************************/
void adc_config()
{

 	ADM = 0x80|L_AD_CHANNEL1;	                        //ADC Enable
	ADR = 0x40;																				//AIN channel Enable
	VREFH = L_VREF_INT;							
	VREFH |= L_VERFH_3V;                          	
	P4CON |= 0x02;	                                  //P41 is configured as an analog port
}
/***********************************************************
**Function name   :uint16_t GetAD(uint8_t	ad_channel)
**
**Input parameter :adc channel   
**
**Input parameter : ADC value
**
**Description     : Collects the value of an adc channel
***********************************************************/
uint16_t GetAD(uint8_t	ad_channel)
{ 	
	uint8_t i = 0;
	uint16_t j = 0;
	uint16_t temp_ad_max = 0;
	uint16_t temp_ad_min = 0xffff;
	uint16_t temp_ad_buf = 0;
	uint16_t temp_ad_data = 0;
	
	ADM = (ADM&0XE0)|ad_channel;
	
	for(i = 0; i < L_AD_SAMPLE_MAX + 2; i++)				
	{
		ADM |= 0x40;													//Start ADC
		while((ADM & 0x20) != 0X20);					//EOC   				
		ADM &= 0xdf;
		
		temp_ad_buf = ADB;
		temp_ad_buf = (temp_ad_buf << 4) + (ADR & 0X0F);
		temp_ad_data += temp_ad_buf; 
		
		if(temp_ad_buf < temp_ad_min)
		{
			temp_ad_min = temp_ad_buf;
		}
		if(temp_ad_buf > temp_ad_max)
		{
			temp_ad_max = temp_ad_buf;
		}	
	}
	
	temp_ad_data = (temp_ad_data - temp_ad_min - temp_ad_max) >> L_AD_SHIFT;					
	return	temp_ad_data;
}