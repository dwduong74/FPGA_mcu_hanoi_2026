#include "key.h"
#include "led.h"
//		CONST
//============================================================================================================
#define	keyout0		P44																						//按键接口引脚定义
#define	keyout1		P45
#define	keyout2		P46
#define	keyout3		P47

#define	keyin0		P24
#define	keyin1		P25
#define	keyin2		P26
#define	keyin3		P27

#define	delaytime							10																//消抖时间10*10ms = 100ms
#define	keyfifo_length				8																	//FIFO长度
//============================================================================================================
//		DATA
//============================================================================================================
uint8_t	keyinbuf1,keyinbuf2;																		//按键扫描变量
uint8_t	keychkbuf1,keychkbuf2;																	//去抖时的中间变量
uint8_t	keycvtbuf1,keycvtbuf2;																	//键盘的操作信息
uint8_t	keyoldbuf1,keyoldbuf2;																	//旧键值存储
uint8_t	skeychat;																								//按键去抖计时器
uint8_t	rkeyval;																								//找出按键值中间变量
uint8_t	keyfifo[keyfifo_length];																//存储按键信息
uint8_t	keyfifo_pointer;																				//keyfifo写指针
bit			fkeyin;																									//键值发生变化标志
//============================================================================================================
//		Function
//============================================================================================================
/**************************************************************************************************************
*	Name 		:	Mnkey
* Function:	检测按键状态
**************************************************************************************************************/

void	Mnkey()
{
//=============================================================================================================
//	keyin
//=============================================================================================================
//row1
  uint8_t i ;
  
  P2UR |= 0XF0;
  P2M  &= 0X0F;
  P4M  &= ~0xF0;	
	
	P4M |= 0x10;																										//将第一行设为输出模式
	keyout0 = 0;																									//第一行输出低电平
  for(i = 0;i< 8;i++);                                          //When switching the status, you need to wait for a while, otherwise the status will be incorrectly read
	keyinbuf1 = keyin3*8 | keyin2*4 | keyin1*2 | keyin0;					//读取键值，保存在keyingbuf1的低四位
	keyout0 = 1;																									//先拉高
  P4M &= ~0x10;	
	P4M |= 0x20;																										//再将第一行设为输入，第二行设为输出模式
	
//row2
	keyout1 = 0;							
  for(i = 0;i< 8;i++);  
	keyinbuf1 |= keyin3*128 | keyin2*64 | keyin1*32 | keyin0*16;	//读取键值，并保存在keyinbuf1的高四位，以此方式逐列扫描
	keyout1 = 1;
  P4M &= ~0x20;	
	P4M = 0x40;
	
//row3	
	keyout2 = 0;
  for(i = 0;i< 8;i++);
	keyinbuf2 = keyin3*8 | keyin2*4 | keyin1*2 | keyin0;
	keyout2 = 1;
  P4M &= ~0x40;
	P4M |= 0x80;
	
//row4	

	keyout3 = 0;
  for(i = 0;i< 8;i++);
	keyinbuf2 |= keyin3*128 | keyin2*64 | keyin1*32 | keyin0*16;
	keyout3 = 1;
  
	P4M &= ~0x80;
	
	keyinbuf1 = ~ keyinbuf1;																			//取反，取正逻辑
	keyinbuf2 = ~ keyinbuf2;
	
//=============================================================================================================
//	keychk
//=============================================================================================================
	if ((keyinbuf1 == keychkbuf1) && (keyinbuf2 == keychkbuf2))		//判断扫描值是否不变？
	{	
		if ((1 == fkeyin) && (0 == skeychat))												//fkeyin 为1表示有按键操作					
		{																														//skeychat 为0表示按键去抖计时完毕
				keycvtbuf1 = keychkbuf1;												
				keycvtbuf2 = keychkbuf2;
				fkeyin = 0;
		}
	}
	else
	{
		keychkbuf1 = keyinbuf1;																			//若两次按键值不同，则将上次键值更新
		keychkbuf2 = keyinbuf2;
		fkeyin = 1;
		skeychat = delaytime;																				//重赋消抖时间
	}	
	
  if(skeychat != 0)
    skeychat -- ;
//=============================================================================================================
//	keycvt
//=============================================================================================================
	if ((keycvtbuf1 != keyoldbuf1) | (keycvtbuf2 != keyoldbuf2))	//fkeycvtok 为1有按键信息要处理
	{
		uint8_t	i,j;
		if ((0 == keyoldbuf1) && (0 == keyoldbuf2))									//单键按下
		{
			if (keycvtbuf1)
			{
				keychkbuf1=keycvtbuf1;
			}			
			else
			{
				keychkbuf1=keycvtbuf2;			
			}	
			
			i = 9;
			j = 0x80;																
			while(-- i)																								//最多循环8次
			{
				if (j & keychkbuf1)																			//找出具体新键按下位置
				{				
          led_ctr(LED0,!LED0_PIN);
					i = 1;																								//找出键值就退出循环										
				}
				j >>= 1;																								//通过移位找出具体键值
			}		
		}
		keyoldbuf1 = keycvtbuf1;																		//将旧键值更新
		keyoldbuf2 = keycvtbuf2;
	}
}


/**************************************************************************************************************
*	Name 		:	Wr_FIFO	
* Function:	键值写入FIFO中
**************************************************************************************************************/
void	Wr_FIFO()
{
	keyfifo[keyfifo_pointer] = rkeyval;													//写入FIFO
	if (++ keyfifo_pointer >= keyfifo_length)										//写指针大于等于FIFO长度则对其清零
	{
		keyfifo_pointer = 0;
	}
  
}