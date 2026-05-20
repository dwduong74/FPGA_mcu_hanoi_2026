/******************** (C) COPYRIGHT 2024 SONiX *******************************
* COMPANY:			SONiX
* DATE:					2024/11
* AUTHOR:				SA1
* IC:						SN32F400
* DESCRIPTION:	I2C0 related functions.
*____________________________________________________________________________
* REVISION	Date				User		Description
* 1.0				2024/11/27	SA1			1. First release
*
*____________________________________________________________________________
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS TIME TO MARKET.
* SONiX SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL 
* DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT OF SUCH SOFTWARE
* AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN 
* IN CONNECTION WITH THEIR PRODUCTS.
*****************************************************************************/

/*_____ I N C L U D E S ____________________________________________________*/
#include <SN32F400_Def.h>
#include "I2C.h"
#include ".\..\SysTick\SysTick.h"
#include "..\..\Utility\Utility.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
uint8_t bI2C0_RxFIFO[I2C_RX_FIFO_LENGTH];
uint8_t bI2C0_TxFIFO[I2C_TX_FIFO_LENGTH] = {0};
uint8_t bI2C0_RxFIFO_cnts, bI2C_Rx_cnts;
uint8_t bI2C0_TxFIFO_cnts, bI2C_Tx_cnts;
uint8_t *bTX_ptr;
uint8_t	EEPROM_ADR_H, EEPROM_ADR_L;
uint8_t	EEPROM_WR;				// 0 : write
													// 1 : read
uint8_t Busy = 1, Error = 0;
uint8_t Read_Down = 0;
uint8_t Send_Address;
uint8_t temp = 0;
volatile uint8_t Timeout = 0;

typedef enum{
	eI2C0_ISR_NORMAL,
	eI2C0_ISR_MASTER_TX,
	eI2C0_ISR_MASTER_RX,
	eI2C0_ISR_SLAVE_TX,
	eI2C0_ISR_SLAVE_RX,
}I2C0_ISR_Selection_e;

static uint8_t _b_I2C0_DMA_SlvAddr;
static I2C0_DMA_Status_e		_eI2C0_DMA_STATUS = eI2C_DMA_IDLE;
static I2C0_ISR_Selection_e _eI2C0_ISR_SEL = eI2C0_ISR_NORMAL;

static void __i2c0_irq_handler_normal(void);
static void __i2c0_irq_handler_dma_mst_tx(void);
static void __i2c0_irq_handler_dma_mst_rx(void);
static void __i2c0_irq_handler_dma_slv_tx(void);
static void __i2c0_irq_handler_dma_slv_rx(void);

static void (*__fptrI2C0_irq_handler[])(void) = {
			__i2c0_irq_handler_normal,  // eI2C0_ISR_NORMAL
	__i2c0_irq_handler_dma_mst_tx,  // eI2C0_ISR_MASTER_TX
	__i2c0_irq_handler_dma_mst_rx,  // eI2C0_ISR_MASTER_RX
	__i2c0_irq_handler_dma_slv_tx,  // eI2C0_ISR_SLAVE_TX
	__i2c0_irq_handler_dma_slv_rx,  // eI2C0_ISR_SLAVE_RX
};
/*_____ D E F I N I T I O N S ______________________________________________*/

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

