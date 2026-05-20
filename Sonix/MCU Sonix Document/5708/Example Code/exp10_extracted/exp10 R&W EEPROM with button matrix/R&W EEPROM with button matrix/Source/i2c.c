#include "i2c.h"
#define ERROR	0xff																		//error data
/**************************************************************************************************************
*	Name :		I2C_Init	
* Function:	初始化I2C
**************************************************************************************************************/
void I2C_Init(void)
{
	P1M &= 0x30;                  		
	I2CCON |= 0x40;              			
}
/**************************************************************************************************************
*	Name :		I2C_Start	
* Function:	发送start信号
**************************************************************************************************************/
void I2C_Start(void)
{
	I2CCON |= 0x20;   
	I2CCON &= 0xF7;  	
	while((I2CCON&0x08)!=0x08); 					
	I2CCON &= 0xDF;             					
}
/**************************************************************************************************************
*	Name :		I2C_Stop	
* Function:	发送stop信号
**************************************************************************************************************/
void I2C_Stop(void)
{
	I2CCON |= 0x10;         							
	I2CCON &= 0xF7;	
}
/**************************************************************************************************************
*	Name :		I2C_TxData	
* Function:	发送单字节数据
× Input：		需要发送的数据
**************************************************************************************************************/
void I2C_TxData(uint8_t	Data)
{
	if(I2CSTA == 0x08 || I2CSTA == 0x10 || I2CSTA == 0x18 || I2CSTA == 0x28) 									
	{
		I2CDAT = Data;
		I2CCON &= 0xF7;             			 
		while((I2CCON&0x08)!=0x08);       
	}
}
/**************************************************************************************************************
*	Name :		I2C_RdData	
* Function:	读取数据
× Input：		Chk：应答信号
× Output:   读取到的数据
**************************************************************************************************************/
uint8_t	I2C_RdData(bit	Chk)
{
	if(I2CSTA == 0x40 || I2CSTA == 0x50)								
	{
		if(Chk == 1)	I2CCON |= 0x04;         						//判断是否需要继续接收，回应ack或nack    			
		else	I2CCON &= 0xfb;   
		
		I2CCON &= 0xF7;             										
		while((I2CCON&0x08)!=0x08); 											//接收数据
		return I2CDAT;																		
	}
	return	ERROR;
}	
/**************************************************************************************************************
*	Name :		EP_MultiRead	
* Function:	多字节读取eeprom
* Input:		DevSel器件地址，ByteAddr通信地址，RxData存储数组，Datalength数据长度
**************************************************************************************************************/
void 	EP_MultiRead(uint8_t	DevSel,uint8_t ByteAddr,uint8_t	*RxData,uint8_t	Datalength)
{
	uint8_t	i;
	
	I2C_Start();
	I2C_TxData(DevSel);
	I2C_TxData(ByteAddr);																												//写地址	
																																				
	I2C_Start();
	I2C_TxData(DevSel|0x01);																										//读命令
	for(i = 0;i < Datalength - 1;i ++)
	{
		*(RxData + i) = I2C_RdData(ACK);																					
	}
	(*(RxData + Datalength - 1)) = I2C_RdData(NACK);														//接收数据，返回ack信号	
	I2C_Stop();
}

/**************************************************************************************************************
*	Name :		EP_MultiWrite	
* Function:	多字节写入
* Input:		DevSel器件地址，ByteAddr通信地址，TxData写入数组，Datalength数据长度
**************************************************************************************************************/
void	EP_MultiWrite(uint8_t	DevSel,uint8_t ByteAddr,uint8_t	*TxData,uint8_t	Datalength)
{
	uint8_t	i = 0;
	
	I2C_Start();
	I2C_TxData(DevSel);
	I2C_TxData(ByteAddr);																													//写地址	
	for(i = 0;i < Datalength;i ++)
	{
		I2C_TxData(*(TxData+i));																										//写数据
	}
	I2C_Stop();
}

/**************************************************************************************************************
*	Name :		EP_WriteByte	
* Function:	单字节写入
* Input:		DevSel器件地址，ByteAddr通信地址，DataIn数据
**************************************************************************************************************/
void EP_WriteByte(uint8_t	DevSel, uint8_t ByteAddr, uint8_t	DataIn)
{
	I2C_Start();
	I2C_TxData(DevSel);																														//写地址
	I2C_TxData(ByteAddr);
	I2C_TxData(DataIn);																														//写数据	
	I2C_Stop();
}

/**************************************************************************************************************
*	Name :		EP_RdRead	
* Function:	单字节写入
* Input:		DevSel器件地址，ByteAddr通信地址
* Output:   对应地址的数据
**************************************************************************************************************/
uint8_t EP_RdRead(uint8_t	DevSel, uint8_t	ByteAddr)
{
	uint8_t	temp = 0;
	
	I2C_Start();
	I2C_TxData(DevSel);
	I2C_TxData(ByteAddr);
	I2C_Start();
	I2C_TxData(DevSel|0x01);
	temp = I2C_RdData(NACK);
	I2C_Stop();
	return	temp;
}