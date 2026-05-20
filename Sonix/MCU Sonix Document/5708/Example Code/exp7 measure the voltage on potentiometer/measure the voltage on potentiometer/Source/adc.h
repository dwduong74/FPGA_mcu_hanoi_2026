#ifndef _ADC_
#define _ADC_
#include "mcu.h"
#define	L_AD_CHANNEL1			0x01				//AIN1	P41								
#define	L_AD_CHANNEL2			0x02				//AIN2	P42
#define	L_AD_CHANNEL3			0x1a				//AIN14	内部参考电压检测通道,可以反向求出VDD的大小

#define L_AD_SPEEDDIV16		0x00				//fosc/16
#define L_AD_SPEEDDIV8		0x10				//fosc/8
#define L_AD_SPEEDDIV1		0x20				//fosc/1
#define L_AD_SPEEDDIV2		0x30				//fosc/2

#define	L_VREF_INT				0x00				//内部参考,P54为GPIO
#define	L_VREF_EXT				0X80				//外部参考,FROM P54

#define	L_VERFH_2V				0x00				//2V
#define	L_VERFH_3V				0x01				//3V
#define	L_VERFH_4V				0x02				//4V
#define	L_VERFH_VDD				0x03				//VDD
#define	L_VERFH_INTVDD		0x04				//内部vdd输入检测				

#define	L_AD_SAMPLE_MAX			8					//采样8次，这个参数不用改动，防止AD值累加平均出错
#define	L_AD_SHIFT					3					//右移3次平均
void adc_config(void);
uint16_t ADAverge(uint16_t *ad_array);
uint16_t GetAD(uint8_t	ad_channel);
#endif