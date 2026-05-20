/******************** (C) COPYRIGHT 2024 SONiX *******************************
* COMPANY:		SONiX
* DATE:				2024/11
* AUTHOR:			SA1
* IC:					SN32F400
*____________________________________________________________________________
*	REVISION	Date				User		Description
*	1.0				2024/11/27	SA1			1. First version released
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
#include <SN32F400.h>
#include <SN32F400_Def.h>
#include ".\Interface\DMA\DMA.h"
#include ".\Interface\I2C\I2C.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);

/*_____ D E F I N I T I O N S ______________________________________________*/
#ifndef	SN32F407					//Do NOT Remove or Modify!!!
	#error Please install SONiX.SN32F4_DFP.1.0.1.pack or version >= 1.0.1
#endif
#define	PKG						SN32F407				//User SHALL modify the package on demand (SN32F407)

// DMA transfer size 16 bytes
#define DMA_TEST_SIZE 0x10

uint8_t b_aryI2C0Data[DMA_TEST_SIZE];

void DMAI2C0MasterTX(void);
void DMAI2C0MasterRX(void);
void DMAI2C0SlaveTX(void);
void DMAI2C0SlaveRX(void);
/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/
	
/*****************************************************************************
* Function		: main
* Description	: Demo code of DMA I2C.
* Input			: None
* Output		: None
* Return		: None
* Note			: Connect I2C0 to the other device
*****************************************************************************/
int	main(void)
{
	//User can configure System Clock with Configuration Wizard in system_SN32F400.c
	SystemInit();
	SystemCoreClockUpdate();				//Must call for SN32F400, Please do NOT remove!!!

	//Note: User can refer to ClockSwitch sample code to switch various HCLK if needed.

	PFPA_Init();										//User shall set PFPA if used, do NOT remove!!!

	//1. User SHALL define PKG on demand.
	//2. User SHALL set the status of the GPIO which are NOT pin-out to input pull-up.
	NotPinOut_GPIO_init();

	//--------------------------------------------------------------------------
	//User Code starts HERE!!!
	
	// Initial I2C
	I2C0_Init();
	
	DMAI2C0MasterTX();
//	DMAI2C0MasterRX();
//	DMAI2C0SlaveTX();
//	DMAI2C0SlaveRX();
	
	while(1);	// Pass
}

/*****************************************************************************
* Function		: DMAI2C0MasterTX
* Description	: Start DMA I2C Master TX transfer
* Input				: None
* Output			: None
* Return			: None
* Note				: None
*****************************************************************************/
void DMAI2C0MasterTX(void)
{
	//Initial DMA & I2C for memory to peripheral(I2C TX) transfer 
	DMA_InitSt stDMACH_Init;
	DMA_StartSt stDMACH_Start;
	uint32_t eI2C0_DMA_Status = eI2C_DMA_START;
	uint32_t i;
	
	// Set DMA CH0 as M(SRAM) to P(I2C TX) mode
	// Source : Memory, Increment, 8bit, no request, Burst 1
	stDMACH_Init.b_SrcAddrCtrl = DMA_SRCAD_CTL_INC;
	stDMACH_Init.b_SrcMode     = DMA_SRC_MEMORY;
	stDMACH_Init.b_SrcWidth    = DMA_SRC_WIDTH_8BIT;
	stDMACH_Init.b_SrcReqSel   = DMA_SRCRS_NONE;
	stDMACH_Init.b_BurstSize   = DMA_BURST_1;
	// Destination : Peripheral I2C, Fixed, 8bit, I2C0 TX
	stDMACH_Init.b_DstAddrCtrl = DMA_DSTAD_CTL_FIX;
	stDMACH_Init.b_DstMode     = DMA_DST_PERIPHERAL;
	stDMACH_Init.b_DstWidth    = DMA_DST_WIDTH_8BIT;
	stDMACH_Init.b_DstReqSel   = DMA_DSTRS_I2C;
	// Channel : LV0, Enable TC/ABT interrupt
	stDMACH_Init.b_Priority    = DMA_CHPRI_LV0;
	stDMACH_Init.b_IntTCEn		 = DMA_INT_TC_MSK_DIS;
	stDMACH_Init.b_IntABTEn		 = DMA_INT_ABT_MSK_DIS;
	
	// Initial DMA
	DMA_Init(&stDMACH_Init, eDMA_CH0);
	
	// Set source/destination address and total size
	stDMACH_Start.w_SrcAddr 	= (uint32_t)b_aryI2C0Data;
	stDMACH_Start.w_DstAddr	 	= (uint32_t)&SN_I2C0->TXDATA;
	stDMACH_Start.w_TotalSize = DMA_TEST_SIZE;
	
	// Init data buffer
	for(i = 0; i<DMA_TEST_SIZE ;i++)
	{
		// Set Tx data = 0x10, 0x11, ... ,0x20
		b_aryI2C0Data[i] = (i + 0x10) & 0xFF;
	}
	
	// Reset DMA interrupt flag
	stDMA0IntFlag[eDMA_CH0].Flag.All = 0x00;
	
	// DMA Start
	DMA_Start(&stDMACH_Start, eDMA_CH0);
	
	I2C0_DMA_Master_TX_Start(I2C_ADDR_SLAVE_ADDR0, DMA_TEST_SIZE);
	
	// Wait for I2C DMA done
	while((eI2C0_DMA_Status == eI2C_DMA_START) || (eI2C0_DMA_Status == eI2C_DMA_WORKING))
	{
		eI2C0_DMA_Status = I2C0_Get_DMA_Status();
	}
	
	if(eI2C0_DMA_Status != eI2C_DMA_DONE)
	{
		// Fail : When an unexpected condition occurs on the I2C bus, FW must abort the DMA operation
		DMA_Abort(eDMA_CH0);
		while(stDMA0IntFlag[eDMA_CH0].Flag.bits.ABT != 1);
	}
	else
	{
		// Pass
		while(stDMA0IntFlag[eDMA_CH0].Flag.bits.TC != 1);
	}
}