/*****************************************************************************
* Function		: I2C_IRQHandler
* Description	: I2C interrupt service routine
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void I2C0_IRQHandler(void)
{		
	__fptrI2C0_irq_handler[_eI2C0_ISR_SEL]();
}

/*****************************************************************************
* Function		: __i2c0_irq_handler_normal
* Description	: I2C interrupt service routine in normal CPU mode
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static void __i2c0_irq_handler_normal(void)
{		
	if (((SN_I2C0->STAT) & (mskI2C_LOST_ARB_LOST_ARBITRATION)) == Lost_Arbitration)
	{
		SN_I2C0->STAT_b.I2CIF = 1;		
		SN_I2C0->CTRL_b.I2CEN = 0;			//Disable I2C				
		SN_I2C0->CTRL_b.I2CEN = 1;			//Enable I2C		
		I2C0_Start();										//Re-start				
	}
	
	/* Stop Done */
	else if (((SN_I2C0->STAT) & (mskI2C_STA_MASTER_STA_STO)) == STOP_DONE)
	{	
		Busy = 0;	
		SN_I2C0->STAT_b.I2CIF = 1;			
		if (EEPROM_WR == 1)
		{
			Read_Down = 1;
		}
	}	
	else
	{	
		SN_I2C0->STAT_b.I2CIF = 1;	

		switch (SN_I2C0->STAT)
		{			
			case (Lost_Arbitration | mskI2C_MST_MASTER):
				I2C0_Start();	
			break;
			
			/* RX with ACK/NACK transfer is down */	
			case (RX_DONE | mskI2C_MST_MASTER):
				bI2C0_RxFIFO[bI2C_Rx_cnts++] = SN_I2C0->RXDATA;					
				if (bI2C_Rx_cnts < (bI2C0_RxFIFO_cnts - 1))
				{
					SN_I2C0->CTRL_b.ACK	=	1;
				}
				else if (bI2C_Rx_cnts == (bI2C0_RxFIFO_cnts - 1))
				{
					SN_I2C0->CTRL_b.NACK = 1;			
				}					
				else if (bI2C_Rx_cnts == bI2C0_RxFIFO_cnts)
				{
					I2C0_Stop(); 
				}
				Busy = 0;				
			break;
				
			/* SLA+W or Data has been transmitted and ACK has been received */
			case (ACK_DONE | mskI2C_MST_MASTER):
				if (EEPROM_WR == 1)
				{
					Busy = 0;					
					if(bI2C0_RxFIFO_cnts == 1)
					{
						SN_I2C0->CTRL_b.NACK = 1;
					}						
					else
					{
						SN_I2C0->CTRL_b.ACK = 1;
					}
				}			
				if (EEPROM_WR == 0)
				{
					if (Send_Address == 0) 
					{
						bI2C_Tx_cnts++;
						if (bI2C_Tx_cnts <  bI2C0_TxFIFO_cnts)
						{
							SN_I2C0->TXDATA = *bTX_ptr++;
						}
						else if (bI2C_Tx_cnts == bI2C0_TxFIFO_cnts)
						{
							Busy = 0;
						}							
					}
					else
						Busy = 0;
				}
			break;
			
			/* SLA+W or Data has been transmitted and NACK has been received */	
			case (NACK_DONE | mskI2C_MST_MASTER):
				SN_I2C0->CTRL_b.STO = 1;
				Error  = 1;
			break;
			
			/* START has been transmitted and prepare SLA+W/SLA+R */	
			case (START_DONE | mskI2C_MST_MASTER):
				#if (EEPROM_less_than_32K == 1)
					SN_I2C0->TXDATA = Device_ADDRESS | (EEPROM_ADR_H << 1) | EEPROM_WR;
				#else
					SN_I2C0->TXDATA = Device_ADDRESS | EEPROM_WR;
				#endif
			break;
			
			default:
				SN_I2C0->CTRL_b.I2CEN = 0;
				SN_I2C0->CTRL_b.I2CEN = 1;
				SN_I2C0->CTRL_b.STA = 1;
			break;
		}
	}	
}

