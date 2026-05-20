/******************** (C) COPYRIGHT 2024 SONiX *******************************
* COMPANY:		SONiX
* DATE:				2024/11
* AUTHOR:			SA1
* IC:					SN32F400
*____________________________________________________________________________
*	REVISION	Date				User		Description
*	1.0				2024/11/26	SA1			1. First version released
*																2. Compatible to CMSIS DFP Architecture in Keil MDK v5.X (http://www.keil.com/dd2/pack/)
*																3. Run HexConvert to generate bin file and show checksum after building.
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
#include ".\Interface\SysTick\SysTick.h"
#include ".\Interface\WDT\WDT.h"
#include ".\Interface\CT\CT16.h"
#include ".\Interface\CT\CT16B0.h"


/*_____ D E C L A R A T I O N S ____________________________________________*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);
void MN_CT16B0_Init(void);


/*_____ D E F I N I T I O N S ______________________________________________*/
#ifndef	SN32F407					//Do NOT Remove or Modify!!!
	#error Please install SONiX.SN32F4_DFP.1.0.1.pack or version >= 1.0.1
#endif
#define	PKG						SN32F407				//User SHALL modify the package on demand (SN32F407)

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

/*****************************************************************************
* Function		: main
* Description	: LED toggles based on SysTick timer and CT16B0 timer,
*				        and add WDT reset function. 
* Input			: None
* Output		: None
* Return		: None
* Note			: Connect LEDs to P2.0 and P2.1
*****************************************************************************/
int	main(void)
{
	#if SYSTICK_IRQ == POLLING_METHOD
	static uint32_t i = 0;
	#endif

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

	SN_GPIO2->MODE = 0x3;			//Set P2 as output

	MN_CT16B0_Init();

	SysTick_Init();
	
	WDT_Init();									//Set WDT reset overflow time ~ 250ms

	while (1)
	{
		__WDT_FEED_VALUE;
		
		#if SYSTICK_IRQ == POLLING_METHOD
		if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
		{
			if (++i == 50)
			{
				i = 0;
				SN_GPIO2->DATA ^= 0x2;		//Toggle P2.1
			}
		}
		#endif
	}
}

/*****************************************************************************
* Function		: MN_CT16B0_Init
* Description	: Init CT16B0 timer. 
* Input			: None
* Output		: None
* Return		: None
* Note			: Connect LEDs to P2.0 and P2.1
*****************************************************************************/
void MN_CT16B0_Init(void)
{
	CT16B0_Init();

	//Set MR0 value for 1 ms period  @ HCLK = 12MHz
	SN_CT16B0->MR0 = 0x2EE0;

	//Set MR0 match as TC stop, and enable MR0 interrupt
	SN_CT16B0->MCTRL = mskCT16_MR0RST_EN|mskCT16_MR0IE_EN;

	//Set CT16B0 as the up-counting mode.
	SN_CT16B0->TMRCTRL = (mskCT16_CRST|mskCT16_CM_EDGE_UP);

	//Wait until timer reset done.
	while (SN_CT16B0->TMRCTRL & mskCT16_CRST);

	//Let TC start counting.
	SN_CT16B0->TMRCTRL |= mskCT16_CEN_EN;
	
	//Enable CT16B0 NVIC interrupt
	NVIC_ClearPendingIRQ(CT16B0_IRQn);
	CT16B0_NvicEnable();
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