/*****************************************************************************
* Function		: DMAI2C0MasterRX
* Description	: Start DMA I2C Master RX transfer
* Input				: None
* Output			: None
* Return			: None
* Note				: None
*****************************************************************************/
void DMAI2C0MasterRX(void)
{
	//Initial DMA & I2C for peripheral(I2C RX) to memory transfer 
	DMA_InitSt stDMACH_Init;
	DMA_StartSt stDMACH_Start;
	uint32_t eI2C0_DMA_Status = eI2C_DMA_START;
	uint32_t i;
	
	// Set DMA CH0 as P(I2C RX) to M(SRAM) mode
	// Source : Peripheral I2C, Fixed, 8bit, I2C0 TX
	stDMACH_Init.b_SrcAddrCtrl = DMA_SRCAD_CTL_FIX;
	stDMACH_Init.b_SrcMode     = DMA_SRC_PERIPHERAL;
	stDMACH_Init.b_SrcWidth    = DMA_SRC_WIDTH_8BIT;
	stDMACH_Init.b_SrcReqSel   = DMA_SRCRS_I2C;
	stDMACH_Init.b_BurstSize   = DMA_BURST_1;
	// Destination : Memory, Increment, 8bit, no request, Burst 1
	stDMACH_Init.b_DstAddrCtrl = DMA_DSTAD_CTL_INC;
	stDMACH_Init.b_DstMode     = DMA_DST_MEMORY;
	stDMACH_Init.b_DstWidth    = DMA_DST_WIDTH_8BIT;
	stDMACH_Init.b_DstReqSel   = DMA_DSTRS_NONE;
	// Channel : LV0, Enable TC/ABT interrupt
	stDMACH_Init.b_Priority    = DMA_CHPRI_LV0;
	stDMACH_Init.b_IntTCEn		 = DMA_INT_TC_MSK_DIS;
	stDMACH_Init.b_IntABTEn		 = DMA_INT_ABT_MSK_DIS;
	
	// Initial DMA
	DMA_Init(&stDMACH_Init, eDMA_CH0);
	
	// Set source/destination address and total size
	stDMACH_Start.w_SrcAddr 	= (uint32_t)&SN_I2C0->RXDATA;
	stDMACH_Start.w_DstAddr	 	= (uint32_t)b_aryI2C0Data;
	stDMACH_Start.w_TotalSize = DMA_TEST_SIZE;
	
	// Init data buffer
	for(i = 0; i<DMA_TEST_SIZE ;i++)
	{
		b_aryI2C0Data[i] = 0x00;
	}
	
	// Reset DMA interrupt flag
	stDMA0IntFlag[eDMA_CH0].Flag.All = 0x00;
	
	// DMA Start
	DMA_Start(&stDMACH_Start, eDMA_CH0);
	
	I2C0_DMA_Master_RX_Start(I2C_ADDR_SLAVE_ADDR0, DMA_TEST_SIZE, I2C_RX_DMA_LAST_NACK);
	
	// Wait for I2C DMA done
	while((eI2C0_DMA_Status == eI2C_DMA_START) || (eI2C0_DMA_Status == eI2C_DMA_WORKING))
	{
		eI2C0_DMA_Status = I2C0_Get_DMA_Status();
	}
	
	if(eI2C0_DMA_Status != eI2C_DMA_DONE)
	{
		// Fail : When an unexpected condition occurs on the I2C bus, FW must abort the DMA operation
		DMA_Abort(eDMA_CH0);
		while(stDMA0IntFlag[eDMA_CH0].Flag.bits.ABT != 1);
	}
	else
	{
		while(stDMA0IntFlag[eDMA_CH0].Flag.bits.TC != 1);
		// Check rx data
		for(i = 0; i<DMA_TEST_SIZE ;i++)
		{
			if(b_aryI2C0Data[i] != ((i + 0x10) & 0xFF))
				while(1);	// Fail
		}
		// Pass
	}
}