/*****************************************************************************
* Function		: I2C0_Init
* Description	: Set specified value to specified bits of assigned register
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void I2C0_Init(void)
{
	//I2C0 interrupt enable
	NVIC_ClearPendingIRQ(I2C0_IRQn);	
	NVIC_EnableIRQ(I2C0_IRQn);
	NVIC_SetPriority(I2C0_IRQn, 0);

	//Enable HCLK for I2C0
	SN_SYS1->AHBCLKEN_b.I2C0CLKEN = 1;						//Enable clock for I2C0

	//I2C0 speed
	SN_I2C0->SCLHT = I2C0_SCLHT;
	SN_I2C0->SCLLT = I2C0_SCLLT;
	
	//I2C0 enable
	SN_I2C0->CTRL_b.I2CEN = I2C_I2CEN_EN;
}

/*****************************************************************************
* Function		: I2C0_Start
* Description	: transmit a START bit
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void I2C0_Start(void)
{
	SN_I2C0->CTRL_b.STA = 1;
}

/*****************************************************************************
* Function		: I2C0_Stop
* Description	: transmit a STOP condition in master mode
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void I2C0_Stop(void)
{
	SN_I2C0->CTRL_b.STO = 1;
}

/*****************************************************************************
* Function		: I2C0_Read
* Description	: read N Byte data from EEPROM
* Input			: eeprom_adr - data word address
*						: read_num - byte length
* Output		: None
* Return		: OK or Fail  
* Note			: None
*****************************************************************************/
uint8_t I2C0_Read(uint16_t eeprom_adr, uint8_t read_num)
{	
	bI2C_Rx_cnts = 0;
		
	EEPROM_ADR_H =  eeprom_adr >> 8;				//data word address low byte
	EEPROM_ADR_L =  eeprom_adr & 0x00ff;		//data word address high byte
	
	bI2C0_RxFIFO_cnts = read_num;						//byte length 					

	Busy = 1;
		
	EEPROM_WR = 0;													//write
	
	I2C0_Start();														//I2C start

	SysTick->CTRL = 0x7;										//Enable SysTick timer and interrupt	
	
	Send_Address = 1;												//data word address setting flag 
	
	while (Busy == 1 && Timeout == 0);
	
	SysTick->CTRL = 0x0;										//Disable SysTick timer and interrupt	
	
	if (Error == 1 || Timeout == 1) 
	{
		return FALSE;
	}
		
	#if (EEPROM_less_than_32K == 0)	
		
		SN_I2C0->TXDATA = EEPROM_ADR_H;				//data word address high byte

		SysTick->CTRL = 0x7;									//Enable SysTick timer and interrupt	
	
		Busy = 1;
		
		while (Busy == 1 && Timeout == 0);
		
		SysTick->CTRL = 0x0;									//Disable SysTick timer and interrupt	

		if (Error == 1 || Timeout == 1)
		{
			return FALSE;
		}

	#endif
	
	SN_I2C0->TXDATA = EEPROM_ADR_L;					//data word address low byte
	
	SysTick->CTRL = 0x7;										//Enable SysTick timer and interrupt	
			
	Busy = 1;
	
	while (Busy == 1 && Timeout == 0);
		
	SysTick->CTRL = 0x0;										//Disable SysTick timer and interrupt	
	
	if (Error == 1 || Timeout == 1)
	{
		return FALSE;	
	}

	Read_Down = 0;													  
	
	Send_Address = 0;
			
	EEPROM_WR = 1;													//read		
	
	I2C0_Start();														//I2C start
	
	SysTick->CTRL = 0x7;										//Enable SysTick timer and interrupt	
	
	while (Read_Down == 0 && Timeout == 0);

	SysTick->CTRL = 0x0;										//Disable SysTick timer and interrupt	
	
	Read_Down = 0;	
	
	if (Error == 1 || Timeout == 1)
	{
		return FALSE;
	}
	
	return TRUE;
}

