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

/*_____ D E C L A R A T I O N S ____________________________________________*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);

void DMAMtoMInit(void);

void DMAFlashtoSramStart(void);
void DMASramtoSramStart(void);

/*_____ D E F I N I T I O N S ______________________________________________*/
#ifndef	SN32F407					//Do NOT Remove or Modify!!!
	#error Please install SONiX.SN32F4_DFP.1.0.1.pack or version >= 1.0.1
#endif
#define	PKG						SN32F407				//User SHALL modify the package on demand (SN32F407)

// DMA transfer size 256 bytes
#define DMA_TEST_SIZE 0x100

uint8_t b_arySramSrcData[DMA_TEST_SIZE];
uint8_t b_arySramDstData[DMA_TEST_SIZE];

// Random Flash data 
const uint8_t b_aryFlashSrcData[DMA_TEST_SIZE] = 
{
	0x59, 0x79, 0x69, 0xfd, 0x9d, 0xe3, 0xf0, 0xef, 0x2d, 0xed, 0xb4, 0xb4, 0x05, 0x8b, 0xff, 0x08,
	0x19, 0xe5, 0x47, 0x2f, 0x2b, 0x33, 0x49, 0x30, 0x82, 0x04, 0x62, 0x9a, 0xdd, 0xec, 0xa3, 0x3b,
	0xc5, 0xf9, 0x85, 0x38, 0x54, 0x8f, 0x5d, 0xa0, 0x0f, 0x19, 0x07, 0x0b, 0xdd, 0xb0, 0xea, 0x33,
	0xb9, 0x77, 0x25, 0x10, 0xc6, 0x84, 0x6c, 0xec, 0x32, 0xb5, 0x94, 0x70, 0x4d, 0x68, 0xa3, 0x33,
	0x81, 0x4c, 0x33, 0x20, 0x3b, 0x10, 0xf6, 0x78, 0x90, 0x79, 0x13, 0xaf, 0xe5, 0xe0, 0x83, 0x02,
	0xe9, 0xad, 0xec, 0x63, 0x5f, 0xc5, 0x6e, 0xd1, 0xaa, 0x6b, 0xc5, 0xaf, 0xf9, 0x8c, 0x84, 0x8f,
	0x27, 0xf3, 0xb3, 0xe4, 0xb7, 0xf0, 0xc2, 0x14, 0x7e, 0x37, 0xa1, 0xc9, 0xa0, 0xf3, 0xed, 0x2e,
	0x50, 0x17, 0xa7, 0xbe, 0x65, 0x98, 0xeb, 0xa2, 0x6a, 0x7d, 0xdf, 0x06, 0x69, 0xe1, 0x5e, 0x17,
	0xd7, 0x31, 0xdc, 0x00, 0xb3, 0xa3, 0x7c, 0x20, 0x3b, 0x3d, 0x91, 0xad, 0x95, 0x3c, 0x57, 0xbf,
	0x18, 0x39, 0x82, 0xed, 0x90, 0x10, 0xec, 0x4d, 0xaf, 0x1d, 0x5c, 0xd0, 0x84, 0x67, 0xe6, 0x90,
	0xff, 0x9c, 0xa9, 0xf3, 0xa0, 0xc6, 0xaa, 0x39, 0x44, 0xa5, 0x79, 0x01, 0xab, 0x35, 0x38, 0x6d,
	0xe5, 0x54, 0xf0, 0xc5, 0x43, 0xd6, 0x44, 0xb5, 0x1d, 0xc5, 0x50, 0x7b, 0x3b, 0xfd, 0xa2, 0xfb,
	0x60, 0xae, 0xa4, 0x3b, 0xa5, 0xe5, 0x64, 0xc3, 0xbc, 0x02, 0x14, 0xff, 0xe2, 0xd5, 0xe1, 0x44,
	0x65, 0x94, 0x04, 0x0b, 0x5d, 0xc9, 0xbd, 0xb3, 0xbd, 0x4d, 0x10, 0xff, 0x54, 0x01, 0x80, 0x2b,
	0xb5, 0x25, 0xf3, 0x04, 0xb0, 0x97, 0x01, 0x09, 0xdd, 0xf9, 0x5b, 0x11, 0xeb, 0x31, 0x0a, 0x4a,
	0x60, 0x10, 0x20, 0xca, 0x07, 0x6c, 0x39, 0x8b, 0x72, 0xb2, 0x79, 0xa4, 0x4d, 0x2a, 0xb9, 0x3d
};
	
/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/
	