/*****************************************************************************
* Function		: DMAI2C0SlaveTX
* Description	: Start DMA I2C Slave TX transfer
* Input				: None
* Output			: None
* Return			: None
* Note				: None
*****************************************************************************/
void DMAI2C0SlaveTX(void)
{
	//Initial DMA & I2C for memory to peripheral(I2C TX) transfer 
	DMA_InitSt stDMACH_Init;
	DMA_StartSt stDMACH_Start;
	uint32_t eI2C0_DMA_Status = eI2C_DMA_START;
	uint32_t i;
	
	// Set I2C slave address
	I2C0_SlaveAddressSet(I2C_ADD_MODE_7BIT, I2C_GCEN_DIS);
	
	// Set DMA CH0 as M(SRAM) to P(I2C TX) mode
	// Source : Memory, Increment, 8bit, no request, Burst 1
	stDMACH_Init.b_SrcAddrCtrl = DMA_SRCAD_CTL_INC;
	stDMACH_Init.b_SrcMode     = DMA_SRC_MEMORY;
	stDMACH_Init.b_SrcWidth    = DMA_SRC_WIDTH_8BIT;
	stDMACH_Init.b_SrcReqSel   = DMA_SRCRS_NONE;
	stDMACH_Init.b_BurstSize   = DMA_BURST_1;
	// Destination : Peripheral I2C, Fixed, 8bit, I2C0 TX
	stDMACH_Init.b_DstAddrCtrl = DMA_DSTAD_CTL_FIX;
	stDMACH_Init.b_DstMode     = DMA_DST_PERIPHERAL;
	stDMACH_Init.b_DstWidth    = DMA_DST_WIDTH_8BIT;
	stDMACH_Init.b_DstReqSel   = DMA_DSTRS_I2C;
	// Channel : LV0, Enable TC/ABT interrupt
	stDMACH_Init.b_Priority    = DMA_CHPRI_LV0;
	stDMACH_Init.b_IntTCEn		 = DMA_INT_TC_MSK_DIS;
	stDMACH_Init.b_IntABTEn		 = DMA_INT_ABT_MSK_DIS;
	
	// Initial DMA
	DMA_Init(&stDMACH_Init, eDMA_CH0);
	
	// Set source/destination address and total size
	stDMACH_Start.w_SrcAddr 	= (uint32_t)b_aryI2C0Data;
	stDMACH_Start.w_DstAddr	 	= (uint32_t)&SN_I2C0->TXDATA;
	stDMACH_Start.w_TotalSize = DMA_TEST_SIZE;
	
	// Init data buffer
	for(i = 0; i<DMA_TEST_SIZE ;i++)
	{
		// Set Tx data = 0x10, 0x11, ... ,0x20
		b_aryI2C0Data[i] = (i + 0x10) & 0xFF;
	}
	
	// Reset DMA interrupt flag
	stDMA0IntFlag[eDMA_CH0].Flag.All = 0x00;
	
	// DMA Start
	DMA_Start(&stDMACH_Start, eDMA_CH0);
	
	I2C0_DMA_Slave_TX_Start(DMA_TEST_SIZE);
	
	// Wait for I2C DMA done
	while((eI2C0_DMA_Status == eI2C_DMA_START) || (eI2C0_DMA_Status == eI2C_DMA_WORKING))
	{
		eI2C0_DMA_Status = I2C0_Get_DMA_Status();
	}
	
	if(eI2C0_DMA_Status != eI2C_DMA_DONE)
	{
		// Fail : When an unexpected condition occurs on the I2C bus, FW must abort the DMA operation
		DMA_Abort(eDMA_CH0);
		while(stDMA0IntFlag[eDMA_CH0].Flag.bits.ABT != 1);
	}
	else
	{
		// Pass
		while(stDMA0IntFlag[eDMA_CH0].Flag.bits.TC != 1);
	}
}