/*****************************************************************************
* Function		: I2C0_Write
* Description	: write N Byte data to EEPROM
* Input			: eeprom_adr - data word address
*						: write_num - byte length
* Output		: None
* Return		: OK or Fail  
* Note			: None
*****************************************************************************/
uint8_t I2C0_Write(uint16_t eeprom_adr, uint8_t write_num)
{
	Timeout = 0;
	
	bI2C_Tx_cnts = 0;
	
	bTX_ptr = &bI2C0_TxFIFO[0];							//write data buffer	
	
	EEPROM_ADR_H =  eeprom_adr >> 8;				//data word address high byte
	
	EEPROM_ADR_L =  eeprom_adr & 0x00ff;		//data word address low byte
	
	bI2C0_TxFIFO_cnts = write_num;						//byte length	
		
	Busy = 1;
	
	EEPROM_WR = 0;													//write
	
	SysTick->CTRL = 0x7;										//Enable SysTick timer and interrupt
	
	I2C0_Start();														//I2C start	

	Send_Address = 1;												//data word address setting flag 
	
	while (Busy == 1 && Timeout == 0);
		
	SysTick->CTRL = 0x0;										//Disable SysTick timer and interrupt
	
	if (Error == 1 || Timeout == 1) 
	{
		return FALSE;
	}

	#if (EEPROM_less_than_32K == 0)

		SN_I2C0->TXDATA = EEPROM_ADR_H;					//data word address high byte
		
		SysTick->CTRL = 0x7;										//Enable SysTick timer and interrupt	
		
		Busy = 1;

		while (Busy == 1 && Timeout == 0);
			
		SysTick->CTRL = 0x0;										//Disable SysTick timer and interrupt
		
		if (Error == 1 || Timeout == 1)
		{
			return FALSE;
		}

	#endif
	
	SN_I2C0->TXDATA = EEPROM_ADR_L;					//data word address low byte
	
	SysTick->CTRL = 0x7;										//Enable SysTick timer and interrupt

	Busy = 1;	

	while (Busy == 1 && Timeout == 0);

	if (Error == 1 || Timeout == 1 )
	{
		return FALSE;
	}
	
	SysTick->CTRL = 0x0;										//Disable SysTick timer and interrupt	
	
	Send_Address = 0;
	
	SN_I2C0->TXDATA = *bTX_ptr++;						//write data
	
	SysTick->CTRL = 0x7;										//Enable SysTick timer and interrupt
	
	Busy = 1;	

	while (Busy == 1 && Timeout == 0);
	
	if (Error == 1 || Timeout == 1)
	{
		return FALSE;
	}
	
	SysTick->CTRL = 0x0;									//Disable SysTick timer and interrupt	
	
	Busy = 1;	
	
	I2C0_Stop();
	
	SysTick->CTRL = 0x7;										//Enable SysTick timer and interrupt
	
	while (Busy == 1 && Timeout == 0);
	
	if (Error == 1 || Timeout == 1)
	{
		return FALSE;
	}
	
	SysTick->CTRL = 0x0;										//Disable SysTick timer and interrupt		
	
	return TRUE;
}

/*****************************************************************************
* Function		: I2C0_SlaveAddressSet
* Description	: Set I2C0 slave address register
* Input			: b_AddMode - Slave address mode = 7bit(0x00) or 10bit(0x01)
* 					: b_GCEN - General call address = disable(0x00) or enable(0x01) 
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void I2C0_SlaveAddressSet(uint8_t b_AddMode, uint8_t b_GCEN)
{
	uint8_t b_addr_shift;
	SN_I2C0->SLVADDR0_b.ADD_MODE = b_AddMode;
	SN_I2C0->SLVADDR0_b.GCEN = b_GCEN;
	
	// ADD[9:0] is valid when ADD_MODE = 1
	// ADD[7:1] is valid when ADD_MODE = 0
	b_addr_shift = (b_AddMode == I2C_ADD_MODE_7BIT)? 1 : 0;
	SN_I2C0->SLVADDR0_b.ADDR 	= I2C_ADDR_SLAVE_ADDR0 << b_addr_shift;
	SN_I2C0->SLVADDR1_b.ADDR1 = I2C_ADDR_SLAVE_ADDR1 << b_addr_shift;
	SN_I2C0->SLVADDR1_b.ADDR2 = I2C_ADDR_SLAVE_ADDR2 << b_addr_shift;
	SN_I2C0->SLVADDR1_b.ADDR3 = I2C_ADDR_SLAVE_ADDR3 << b_addr_shift;
	SN_I2C0->SLVADDR2_b.ADDR4 = I2C_ADDR_SLAVE_ADDR4 << b_addr_shift;
	SN_I2C0->SLVADDR2_b.ADDR5 = I2C_ADDR_SLAVE_ADDR5 << b_addr_shift;
	SN_I2C0->SLVADDR2_b.ADDR6 = I2C_ADDR_SLAVE_ADDR6 << b_addr_shift;
	SN_I2C0->SLVADDR3_b.ADDR7 = I2C_ADDR_SLAVE_ADDR7 << b_addr_shift;
	SN_I2C0->SLVADDR3_b.ADDR8 = I2C_ADDR_SLAVE_ADDR8 << b_addr_shift;
	SN_I2C0->SLVADDR3_b.ADDR9 = I2C_ADDR_SLAVE_ADDR9 << b_addr_shift;
}

/*****************************************************************************
* Function		: I2C0_Get_DMA_Status
* Description	: Get I2C0 DMA status
* Input			: None
* Output		: None
* Return		: I2C0_DMA_Status_e - status of I2C0 DMA 
* Note			: None
*****************************************************************************/
uint32_t I2C0_Get_DMA_Status(void)
{
	return _eI2C0_DMA_STATUS;
}

