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
#include ".\Interface\SPI\SPI.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);

void DMASPI0TXInit(void);
void DMASPI0RXInit(void);

void DMASPI0TXStart(void);
void DMASPI0RXStart(void);

/*_____ D E F I N I T I O N S ______________________________________________*/
#ifndef	SN32F407					//Do NOT Remove or Modify!!!
	#error Please install SONiX.SN32F4_DFP.1.0.1.pack or version >= 1.0.1
#endif
#define	PKG						SN32F407				//User SHALL modify the package on demand (SN32F407)

// DMA transfer size 256 bytes
#define DMA_TEST_SIZE 0x100

volatile uint8_t b_arySPITXData[DMA_TEST_SIZE];
volatile uint8_t b_arySPIRXData[DMA_TEST_SIZE];

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/
	
/*****************************************************************************
* Function		: main
* Description	: Demo code of DMA SPI.
* Input			: None
* Output		: None
* Return		: None
* Note			: Connect SPI MISO0 and MOSI0 
*****************************************************************************/
int	main(void)
{
	uint32_t i;
	
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
	
	// Initial SPI
	SPI0_Init();
	
	//Auto-SEL enable
	SN_SPI0->CTRL0_b.SELDIS = SPI_SELDIS_EN;
	
	// Disable SPI ISR when using DMA SPI
	NVIC_DisableIRQ(SPI0_IRQn);
	
	// Init DMA
	DMASPI0TXInit();
	DMASPI0RXInit();
	
	// Init data buffer
	for(i = 0; i<DMA_TEST_SIZE ;i++)
	{
		// Set Tx data = 0x00, 0x01, ... ,0xFF
		b_arySPITXData[i] = i & 0xFF;
		// Set Rx data = 0x00
		b_arySPIRXData[i] = 0x00;
	}
	
	// Start DMA SPI TX/RX transfer
	DMASPI0RXStart();
	DMASPI0TXStart();
	// Set SPI DMA Enable
	SPI0_DMAEnable(mskSPI_TX_DMA_EN | mskSPI_RX_DMA_EN, DMA_TEST_SIZE);
	
	// Wait for DMA TC interrupt flag
	while(stDMA0IntFlag[eDMA_CH0].Flag.bits.TC != 1);
	while(stDMA0IntFlag[eDMA_CH1].Flag.bits.TC != 1);
	
	// make sure all data has been TX though SPI
	while(SN_SPI0->RIS_b.TXEMPIF != 1);
	
	// Check TX data = RX data
	for(i = 0; i<DMA_TEST_SIZE ;i++)
	{
		if(b_arySPITXData[i] != b_arySPIRXData[i])
			while(1); //Fail
	}
	
	while(1);	// Pass
}

/*****************************************************************************
* Function		: DMASPI0TXInit
* Description	: Initial DMA & SPI for memory to peripheral(SPI TX) transfer 
* Input				: None
* Output			: None
* Return			: None
* Note				: None
*****************************************************************************/
void DMASPI0TXInit(void)
{
	DMA_InitSt stDMACH_Init;
	
	// The settings of SPI FIFO TX/RX TH and DMA width/burst size must match the table in SPI datasheet
	// Data length = 8bit, SPI FIFO depth = 16 bytes
	SN_SPI0->FIFO_TH_b.NEW_TH_EN = SPI_NEW_TH_EN;
	SN_SPI0->FIFO_TH_b.NEW_TXFIFO_TH = SPI_NEW_TXFIFOTH_4;
	
	// Set DMA CH0 as M(SRAM) to P(SPI TX) mode
	// Source : Memory, Increment, 8bit, no request, Burst 4
	stDMACH_Init.b_SrcAddrCtrl = DMA_SRCAD_CTL_INC;
	stDMACH_Init.b_SrcMode     = DMA_SRC_MEMORY;
	stDMACH_Init.b_SrcWidth    = DMA_SRC_WIDTH_8BIT;
	stDMACH_Init.b_SrcReqSel   = DMA_SRCRS_NONE;
	stDMACH_Init.b_BurstSize   = DMA_BURST_4;
	// Destination : Peripheral SPI, Fixed, 8bit, SPI0 TX
	stDMACH_Init.b_DstAddrCtrl = DMA_DSTAD_CTL_FIX;
	stDMACH_Init.b_DstMode     = DMA_DST_PERIPHERAL;
	stDMACH_Init.b_DstWidth    = DMA_DST_WIDTH_8BIT;
	stDMACH_Init.b_DstReqSel   = DMA_DSTRS_SPI0_TX;
	// Channel : LV0, Enable TC/ABT interrupt
	stDMACH_Init.b_Priority    = DMA_CHPRI_LV0;
	stDMACH_Init.b_IntTCEn		 = DMA_INT_TC_MSK_DIS;
	stDMACH_Init.b_IntABTEn		 = DMA_INT_ABT_MSK_DIS;
	
	// Initial DMA
	DMA_Init(&stDMACH_Init, eDMA_CH0);
}

