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
#include ".\Interface\ADC\ADC.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);

void DMAADCInit(void);
void DMAADCStart(void);

/*_____ D E F I N I T I O N S ______________________________________________*/
#ifndef	SN32F407					//Do NOT Remove or Modify!!!
	#error Please install SONiX.SN32F4_DFP.1.0.1.pack or version >= 1.0.1
#endif
#define	PKG						SN32F407				//User SHALL modify the package on demand (SN32F407)

// DMA transfer size 32 * 2 bytes
#define DMA_TEST_SIZE 0x20

uint16_t db_aryADCData[DMA_TEST_SIZE];

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/
	
/*****************************************************************************
* Function		: main
* Description	: Demo code of DMA ADC.
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
int	main(void) /* __attribute__((optnone)) */
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
	
	// Initial ADC
	ADC_FuncInit(ADC_DIV32, ADC_12BIT, Multiple_Channel, Continuous_Mode);
	
	// Init DMA
	DMAADCInit();
	
	// Start DMA ADC
	DMAADCStart();
	ADC_DMA_Start(ADC_CHS_AIN0|ADC_CHS_AIN1|ADC_CHS_AIN2|ADC_CHS_AIN3, ADC_DMA_FIFO_TH_3, DMA_TEST_SIZE);
	
	// Wait for DMA TC interrupt flag
	while(stDMA0IntFlag[eDMA_CH0].Flag.bits.TC != 1);
	
	while(1);	// Pass
}

/*****************************************************************************
* Function		: DMAADCInit
* Description	: Initial DMA & ADC for peripheral(ADC) to memory transfer 
* Input				: None
* Output			: None
* Return			: None
* Note				: None
*****************************************************************************/
void DMAADCInit(void)
{
	DMA_InitSt stDMACH_Init;
	
	// Set DMA CH0 as P(ADC) to M(SRAM) mode
	// Source : Peripheral ADC, Fixed, 8bit, ADC, Burst 4
	stDMACH_Init.b_SrcAddrCtrl = DMA_SRCAD_CTL_FIX;
	stDMACH_Init.b_SrcMode     = DMA_SRC_PERIPHERAL;
	stDMACH_Init.b_SrcWidth    = DMA_SRC_WIDTH_16BIT;
	stDMACH_Init.b_SrcReqSel   = DMA_SRCRS_ADC;
	stDMACH_Init.b_BurstSize   = DMA_BURST_4;
	// Destination : Memory, Increment, 8bit, no request
	stDMACH_Init.b_DstAddrCtrl = DMA_DSTAD_CTL_INC;
	stDMACH_Init.b_DstMode     = DMA_DST_MEMORY;
	stDMACH_Init.b_DstWidth    = DMA_DST_WIDTH_16BIT;
	stDMACH_Init.b_DstReqSel   = DMA_DSTRS_NONE;
	// Channel : LV0, Enable TC/ABT interrupt
	stDMACH_Init.b_Priority    = DMA_CHPRI_LV0;
	stDMACH_Init.b_IntTCEn		 = DMA_INT_TC_MSK_DIS;
	stDMACH_Init.b_IntABTEn		 = DMA_INT_ABT_MSK_DIS;
	
	// Initial DMA
	DMA_Init(&stDMACH_Init, eDMA_CH0);
}

/*****************************************************************************
* Function		: DMAADCStart
* Description	: Start DMA ADC transfer 
* Input				: None
* Output			: None
* Return			: None
* Note				: None
*****************************************************************************/
void DMAADCStart(void)
{	
	DMA_StartSt stDMACH_Start;
	
	// Set source/destination address and total size
	stDMACH_Start.w_SrcAddr 	= (uint32_t)&SN_ADC->ADB;
	stDMACH_Start.w_DstAddr	 	= (uint32_t)db_aryADCData;
	stDMACH_Start.w_TotalSize = DMA_TEST_SIZE;
	
	// Reset DMA interrupt flag
	stDMA0IntFlag[eDMA_CH0].Flag.All = 0x00;
	
	// DMA Start
	DMA_Start(&stDMACH_Start, eDMA_CH0);
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