/*****************************************************************************
* Function		: I2C0_DMA_Master_TX_Start
* Description	: Send N bytes as I2C master
* Input			: b_SlvAddr - target slave address (7bit address)
* 					: w_Size - DMA Tx data length (unit:byte)
* Output		: None
* Return		: None 
* Note			: None
*****************************************************************************/
void I2C0_DMA_Master_TX_Start(uint8_t b_SlvAddr, uint32_t w_Size)
{
	_b_I2C0_DMA_SlvAddr = b_SlvAddr;
	SN_I2C0->DMA = mskI2C_TX_DMA_DIS | mskI2C_RX_DMA_DIS | w_Size;
	
	_eI2C0_ISR_SEL = eI2C0_ISR_MASTER_TX;
	_eI2C0_DMA_STATUS = eI2C_DMA_START;
	SN_I2C0->CTRL_b.STA = 1;
}

/*****************************************************************************
* Function		: I2C0_DMA_Master_RX_Start
* Description	: Receive N bytes as I2C master
* Input			: b_SlvAddr - target slave address (7bit address)
* 					: w_Size - DMA Rx data length (unit:byte)
* 					: b_Last_NACK - I2C issues ACK(0x00) or NACK(0x01) for last data
* Output		: None
* Return		: None 
* Note			: None
*****************************************************************************/
void I2C0_DMA_Master_RX_Start(uint8_t b_SlvAddr, uint32_t w_Size, uint8_t b_Last_NACK)
{
	_b_I2C0_DMA_SlvAddr = b_SlvAddr;
	SN_I2C0->DMA = mskI2C_TX_DMA_DIS | mskI2C_RX_DMA_DIS | w_Size;
	SN_I2C0->DMA_b.RX_DMA_LAST_NACK = b_Last_NACK;
	
	_eI2C0_ISR_SEL = eI2C0_ISR_MASTER_RX;
	_eI2C0_DMA_STATUS = eI2C_DMA_START;
	SN_I2C0->CTRL_b.STA = 1;
}

/*****************************************************************************
* Function		: I2C0_DMA_Slave_TX_Start
* Description	: Send N bytes as I2C slave
* Input			: w_Size - DMA Tx data length (unit:byte)
* Output		: None
* Return		: None 
* Note			: None
*****************************************************************************/
void I2C0_DMA_Slave_TX_Start(uint32_t w_Size)
{
	SN_I2C0->DMA = mskI2C_TX_DMA_DIS | mskI2C_RX_DMA_DIS | w_Size;
	_eI2C0_ISR_SEL = eI2C0_ISR_SLAVE_TX;
	_eI2C0_DMA_STATUS = eI2C_DMA_START;
}

