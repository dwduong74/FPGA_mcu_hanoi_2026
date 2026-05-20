/******************** (C) COPYRIGHT 2024 SONiX *******************************
* COMPANY:		SONiX
* DATE:				2024/12
* AUTHOR:			SA1
* IC:					SN32F400
*____________________________________________________________________________
*	REVISION	Date				User		Description
*	1.0				2024/12/02	SA1			1. First version released
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
#include "Interface\SPI\SPI.h"
#include "Utility\Utility.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);
void SPI0_NBytesTxRxIrp(uint32_t);

/*_____ D E F I N I T I O N S ______________________________________________*/
#ifndef	SN32F407					//Do NOT Remove or Modify!!!
	#error Please install SONiX.SN32F4_DFP.1.0.1.pack or version >= 1.0.1
#endif
#define	PKG						SN32F407				//User SHALL modify the package on demand (SN32F407)


#define SPI_TEST_CNT	32
	
uint16_t hwSPI_Tx_Fifo[SPI_TEST_CNT];
uint16_t hwSPI_Rx_Fifo[SPI_TEST_CNT];
uint32_t wSPI_NBytes = 0;
uint32_t wSPI_Send_Pointer = 0;
uint32_t wSPI_Get_Pointer = 0;

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

/*****************************************************************************
* Function		: main
* Description	: SPI0 is master. (Connect MISO0 & MOIS0) 
								Exapmle Mode is SPI (CPHA=0, CPOL=0, MLS=0).
								Change mode by setting FORMAT, CPHA, CPOL, MLSB.
* Input			: None
* Output		: None
* Return		: None
* Note			: Connect MISO0 and MOSI0 
*****************************************************************************/
int	main(void)
{
	uint32_t i = 0, j = 0;

	//User can configure System Clock with Configuration Wizard in system_SN32F800.c
	SystemInit();
	SystemCoreClockUpdate();				//Must call for SN32F800, Please do NOT remove!!!

	//Note: User can refer to ClockSwitch sample code to switch various HCLK if needed.

	PFPA_Init();										//User shall set PFPA if used, do NOT remove!!!

	//1. User SHALL define PKG on demand.
	//2. User SHALL set the status of the GPIO which are NOT pin-out to input pull-up.
	NotPinOut_GPIO_init();

	//--------------------------------------------------------------------------
	//User Code starts HERE!!!

	//System Interface Init
	SPI0_Init();

	//Write TX FIFO
	for (i = 0; i < SPI_TEST_CNT; i++)
	{
		hwSPI_Tx_Fifo[i] = i;
	}	

	//Start Transmission
	j = SPI_TEST_CNT;

	SPI0_NBytesTxRxIrp(j);

	//Check data 
	for (i = 0; i < j; i++)
	{
		if (hwSPI_Tx_Fifo[i] != hwSPI_Rx_Fifo[i])
		{
			while (1);	//Error
		}		
	}
	while (1); //Pass
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

/***************************************************************************************************************
* Function		: SPI0_NBytesTxRxIrp
* Description	: None
* Input			: N_Bytes
* Output		: None
* Return		: None
* Note			: Avoid all of the interrupts which may make FW can't fill in TX FIFO (SN_SPI0->DATA=...) in time.
***************************************************************************************************************/
void SPI0_NBytesTxRxIrp(uint32_t N_Bytes)
{
	wSPI_NBytes = N_Bytes; 

	SN_SPI0->IE_b.TXFIFOTHIE = SPI_TXFIFOTHIE_EN;

	while (wSPI_Send_Pointer != wSPI_NBytes);	

	while (1)
	{
		while (SN_SPI0->STAT & mskSPI_RX_EMPTY); //Get all remaining data				

		hwSPI_Rx_Fifo[wSPI_Get_Pointer++] = SN_SPI0->DATA;

		if (wSPI_Get_Pointer == wSPI_NBytes)
		{
			break;
		}
	}

	__SPI0_SET_SEL0;										//SEL is high

	//Reset Variable
	wSPI_Send_Pointer = 0;
	wSPI_Get_Pointer = 0;
}

/*****************************************************************************
* Function		: SPI0_IRQHandler
* Description	: None
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void SPI0_IRQHandler(void)
{
	__SPI0_CLR_SEL0;										//SEL is low

	SN_SPI0->DATA = hwSPI_Tx_Fifo[wSPI_Send_Pointer++];

	if (!(SN_SPI0->STAT & mskSPI_RX_EMPTY))		//Check having any data in RXFIFO
	{	
		hwSPI_Rx_Fifo[wSPI_Get_Pointer++] = SN_SPI0->DATA;
	}	

	if (wSPI_Send_Pointer == wSPI_NBytes)
	{
		SN_SPI0->IE_b.TXFIFOTHIE = SPI_TXFIFOTHIE_DIS;	//TX FIFO threshold interrupt disable
	}		

	SN_SPI0->IC = mskSPI_TXFIFOTHIC;	//Clear overFlow flag
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
