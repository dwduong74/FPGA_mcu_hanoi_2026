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
#include ".\Interface\ADC\ADC.h"
#include ".\Utility\Utility.h"	

/*_____ D E C L A R A T I O N S ____________________________________________*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);
void ADC_SingleCH_SingleMode_Example(void);
void ADC_SingleCH_ContinuousMode_Example(void);
void ADC_MultipleCH_SingleMode_Example(void);
void ADC_MultipleCH_ContinuousMode_Example(void);
void ADC_Window_Watchdog_Example(void);
uint16_t hwADC_Value[22] = {0x0};   

/*_____ D E F I N I T I O N S ______________________________________________*/
#ifndef	SN32F407					//Do NOT Remove or Modify!!!
	#error Please install SONiX.SN32F4_DFP.1.0.1.pack or version >= 1.0.1
#endif
#define	PKG						SN32F407				//User SHALL modify the package on demand (SN32F407)

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

/*****************************************************************************
* Function		: main
* Description	: Demo code of ADC measuring. 
* Input			: None
* Output		: None
* Return		: None
* Note			: None
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

	ADC_SingleCH_SingleMode_Example();
	//ADC_SingleCH_ContinuousMode_Example();
	//ADC_MultipleCH_SingleMode_Example();
	//ADC_MultipleCH_ContinuousMode_Example();
	//ADC_Window_Watchdog_Example();

	while (1);
}

/*****************************************************************************
* Function		: ADC_SingleCH_SingleMode_Example
* Description	: Demo code of ADC Single channel + Single mode
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void ADC_SingleCH_SingleMode_Example(void)
{
	uint16_t wADC_ChSelect = ADC_CHS_AIN3;
	
	__ADC_INTERRUPT_FLAG_CLEAR;
	
	ADC_FuncInit(ADC_DIV32, ADC_12BIT, Single_Channel, Single_Mode);
	ADC_InterruptInit(EOCALIE_DIS, OVRIE_DIS, AWWIE_DIS, EOSIE_DIS, wADC_ChSelect);

	while (1)
	{
		if (ADC_Convert(wADC_ChSelect, ADC_FUNCTION_MODE1) == TRUE)
		{
			hwADC_Value[0] = ADC_Read();
		}
		else
		{
			while (1); //Fail
		}
		__ADC_INTERRUPT_FLAG_CLEAR;
	}
}

/*****************************************************************************
* Function		: ADC_SingleCH_SingleMode_Example
* Description	: Demo code of ADC Single channel + Continuous mode
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void ADC_SingleCH_ContinuousMode_Example(void)
{
	uint16_t wADC_ChSelect = ADC_CHS_AIN3;
	int i;
	
	__ADC_INTERRUPT_FLAG_CLEAR;

	ADC_FuncInit(ADC_DIV32, ADC_12BIT, Single_Channel, Continuous_Mode);
	ADC_InterruptInit(EOCALIE_DIS, OVRIE_EN, AWWIE_DIS, EOSIE_DIS, ADC_IE_AIN_DIS);

	while (1)
	{
		if (ADC_Convert(wADC_ChSelect, ADC_FUNCTION_MODE2) == TRUE)
		{
			for (i = 0; i < 22; i++)
			{
				hwADC_Value[i] = ADC_Read();
			}
		}
		else
		{
			while (1); //Fail
		}
		__ADC_INTERRUPT_FLAG_CLEAR;
	}	
}

/*****************************************************************************
* Function		: ADC_SingleCH_SingleMode_Example
* Description	: Demo code of ADC Multiple channel + Single mode
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void ADC_MultipleCH_SingleMode_Example(void)
{
	uint16_t wADC_ChSelect = (ADC_CHS_AIN1 | ADC_CHS_AIN3 | ADC_CHS_AIN5 | ADC_CHS_AIN7);
	int i;
	
	__ADC_INTERRUPT_FLAG_CLEAR;

	ADC_FuncInit(ADC_DIV32, ADC_12BIT, Multiple_Channel, Single_Mode);
	ADC_InterruptInit(EOCALIE_DIS, OVRIE_DIS, AWWIE_DIS, EOSIE_EN, ADC_IE_AIN_DIS);	

	while (1)
	{
		if (ADC_Convert(wADC_ChSelect, ADC_FUNCTION_MODE3) == TRUE)
		{
			for (i = 0; i < 4; i++)
			{
				hwADC_Value[i] = ADC_Read();
			}
		}
		else
		{
			while (1); //Fail
		}
		__ADC_INTERRUPT_FLAG_CLEAR;
	}
}

/*****************************************************************************
* Function		: ADC_SingleCH_SingleMode_Example
* Description	: Demo code of ADC Multiple channel + Continuous mode
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void ADC_MultipleCH_ContinuousMode_Example(void)
{
	uint16_t wADC_ChSelect = (ADC_CHS_AIN1 | ADC_CHS_AIN3 | ADC_CHS_AIN5 | ADC_CHS_AIN7);
	int i;
	
	__ADC_INTERRUPT_FLAG_CLEAR;

	ADC_FuncInit(ADC_DIV32, ADC_12BIT, Multiple_Channel, Continuous_Mode);
	ADC_InterruptInit(EOCALIE_DIS, OVRIE_EN, AWWIE_DIS, EOSIE_DIS, ADC_IE_AIN_DIS);	
	
	while (1)
	{
		if (ADC_Convert(wADC_ChSelect, ADC_FUNCTION_MODE4) == TRUE)
		{
			for (i = 0; i < 22; i++)
			{
				hwADC_Value[i] = ADC_Read();
			}
		}
		else
		{
			while (1); //Fail
		}
		__ADC_INTERRUPT_FLAG_CLEAR;
	}
}

/*****************************************************************************
* Function		: ADC_Window_Watchdog_Example
* Description	: Demo code of ADC window watchdog
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void ADC_Window_Watchdog_Example(void)
{
	uint16_t wADC_ChSelect = ADC_CHS_AIN1;
	uint16_t AWW_LT = 0x000, AWW_HT = 0x000;

	__ADC_INTERRUPT_FLAG_CLEAR;

	ADC_FuncInit(ADC_DIV32, ADC_12BIT, Single_Channel, Single_Mode);
	ADC_AWWInit(wADC_ChSelect, AWWMode1, AWW_LT, AWW_HT);
	ADC_InterruptInit(EOCALIE_DIS, OVRIE_DIS, AWWIE_EN, EOSIE_DIS, ADC_IE_AIN_DIS);	

	while (1)
	{
		if (ADC_Convert(wADC_ChSelect, ADC_FUNCTION_MODE5) == TRUE)
		{
			hwADC_Value[0] = ADC_Read();
		}
		else
		{
			while (1); //Fail
		}
		__ADC_INTERRUPT_FLAG_CLEAR;
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