/*****************************************************************************
* Function		: I2C0_DMA_Slave_RX_Start
* Description	: Receive N bytes as I2C slave
* Input			: w_Size - DMA Rx data length (unit:byte)
* 					: b_Last_NACK - I2C issues ACK(0x00) or NACK(0x01) for last data
* Output		: None
* Return		: None 
* Note			: None
*****************************************************************************/
void I2C0_DMA_Slave_RX_Start(uint32_t w_Size, uint8_t b_Last_NACK)
{
	SN_I2C0->DMA = mskI2C_TX_DMA_DIS | mskI2C_RX_DMA_DIS | w_Size;
	SN_I2C0->DMA_b.RX_DMA_LAST_NACK = b_Last_NACK;
	
	_eI2C0_ISR_SEL = eI2C0_ISR_SLAVE_RX;
	_eI2C0_DMA_STATUS = eI2C_DMA_START;
}

/*****************************************************************************
* Function		: __i2c0_irq_handler_dma_mst_tx
* Description	: I2C interrupt service routine in I2C DMA master TX mode
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static void __i2c0_irq_handler_dma_mst_tx(void)
{
	if((SN_I2C0->STAT & mskI2C_START_DN_START) == mskI2C_START_DN_START)
	{
		// START : Transmit Slave address + Write
		SN_I2C0->STAT_b.I2CIF = 1;
		SN_I2C0->TXDATA = (_b_I2C0_DMA_SlvAddr << 0x01);				
	}
	else if((SN_I2C0->STAT & mskI2C_ACK_STAT_RECEIVED_ACK) == mskI2C_ACK_STAT_RECEIVED_ACK)
	{
		SN_I2C0->STAT_b.I2CIF = 1;	
		if(_eI2C0_DMA_STATUS == eI2C_DMA_START)
		{
			// ACK : Receive ACK after transmit slave address and enable DMA function
			SN_I2C0->DMA_b.TX_DMA_EN = ENABLE;	// Enable DMA
			_eI2C0_DMA_STATUS = eI2C_DMA_WORKING;
		}
		else
		{
			// ACK : Receive ACK after DMA Tx done, transmit STOP
			SN_I2C0->DMA_b.TX_DMA_EN = DISABLE;
			SN_I2C0->CTRL_b.STO = 1;
		}
	}
	else if((SN_I2C0->STAT & mskI2C_NACK_STAT_RECEIVED_NACK) == mskI2C_NACK_STAT_RECEIVED_NACK)
	{
		// NACK : Receive NACK after DMA Tx done, transmit STOP
		SN_I2C0->STAT_b.I2CIF = 1;
		SN_I2C0->DMA_b.TX_DMA_EN = DISABLE;
		SN_I2C0->CTRL_b.STO = 1;
		// if DMA size != 0, abort DMA transfer
		if(SN_I2C0->DMA_b.DMA_SIZE != 0)
			_eI2C0_DMA_STATUS = eI2C_DMA_FAIL_RECEIVE_NACK;
	}
	else if((SN_I2C0->STAT & mskI2C_STOP_DN_STOP) == mskI2C_STOP_DN_STOP)
	{
		SN_I2C0->STAT_b.I2CIF = 1;
		if(SN_I2C0->DMA_b.DMA_SIZE == 0)
			_eI2C0_DMA_STATUS = eI2C_DMA_DONE;
	}
	else if((SN_I2C0->STAT & mskI2C_LOST_ARB_LOST_ARBITRATION) == mskI2C_LOST_ARB_LOST_ARBITRATION)
	{
		SN_I2C0->STAT_b.I2CIF = 1;
		_eI2C0_DMA_STATUS = eI2C_DMA_FAIL_LOST_ARBITRATION;
	}
	else
	{
		SN_I2C0->STAT_b.I2CIF = 1;
		_eI2C0_DMA_STATUS = eI2C_DMA_FAIL_OTHERS;
	}
}

/*****************************************************************************
* Function		: __i2c0_irq_handler_dma_mst_rx
* Description	: I2C interrupt service routine in I2C DMA master RX mode
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static void __i2c0_irq_handler_dma_mst_rx(void)
{
	if((SN_I2C0->STAT & mskI2C_START_DN_START) == mskI2C_START_DN_START)
	{
		// START : Transmit Slave address + Read
		SN_I2C0->STAT_b.I2CIF = 1;
		SN_I2C0->TXDATA = (_b_I2C0_DMA_SlvAddr << 0x01) | 0x01;		
	}
	else if((SN_I2C0->STAT & mskI2C_ACK_STAT_RECEIVED_ACK) == mskI2C_ACK_STAT_RECEIVED_ACK)
	{
		// ACK : Receive ACK after transmit slave address, set ACK bit release bus and enable DMA function
		SN_I2C0->STAT_b.I2CIF = 1;
		SN_I2C0->CTRL_b.ACK = 1;
		if(_eI2C0_DMA_STATUS == eI2C_DMA_START)
		{
			SN_I2C0->DMA_b.RX_DMA_EN = ENABLE;
			_eI2C0_DMA_STATUS = eI2C_DMA_WORKING;
		}
		else
		{
			SN_I2C0->DMA_b.RX_DMA_EN = DISABLE;
			SN_I2C0->CTRL_b.STO = 1;
		}
	}
	else if((SN_I2C0->STAT & mskI2C_RX_DN_HANDSHAKE) == mskI2C_RX_DN_HANDSHAKE)
	{
		// RX_DN : Check DMA transfer done, DMA will return ACK / NACK to bus automatically and transmit STOP
		SN_I2C0->STAT_b.I2CIF = 1;
		SN_I2C0->DMA_b.RX_DMA_EN = DISABLE;
		SN_I2C0->CTRL_b.STO = 1;
	}
	else if((SN_I2C0->STAT & mskI2C_NACK_STAT_RECEIVED_NACK) == mskI2C_NACK_STAT_RECEIVED_NACK)
	{
		// NACK : issue stop
		SN_I2C0->STAT_b.I2CIF = 1;
		SN_I2C0->CTRL_b.STO = 1;
		_eI2C0_DMA_STATUS = eI2C_DMA_FAIL_RECEIVE_NACK;
	}
	else if((SN_I2C0->STAT & mskI2C_STOP_DN_STOP) == mskI2C_STOP_DN_STOP)
	{
		// STOP : I2C STOP condition was issued
		SN_I2C0->STAT_b.I2CIF = 1;
		if(SN_I2C0->DMA_b.DMA_SIZE == 0)
			_eI2C0_DMA_STATUS = eI2C_DMA_DONE;
	}
	else if((SN_I2C0->STAT & mskI2C_LOST_ARB_LOST_ARBITRATION) == mskI2C_LOST_ARB_LOST_ARBITRATION)
	{
		SN_I2C0->STAT_b.I2CIF = 1;
		_eI2C0_DMA_STATUS = eI2C_DMA_FAIL_LOST_ARBITRATION;
	}
	else
	{
		SN_I2C0->STAT_b.I2CIF = 1;
		_eI2C0_DMA_STATUS = eI2C_DMA_FAIL_OTHERS;
	}
}

/*****************************************************************************
* Function		: __i2c0_irq_handler_dma_slv_tx
* Description	: I2C interrupt service routine in I2C DMA slave TX mode
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static void __i2c0_irq_handler_dma_slv_tx(void)
{
	if ((SN_I2C0->STAT & mskI2C_SLV_TX_MATCH_ADDR) == mskI2C_SLV_TX_MATCH_ADDR)
	{
		SN_I2C0->STAT_b.I2CIF = 1;
		if(_eI2C0_DMA_STATUS == eI2C_DMA_START)
		{
			// SLV_TX_HIT : Receive Slave address + Write, enable DMA
			SN_I2C0->DMA_b.TX_DMA_EN = ENABLE;	// Enable DMA
			_eI2C0_DMA_STATUS = eI2C_DMA_WORKING;
		}
	}
	else if((SN_I2C0->STAT & mskI2C_ACK_STAT_RECEIVED_ACK) == mskI2C_ACK_STAT_RECEIVED_ACK)
	{
		// ACK : Receive ACK after DMA Tx done
		SN_I2C0->STAT_b.I2CIF = 1;	
		SN_I2C0->DMA_b.TX_DMA_EN = DISABLE;
		// For release SCL and SDA
		SN_I2C0->CTRL_b.ACK = 1;
	}
	else if ((SN_I2C0->STAT & mskI2C_NACK_STAT_RECEIVED_NACK) == mskI2C_NACK_STAT_RECEIVED_NACK)
	{
		// NACK : Receive NACK after DMA Tx done
		SN_I2C0->STAT_b.I2CIF = 1;
		SN_I2C0->DMA_b.TX_DMA_EN = DISABLE;
		// For release SCL and SDA
		SN_I2C0->CTRL_b.ACK = 1;
		// if DMA size != 0, abort DMA transfer
		if(SN_I2C0->DMA_b.DMA_SIZE != 0)
			_eI2C0_DMA_STATUS = eI2C_DMA_FAIL_RECEIVE_NACK;
	}
	else if ((SN_I2C0->STAT & mskI2C_STOP_DN_STOP) == mskI2C_STOP_DN_STOP)
	{
		// STOP
		// Receive STOP condition
		SN_I2C0->STAT_b.I2CIF = 1;
		if((SN_I2C0->DMA_b.DMA_SIZE == 0) && (_eI2C0_DMA_STATUS == eI2C_DMA_WORKING))
			_eI2C0_DMA_STATUS = eI2C_DMA_DONE;
		else
			_eI2C0_DMA_STATUS = eI2C_DMA_FAIL_OTHERS;
	}
	else
	{
		SN_I2C0->STAT_b.I2CIF = 1;
		_eI2C0_DMA_STATUS = eI2C_DMA_FAIL_OTHERS;
	}
}

/*****************************************************************************
* Function		: __i2c0_irq_handler_dma_slv_rx
* Description	: I2C interrupt service routine in I2C DMA slave RX mode
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static void __i2c0_irq_handler_dma_slv_rx(void)
{
	if((SN_I2C0->STAT & mskI2C_SLV_RX_MATCH_ADDR) == mskI2C_SLV_RX_MATCH_ADDR)
	{
		// SLV_RX_HIT : Receive Slave address + Read, enable DMA
		SN_I2C0->STAT_b.I2CIF = 1;
		if(_eI2C0_DMA_STATUS == eI2C_DMA_START)
		{
			SN_I2C0->DMA_b.RX_DMA_EN = ENABLE;
			_eI2C0_DMA_STATUS = eI2C_DMA_WORKING;
		}
	}
	else if((SN_I2C0->STAT & mskI2C_RX_DN_HANDSHAKE) == mskI2C_RX_DN_HANDSHAKE)
	{
		// RX_DN : All DMA is done, I2C will return ACK / NACK to bus automatically
		SN_I2C0->STAT_b.I2CIF = 1;
		SN_I2C0->CTRL_b.ACK = 1; 
		SN_I2C0->DMA_b.RX_DMA_EN = DISABLE;
	}
	else if((SN_I2C0->STAT & mskI2C_STOP_DN_STOP) == mskI2C_STOP_DN_STOP)
	{
		// STOP : Receive STOP
		SN_I2C0->STAT_b.I2CIF = 1;
		SN_I2C0->DMA_b.RX_DMA_EN = DISABLE;
		if((SN_I2C0->DMA_b.DMA_SIZE == 0) && (_eI2C0_DMA_STATUS == eI2C_DMA_WORKING))
		{
			_eI2C0_DMA_STATUS = eI2C_DMA_DONE;
		}
		else
		{
			_eI2C0_DMA_STATUS = eI2C_DMA_FAIL_OTHERS;
		}
	}
	else if((SN_I2C0->STAT & mskI2C_LOST_ARB_LOST_ARBITRATION) == mskI2C_LOST_ARB_LOST_ARBITRATION)
	{
		SN_I2C0->STAT_b.I2CIF = 1;
		_eI2C0_DMA_STATUS = eI2C_DMA_FAIL_LOST_ARBITRATION; 
	}
	else
	{
		SN_I2C0->STAT_b.I2CIF = 1;
		_eI2C0_DMA_STATUS = eI2C_DMA_FAIL_OTHERS; 
	}
}