/*****************************************************************************
* Function		: DMASPI0RXInit
* Description	: Initial DMA for peripheral(SPI RX) to memory transfer 
* Input				: None
* Output			: None
* Return			: None
* Note				: None
*****************************************************************************/
void DMASPI0RXInit(void)
{
	DMA_InitSt 	stDMACH_Init;
	
	// The settings of SPI FIFO TX/RX TH and DMA width/burst size must match the table in SPI datasheet
	// Data length = 8bit, SPI FIFO depth = 16 bytes
	SN_SPI0->FIFO_TH_b.NEW_TH_EN = SPI_NEW_TH_EN;
	SN_SPI0->FIFO_TH_b.NEW_RXFIFO_TH = SPI_NEW_RXFIFOTH_3;
	
	// Set DMA CH1 as P(SPI RX) to M(SRAM) mode
	// Source : Peripheral SPI, Fixed, 8bit, SPI0 RX, Burst 4
	stDMACH_Init.b_SrcAddrCtrl = DMA_SRCAD_CTL_FIX;
	stDMACH_Init.b_SrcMode     = DMA_SRC_PERIPHERAL;
	stDMACH_Init.b_SrcWidth    = DMA_SRC_WIDTH_8BIT;
	stDMACH_Init.b_SrcReqSel   = DMA_SRCRS_SPI0_RX;
	stDMACH_Init.b_BurstSize   = DMA_BURST_4;
	// Destination : Memory, Increment, 8bit, no request
	stDMACH_Init.b_DstAddrCtrl = DMA_DSTAD_CTL_INC;
	stDMACH_Init.b_DstMode     = DMA_DST_MEMORY;
	stDMACH_Init.b_DstWidth    = DMA_DST_WIDTH_8BIT;
	stDMACH_Init.b_DstReqSel   = DMA_DSTRS_NONE;
	// Channel : LV0, Enable TC/ABT interrupt
	stDMACH_Init.b_Priority    = DMA_CHPRI_LV0;
	stDMACH_Init.b_IntTCEn		 = DMA_INT_TC_MSK_DIS;
	stDMACH_Init.b_IntABTEn		 = DMA_INT_ABT_MSK_DIS;
	
	// Initial DMA
	DMA_Init(&stDMACH_Init, eDMA_CH1);
}

/*****************************************************************************
* Function		: DMASPI0TXStart
* Description	: Start DMA SPI TX transfer 
* Input				: None
* Output			: None
* Return			: None
* Note				: None
*****************************************************************************/
void DMASPI0TXStart(void)
{	
	DMA_StartSt stDMACH_Start;
	
	// Set source/destination address and total size
	stDMACH_Start.w_SrcAddr 	= (uint32_t)b_arySPITXData;
	stDMACH_Start.w_DstAddr	 	= (uint32_t)&SN_SPI0->DATA;
	stDMACH_Start.w_TotalSize = DMA_TEST_SIZE;
	
	// Reset DMA interrupt flag
	stDMA0IntFlag[eDMA_CH0].Flag.All = 0x00;
	
	// DMA Start
	DMA_Start(&stDMACH_Start, eDMA_CH0);
}

/*****************************************************************************
* Function		: DMASPI0RXStart
* Description	: Start DMA SPI RX transfer 
* Input				: None
* Output			: None
* Return			: None
* Note				: None
*****************************************************************************/
void DMASPI0RXStart(void)
{	
	DMA_StartSt stDMAStart;
	
	//Set source/destination address and total size
	stDMAStart.w_SrcAddr 	 = (uint32_t)&SN_SPI0->DATA;
	stDMAStart.w_DstAddr	 = (uint32_t)b_arySPIRXData;
	stDMAStart.w_TotalSize = DMA_TEST_SIZE;
	
	//Reset DMA interrupt flag
	stDMA0IntFlag[eDMA_CH1].Flag.All = 0x00;
	
	//DMA Start
	DMA_Start(&stDMAStart, eDMA_CH1);
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