/*****************************************************************************
* Function		: DMAI2C0SlaveRX
* Description	: Start DMA I2C Slave RX transfer
* Input				: None
* Output			: None
* Return			: None
* Note				: None
*****************************************************************************/
void DMAI2C0SlaveRX(void)
{
	//Initial DMA & I2C for peripheral(I2C RX) to memory transfer 
	DMA_InitSt stDMACH_Init;
	DMA_StartSt stDMACH_Start;
	uint32_t eI2C0_DMA_Status = eI2C_DMA_START;
	uint32_t i;
	
	// Set I2C slave address
	I2C0_SlaveAddressSet(I2C_ADD_MODE_7BIT, I2C_GCEN_DIS);
	
	// Set DMA CH0 as P(I2C RX) to M(SRAM) mode
	// Source : Peripheral I2C, Fixed, 8bit, I2C0 TX
	stDMACH_Init.b_SrcAddrCtrl = DMA_SRCAD_CTL_FIX;
	stDMACH_Init.b_SrcMode     = DMA_SRC_PERIPHERAL;
	stDMACH_Init.b_SrcWidth    = DMA_SRC_WIDTH_8BIT;
	stDMACH_Init.b_SrcReqSel   = DMA_SRCRS_I2C;
	stDMACH_Init.b_BurstSize   = DMA_BURST_1;
	// Destination : Memory, Increment, 8bit, no request, Burst 1
	stDMACH_Init.b_DstAddrCtrl = DMA_DSTAD_CTL_INC;
	stDMACH_Init.b_DstMode     = DMA_DST_MEMORY;
	stDMACH_Init.b_DstWidth    = DMA_DST_WIDTH_8BIT;
	stDMACH_Init.b_DstReqSel   = DMA_DSTRS_NONE;
	// Channel : LV0, Enable TC/ABT interrupt
	stDMACH_Init.b_Priority    = DMA_CHPRI_LV0;
	stDMACH_Init.b_IntTCEn		 = DMA_INT_TC_MSK_DIS;
	stDMACH_Init.b_IntABTEn		 = DMA_INT_ABT_MSK_DIS;
	
	// Initial DMA
	DMA_Init(&stDMACH_Init, eDMA_CH0);
	
	// Set source/destination address and total size
	stDMACH_Start.w_SrcAddr 	= (uint32_t)&SN_I2C0->RXDATA;
	stDMACH_Start.w_DstAddr	 	= (uint32_t)b_aryI2C0Data;
	stDMACH_Start.w_TotalSize = DMA_TEST_SIZE;
	
	// Init data buffer
	for(i = 0; i<DMA_TEST_SIZE ;i++)
	{
		b_aryI2C0Data[i] = 0x00;
	}
	
	// Reset DMA interrupt flag
	stDMA0IntFlag[eDMA_CH0].Flag.All = 0x00;
	
	// DMA Start
	DMA_Start(&stDMACH_Start, eDMA_CH0);
	
	I2C0_DMA_Slave_RX_Start(DMA_TEST_SIZE, I2C_RX_DMA_LAST_NACK);
	
	// Wait for I2C DMA done
	while((eI2C0_DMA_Status == eI2C_DMA_START) || (eI2C0_DMA_Status == eI2C_DMA_WORKING))
	{
		eI2C0_DMA_Status = I2C0_Get_DMA_Status();
	}
	
	if(eI2C0_DMA_Status != eI2C_DMA_DONE)
	{
		// Fail : When an unexpected condition occurs on the I2C bus, FW must abort the DMA operation
		DMA_Abort(eDMA_CH0);
		while(stDMA0IntFlag[eDMA_CH0].Flag.bits.ABT != 1);
	}
	else
	{
		while(stDMA0IntFlag[eDMA_CH0].Flag.bits.TC != 1);
		// Check rx data
		for(i = 0; i<DMA_TEST_SIZE ;i++)
		{
			if(b_aryI2C0Data[i] != ((i + 0x10) & 0xFF))
				while(1);	// Fail
		}
		// Pass
	}
}

/*****************************************************************************
* Function		: NotPinOut_GPIO_init
* Description	: Set the status of the GPIO which are NOT pin-out to input pull-up. 
* Input				: None
* Output			: None
* Return			: None
* Note				: 1. User SHALL define PKG on demand.
*****************************************************************************/
void NotPinOut_GPIO_init(void)
{
#if (PKG == SN32F405)
	//set P0.4, P0.6, P0.7 to input pull-up
	SN_GPIO0->CFG = 0x00A008AA;
	//set P1.4 ~ P1.12 to input pull-up
	SN_GPIO1->CFG = 0x000000AA;
	//set P3.8 ~ P3.11 to input pull-up
	SN_GPIO3->CFG = 0x0002AAAA;
#elif (PKG == SN32F403)
	//set P0.4 ~ P0.7 to input pull-up
	SN_GPIO0->CFG = 0x00A000AA;
	//set P1.4 ~ P1.12 to input pull-up
	SN_GPIO1->CFG = 0x000000AA;
	//set P2.5 ~ P2.6, P2.10 to input pull-up
	SN_GPIO2->CFG = 0x000A82AA;
	//set P3.0, P3.8 ~ P3.13 to input pull-up
	SN_GPIO3->CFG = 0x0000AAA8;
#endif
}

/*****************************************************************************
* Function		: HardFault_Handler
* Description	: ISR of Hard fault interrupt
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void HardFault_Handler(void)
{
	NVIC_SystemReset();
}
