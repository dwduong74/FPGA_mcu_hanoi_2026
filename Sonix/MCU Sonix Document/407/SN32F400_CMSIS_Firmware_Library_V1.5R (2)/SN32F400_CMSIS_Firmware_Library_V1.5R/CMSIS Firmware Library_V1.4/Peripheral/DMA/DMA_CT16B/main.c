/******************** (C) COPYRIGHT 2024 SONiX *******************************
* COMPANY:		SONiX
* DATE:				2023/11
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
#include ".\Interface\CT\CT16.h"
#include ".\Interface\CT\CT16B0.h"
#include ".\Interface\CT\CT16B1.h"
#include ".\Interface\CT\CT16B5.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);

void DMACT16B0PWM0123Init(void);
void DMACT16B0PWM0123Start(void);

/*_____ D E F I N I T I O N S ______________________________________________*/
#ifndef	SN32F407					//Do NOT Remove or Modify!!!
	#error Please install SONiX.SN32F4_DFP.1.0.1.pack or version >= 1.0.1
#endif
#define	PKG						SN32F407				//User SHALL modify the package on demand (SN32F407)

// DMA transfer size 256 (unit:2 bytes)
#define DMA_TEST_SIZE 0x100

uint16_t db_tabCT16B0DMA[DMA_TEST_SIZE]; 

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/
	
/*****************************************************************************
* Function		: main
* Description	: Demo code of DMA CT16B.
* Input			: None
* Output		: None
* Return		: None
* Note			: Connect UART0 TX to UART1 RX
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
	
	// Init CT16B0 & DMA
	DMACT16B0PWM0123Init();
	
	// Init data buffer
	/*
	PWM0 duty : 10% -> 	20% -> 30% ->	 40%	...
	PWM1 duty : 30% -> 	40% -> 50% ->	 60%	...
	PWM2 duty : 70% -> 	80% -> 90% -> 100%	...
	PWM3 duty : 90% -> 100% -> 	0% ->  10%	...
	*/
	for(i = 0; i < DMA_TEST_SIZE; i++)
	{
		switch(i % 4)
		{
			case 0:	db_tabCT16B0DMA[i] = ( 2400 + 1200 * (i / 4)) % 12000; break;
			case 1:	db_tabCT16B0DMA[i] = ( 4800 + 1200 * (i / 4)) % 12000; break;
			case 2:	db_tabCT16B0DMA[i] = ( 9600 + 1200 * (i / 4)) % 12000; break;
			case 3:	db_tabCT16B0DMA[i] = (		0 + 1200 * (i / 4)) % 12000; break;
		}
	}
	
	// Start DMA CT16B0 PWM0~3 transfer
	DMACT16B0PWM0123Start();
	
	// Wait for DMA TC interrupt flag
	while(stDMA0IntFlag[eDMA_CH0].Flag.bits.TC != 1);
	
	while(1);	// Pass
}

