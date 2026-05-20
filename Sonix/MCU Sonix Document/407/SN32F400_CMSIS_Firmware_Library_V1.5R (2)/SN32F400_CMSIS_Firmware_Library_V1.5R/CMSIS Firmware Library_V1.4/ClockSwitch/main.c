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
#include ".\System\SYS_con_drive.h"


/*_____ D E C L A R A T I O N S ____________________________________________*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);


/*_____ D E F I N I T I O N S ______________________________________________*/
#ifndef	SN32F407					//Do NOT Remove or Modify!!!
	#error Please install SONiX.SN32F4_DFP.1.0.1.pack or version >= 1.0.1
#endif
#define	PKG						SN32F407				//User SHALL modify the package on demand (SN32F407)

#define	SWITCH_CASE		SYS0_CLK_IHRC		//SYS0_CLK_IHRC, SYS0_CLK_ILRC, SYS0_CLK_EHS, SYS0_CLK_ELS, SYS0_CLK_PLL

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

/*****************************************************************************
* Function		: main
* Description	: Clock Switch & Clockout demo code. 
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
int	main(void)
{
	PFPA_Init();										//User shall set PFPA if used, do NOT remove!!!

	//1. User SHALL define PKG on demand.
	//2. User SHALL set the status of the GPIO which are NOT pin-out to input pull-up.
	NotPinOut_GPIO_init();

	//--------------------------------------------------------------------------
	//User Code starts HERE!!!

	#if (SWITCH_CASE == SYS0_CLK_IHRC)
	SYS0_EnableIHRC(SYS0_IHRC_12MHZ);
	SYS0_SystemClockSwitch(SYS0_SYSCLK_IHRC, SYS0_AHBPRE_DIV1, SYS0_SYSCLK_DIV1);
	SYS0_SystemCoreClockUpdate(12000000);		//Note: Must be called with Exact HCLK frequency, Do NOT remove!!!
	#endif
	
	#if (SWITCH_CASE == SYS0_CLK_ILRC)
	SYS0_SystemClockSwitch(SYS0_SYSCLK_ILRC, SYS0_AHBPRE_DIV2, SYS0_SYSCLK_DIV1);
	SYS0_SystemCoreClockUpdate(16000);				//Note: Must be called with Exact HCLK frequency, Do NOT remove!!!
	#endif
	
	#if (SWITCH_CASE == SYS0_CLK_EHS)
	SYS0_EnableEHSXtal(16);
	SYS0_SystemClockSwitch(SYS0_CLK_EHS, SYS0_AHBPRE_DIV1, SYS0_SYSCLK_DIV1);
	SYS0_SystemCoreClockUpdate(16000000);		//Note: Must be called with Exact HCLK frequency, Do NOT remove!!!
	#endif
	
	#if (SWITCH_CASE == SYS0_CLK_ELS)
	SYS0_EnableELSXtal();
	SYS0_SystemClockSwitch(SYS0_CLK_ELS, SYS0_AHBPRE_DIV1, SYS0_SYSCLK_DIV1);
	SYS0_SystemCoreClockUpdate(32768);			//Note: Must be called with Exact HCLK frequency, Do NOT remove!!!
	#endif
	
	#if (SWITCH_CASE == SYS0_CLK_PLL)
	//Example: EHS=16MHz ---> PLL=48MHz
	//Note: User shall call SYS0_EnableEHSXtal or SYS0_EnableIHRC to make sure
  //			the clock source of PLL is enabled and ready.	
	SYS0_EnableEHSXtal(16);
	SYS0_EnablePLL(6, SYS0_PLL_P2, SYS0_PLL_F1, SYS0_PLL_CLOCK_EHS);
	SYS0_SystemClockSwitch(SYS0_CLK_PLL, SYS0_AHBPRE_DIV1, SYS0_SYSCLK_DIV1);
	SYS0_SystemCoreClockUpdate(48000000);		//Note: Must be called with Exact HCLK frequency, Do NOT remove!!!
	#endif

	//Clockout (Optional)
	SYS1_EnableClockout(SYS1_CLOCKOUT_HCLK, SYS1_CLOCKOUT_DIV2);

	while (1);
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