/*****************************************************************************
* Function		: main
* Description	: Demo code of DMA MtoM mode.
* Input			: None
* Output		: None
* Return		: None
* Note			: None
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
	
	// Init DMA
	DMAMtoMInit();
	
	// Init data buffer
	for (i = 0; i < DMA_TEST_SIZE; i++)
	{
		// Set source data = 0x00, 0x01, ... ,0xFF
		b_arySramSrcData[i] = i & 0xFF;
		// Set destination data = 0x00
		b_arySramDstData[i] = 0x00;
	}
	
	/* Example 1: DMA Flash to SRAM Start*/
	
	// Start DMA transfer
	DMAFlashtoSramStart();
	
	// Wait for DMA TC interrupt flag
	while (stDMA0IntFlag[eDMA_CH0].Flag.bits.TC != 1);
	
	// Check SRAM data = FLASH data
	for (i = 0; i < DMA_TEST_SIZE; i++)
	{
		if (b_aryFlashSrcData[i] != b_arySramDstData[i])
			while (1); //Fail
	}
	
	/* Example 1: DMA Flash to SRAM End*/
	
	// Reset destination data buffer
	for (i = 0; i < DMA_TEST_SIZE; i++)
	{
		// Set destination data = 0x00
		b_arySramDstData[i] = 0x00;
	}
	
	/* Example 2: DMA SRAM to SRAM Start*/
	
	// Start DMA transfer
	DMASramtoSramStart();
	
	// Wait for DMA TC interrupt flag
	while (stDMA0IntFlag[eDMA_CH0].Flag.bits.TC != 1);
	
	// Check TX data = RX data
	for (i = 0; i < DMA_TEST_SIZE; i++)
	{
		if (b_arySramSrcData[i] != b_arySramDstData[i])
			while (1); //Fail
	}
	
	/* Example 2: DMA SRAM to SRAM End*/
	
	while (1);	// Pass
}

/*****************************************************************************
* Function		: DMAMtoMInit
* Description	: Initial DMA for memory to Memory transfer 
* Input				: None
* Output			: None
* Return			: None
* Note				: None
*****************************************************************************/
void DMAMtoMInit(void)
{
	DMA_InitSt stDMACH_Init;
	
	// Set DMA CH0 as M(SRAM/FLASH) to M(SRAM/FLASH) mode
	// Source : Memory, Increment, 8bit, no request, Burst 1
	stDMACH_Init.b_SrcAddrCtrl = DMA_SRCAD_CTL_INC;
	stDMACH_Init.b_SrcMode     = DMA_SRC_MEMORY;
	stDMACH_Init.b_SrcWidth    = DMA_SRC_WIDTH_8BIT;
	stDMACH_Init.b_SrcReqSel   = DMA_SRCRS_NONE;
	stDMACH_Init.b_BurstSize   = DMA_BURST_1;
	// Destination : Memory, Fixed, 8bit, no request
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
}

/*****************************************************************************
* Function		: DMAFlashtoSramStart
* Description	: Start DMA Memory transfer 
* Input				: None
* Output			: None
* Return			: None
* Note				: None
*****************************************************************************/
void DMAFlashtoSramStart(void)
{	
	DMA_StartSt stDMACH_Start;
	
	// Set source/destination address and total size
	stDMACH_Start.w_SrcAddr 	= (uint32_t)b_aryFlashSrcData;
	stDMACH_Start.w_DstAddr	 	= (uint32_t)b_arySramDstData;
	stDMACH_Start.w_TotalSize = DMA_TEST_SIZE;
	
	// Reset DMA interrupt flag
	stDMA0IntFlag[eDMA_CH0].Flag.All = 0x00;
	
	// DMA Start
	DMA_Start(&stDMACH_Start, eDMA_CH0);
}

/*****************************************************************************
* Function		: DMASramtoSramStart
* Description	: Start DMA UART RX transfer 
* Input				: None
* Output			: None
* Return			: None
* Note				: None
*****************************************************************************/
void DMASramtoSramStart(void)
{	
	DMA_StartSt stDMAStart;
	
	//Set source/destination address and total size
	stDMAStart.w_SrcAddr 	 = (uint32_t)b_arySramSrcData;
	stDMAStart.w_DstAddr	 = (uint32_t)b_arySramDstData;
	stDMAStart.w_TotalSize = DMA_TEST_SIZE;
	
	//Reset DMA interrupt flag
	stDMA0IntFlag[eDMA_CH0].Flag.All = 0x00;
	
	//DMA Start
	DMA_Start(&stDMAStart, eDMA_CH0);
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