/*****************************************************************************
* Function		: DMACT16B0PWM0123Init
* Description	: Initial DMA & CT16B0 for memory to peripheral(CT16B0 PWM) transfer 
* Input				: None
* Output			: None
* Return			: None
* Note				: None
*****************************************************************************/
void DMACT16B0PWM0123Init(void)
{
	DMA_InitSt stDMACH_Init;
	
	// Set DMA CH0 as M(SRAM) to P(CT16B0 PWM0~3) mode
	// Source : Memory, Increment, 16bit, no request, Burst 4
	stDMACH_Init.b_SrcAddrCtrl = DMA_SRCAD_CTL_INC;
	stDMACH_Init.b_SrcMode     = DMA_SRC_MEMORY;
	stDMACH_Init.b_SrcWidth    = DMA_SRC_WIDTH_16BIT;
	stDMACH_Init.b_SrcReqSel   = DMA_SRCRS_NONE;
	stDMACH_Init.b_BurstSize   = DMA_BURST_4;
	// Destination : Peripheral CT16B0, Increment cyclic, 16bit, CT16B0 PWM0~3
	stDMACH_Init.b_DstAddrCtrl = DMA_DSTAD_CTL_CYC;
	stDMACH_Init.b_DstMode     = DMA_DST_PERIPHERAL;
	stDMACH_Init.b_DstWidth    = DMA_DST_WIDTH_16BIT;
	stDMACH_Init.b_DstReqSel   = DMA_DSTRS_CT16B0_MR0;
	// Channel : LV0, Enable TC/ABT interrupt
	stDMACH_Init.b_Priority    = DMA_CHPRI_LV0;
	stDMACH_Init.b_IntTCEn		 = DMA_INT_TC_MSK_DIS;
	stDMACH_Init.b_IntABTEn		 = DMA_INT_ABT_MSK_DIS;
	
	// Initial DMA
	DMA_Init(&stDMACH_Init, eDMA_CH0);
	
	// Initial CT16B
	CT16B0_Init();
	
	//Set MR9 value for 1ms period ==> count value = 1000*12 = 12000
	SN_CT16B0->MR9 = 12000;
	
	//Set MR0/1/2/3 value as 10%/30%/70%/90% duty
	SN_CT16B0->MR0 =  1200;
	SN_CT16B0->MR1 =  3600;
	SN_CT16B0->MR2 =  8400;
	SN_CT16B0->MR3 = 10800;
	
	//Enable PWM function, IOs and select the PWM modes
	SN_CT16B0->PWMCTRL = 
		(mskCT16_PWM0EN_EN | mskCT16_PWM0MODE_2 | mskCT16_PWM0IOEN_EN)|	//Enable PWM0 function, IO and select as PWM mode 1
		(mskCT16_PWM1EN_EN | mskCT16_PWM1MODE_2 | mskCT16_PWM1IOEN_EN)|	//Enable PWM1 function, IO and select as PWM mode 1
		(mskCT16_PWM2EN_EN | mskCT16_PWM2MODE_2 | mskCT16_PWM2IOEN_EN)|	//Enable PWM2 function, IO and select as PWM mode 1
		(mskCT16_PWM3EN_EN | mskCT16_PWM3MODE_2 | mskCT16_PWM3IOEN_EN);	//Enable PWM3 function, IO and select as PWM mode 1

	//Set MR9 match TC reset 
	SN_CT16B0->MCTRL = mskCT16_MR9RST_EN;
	
	//MR0 DMA request can issue.
	SN_CT16B0->DMA = mskCT16_MR0_DMA_EN;

	//Set CT16B3 as the up-counting mode.
	SN_CT16B0->TMRCTRL = (mskCT16_CRST | mskCT16_CM_EDGE_UP);
	
	//Wait until timer reset done.
	while (SN_CT16B0->TMRCTRL & mskCT16_CRST);
}

/*****************************************************************************
* Function		: DMACT16B0PWM0123Start
* Description	: Start DMA CT16B PWM transfer 
* Input				: None
* Output			: None
* Return			: None
* Note				: None
*****************************************************************************/
void DMACT16B0PWM0123Start(void)
{	
	DMA_StartSt stDMACH_Start;
	
	// Set source/destination address and total size
	stDMACH_Start.w_SrcAddr 	= (uint32_t)db_tabCT16B0DMA;
	stDMACH_Start.w_DstAddr	 	= (uint32_t)&SN_CT16B0->DMAMRA1;
	stDMACH_Start.w_TotalSize = DMA_TEST_SIZE;
	
	// Reset DMA interrupt flag
	stDMA0IntFlag[eDMA_CH0].Flag.All = 0x00;
	
	// DMA Start
	DMA_Start(&stDMACH_Start, eDMA_CH0);
	
	//Let TC start counting.
	SN_CT16B0->TMRCTRL |= mskCT16_CEN_EN;
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
