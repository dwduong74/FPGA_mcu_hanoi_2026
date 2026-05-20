/******************** (C) COPYRIGHT 2024 SONiX *******************************
* COMPANY:		SONiX
* DATE:				2024/11
* AUTHOR:			SA1
* IC:					SN32F400
*____________________________________________________________________________
*	REVISION	Date				User		Description
*	1.0				2024/11/29	SA1			1. First version released
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
#include ".\Interface\I2C\I2C.h"
#include ".\Interface\SysTick\SysTick.h"
#include ".\Utility\Utility.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);

extern volatile uint8_t EEPROM_ADR_H, EEPROM_ADR_L;
extern volatile uint8_t EEPROM_WR;				// 0: Write, 1: Read
extern volatile uint8_t Busy;
extern volatile uint8_t bI2C0_TxFIFO_cnts; 
extern volatile uint8_t bI2C0_TxFIFO[I2C_TX_FIFO_LENGTH];
extern volatile uint8_t bI2C0_RxFIFO[I2C_RX_FIFO_LENGTH];

/*_____ D E F I N I T I O N S ______________________________________________*/
#ifndef	SN32F407					//Do NOT Remove or Modify!!!
	#error Please install SONiX.SN32F4_DFP.1.0.1.pack or version >= 1.0.1
#endif
#define	PKG						SN32F407				//User SHALL modify the package on demand (SN32F407)

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

/*****************************************************************************
* Function		: main
* Description	: Demo code of I2C R/W EEPROM
* Input			: None
* Output		: None
* Return		: None
* Note			: Connect I2C0 and EEPROM
*****************************************************************************/
int	main(void)
{
	uint8_t i = 0;

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

	SN_GPIO2->MODE = 0x0F;	
	SN_GPIO2->DATA = 0;

	for (i = 0; i < 32; i++)
	{
		bI2C0_TxFIFO[i] = (i * 2) + 1;
	}

	//System Interface Init
	SysTick_Init();						//init SysTick
	I2C0_Init();							//init I2C0 	

	while (1)
	{
	#if (EEPROM_less_than_32K == 0)						
		if (I2C0_Write(0x00, 32))								//page write, data word address = 0x00 and write 32 bytes data to EEPROM 					 
	#else	
		if (I2C0_Write(0x00, 8))								//page write, data word address = 0x00 and write 8 bytes data to EEPROM 
	#endif
		{
			UT_DelayNms (5);

		#if (EEPROM_less_than_32K == 0)
			if (I2C0_Read(0x00, 32))							//read 32 bytes data from EEPROM  	
		#else
			if (I2C0_Read(0x00, 8))								//read 8 bytes data from EEPROM 			 
		#endif
			{
				#if (EEPROM_less_than_32K == 0)
					for (i = 0; i < 32; i++)
				#else
					for (i = 0; i < 8; i++)
				#endif
					{
						if (bI2C0_RxFIFO[i] != bI2C0_TxFIFO[i])	 
						{
							while (1)													 //data incorrect
							{
								SN_GPIO2->BSET_b.BSET0 = 1;

								UT_DelayNms(50);		

								SN_GPIO2->BCLR_b.BCLR0 = 1;

								UT_DelayNms(50);					
							}
						}
					}
					while (1)																 //data correct
					{
						SN_GPIO2->BSET_b.BSET1 = 1;

						UT_DelayNms(50);

						SN_GPIO2->BCLR_b.BCLR1 = 1;

						UT_DelayNms(50);
					}
			}
			else
			{
				while (1)																 //read process error
				{
					SN_GPIO2->BSET_b.BSET3 = 1;

					UT_DelayNms(50);		

					SN_GPIO2->BCLR_b.BCLR3 = 1;

					UT_DelayNms(50);		
				}
			}
		}
		else
		{
			while (1)																 	 //write process error
			{
				SN_GPIO2->BSET_b.BSET2 = 1;

				UT_DelayNms(50);

				SN_GPIO2->BCLR_b.BCLR2 = 1;

				UT_DelayNms(50);
			}
		}
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
