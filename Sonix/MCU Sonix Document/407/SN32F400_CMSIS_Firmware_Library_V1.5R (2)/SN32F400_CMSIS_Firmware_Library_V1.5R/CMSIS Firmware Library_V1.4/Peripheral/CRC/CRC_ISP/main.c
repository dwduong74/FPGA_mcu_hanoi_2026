/******************** (C) COPYRIGHT 2024 SONiX *******************************
* COMPANY:		SONiX
* DATE:				2024/11
* AUTHOR:			SA1
* IC:					SN32F400
*____________________________________________________________________________
*	REVISION	Date				User		Description
*	1.0				2024/11/27	SA1			1. First version released
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
#include ".\Interface\CRC\CRC.h"
#include ".\Interface\Flash\Flash.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);

volatile uint32_t wCRC_RomResult[1];
volatile uint32_t wFlashBackup[128];

/*_____ D E F I N I T I O N S ______________________________________________*/
#ifndef	SN32F407					//Do NOT Remove or Modify!!!
	#error Please install SONiX.SN32F4_DFP.1.0.1.pack or version >= 1.0.1
#endif
#define	PKG						SN32F407				//User SHALL modify the package on demand (SN32F407)

#define	ISP_TARGET_ADDR		0x00007E00				//Word-alignment

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

/*****************************************************************************
* Function		: main
* Description	: Demo codes of CRC. 
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
int	main(void)
{
	volatile uint32_t i, index = 0;
	volatile uint32_t CRC_BKP;
	
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

	//1. Calucation for ROM CRC
	CRC_Enable();

	SN_CRC->CTRL = (mskCRC_RESET | mskCRC_TYPE);

	if ((mskCRC_CALC_MODE >> 3) == CRC_CALC_ROM)
	{
		SN_CRC->CTRL |= mskCRC_RESET;
		wCRC_RomResult[0] = CRC_Calc_ROM();
	}

	//2. Backup data @ISP_TARGET_ADDR		
	for (i = ISP_TARGET_ADDR; i < (ISP_TARGET_ADDR + FLASH_PAGE_SIZE); i += 4)
	{
		wFlashBackup[index++] = *((uint32_t *)i);
		
		__NOP();
	}

	//3. Erase page address @ISP_TARGET_ADDR
	FLASH_EraseSector(ISP_TARGET_ADDR);
	
	//4. Program content of CRC data to User ROM address ISP_TARGET_ADDR
	wFlashBackup[12] = wCRC_RomResult[0];

	if (FLASH_ProgramPage(ISP_TARGET_ADDR, FLASH_PAGE_SIZE, (uint8_t *)wFlashBackup) == FAIL)
	{
		while (1);	//Program Fail
	}
	
	CRC_BKP = *(volatile unsigned long *)(ISP_TARGET_ADDR + 0x30);
	if (CRC_BKP != wCRC_RomResult[0])
	{
		while (1); //CRC check Fail
	}

	while (1); //CRC check Pass
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
