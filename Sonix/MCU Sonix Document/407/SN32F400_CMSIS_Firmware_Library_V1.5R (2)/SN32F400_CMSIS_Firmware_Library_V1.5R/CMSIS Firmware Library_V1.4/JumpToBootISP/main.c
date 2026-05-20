/******************** (C) COPYRIGHT 2024 SONiX *******************************
* COMPANY:		SONiX
* DATE:				2024/11
* AUTHOR:			SA1
* IC:					SN32F400
*____________________________________________________________________________
*	REVISION	Date				User		Description
*	0.1				2024/11/26	SA1			1. Draft version released
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
#include "./Interface/SysTick/SysTick.h"
#include "./Interface/UART/UART.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ D E F I N I T I O N S ______________________________________________*/

/*_____ M A C R O S ________________________________________________________*/
#define IAP_GET_VER							0xA1

/*_____ F U N C T I O N S __________________________________________________*/
uint8_t bUARTcnt, bCheckSum;

void Goto_Bootloader(void);
void IAP_UART0_Init(void);

/*****************************************************************************
* Function		: main
* Description	: 
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
int	main(void)
{	
	SystemInit();
	SystemCoreClockUpdate();
	
	SN_GPIO2->MODE = 0x02;
	
	//Toggle P2.1 in Systick ISR
	SysTick_Init();
	
	//Initiallize UART0 setting as Baud = 921600, 8bit ,Odd parity, 1 stop bit
	IAP_UART0_Init();

	while (1)
	{
		if ((bUART0_RecvNew == 1) && (GulNum == 8))
		{
			bCheckSum = 0;
			//Calculate checksum
			for (bUARTcnt = 0; bUARTcnt < 7 ; bUARTcnt++)
			{
				bCheckSum += bUART0_RecvFIFO[bUARTcnt];
			}
			
			//If recevie IAP_GET_VER CMD means ISP start, jump to BL
			if ((bUART0_RecvFIFO[0] == IAP_GET_VER) && (bUART0_RecvFIFO[7] == bCheckSum))
			{
				//Disable all intterupt before jump to BL
				NVIC->ICER[0] = 0xFFFFFFFF;	
				//Jump to BL
				Goto_Bootloader();
			}
			bUART0_RecvNew = 0;
		}
	}
}

/*****************************************************************************
* Function		: Goto_Bootloader
* Description	: Jump to Boot Loader address.
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void Goto_Bootloader(void)
{	
	__ASM
	(
		"LDR	r0, =0x1fff00c1 \n"
		"BX		r0							\n"
	);
}

/*****************************************************************************
* Function		: IAP_UART0_Init
* Description	: Initialization of UART0 with SNLink IAP.
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void IAP_UART0_Init(void)
{
	SN_SYS1->AHBCLKEN |= UART0_CLK_EN;				//Enables clock for UART0
	
  //===Line Control===
	//setting character Word length(5/6/7/8 bit)
	SN_UART0->LC = (UART_CHARACTER_LEN8BIT		//8bit character length.
								|	UART_STOPBIT_1BIT					//stop bit of 1 bit
								|	UART_PARITY_BIT_EN			//parity bit is enable
								|	UART_PARITY_SELECTODD			//parity bit is odd
								|	UART_BREAK_DISEN					//Break Transmission control disable
								|	UART_DIVISOR_EN);					//Divisor Latch Access enable

	//===Baud Rate Calculation===
	//UART PCLK = 12MHz, Baud rate = 921600
	SN_UART0->FD = (UART_OVER_SAMPLE_8 | UART_MULVAL_7 | UART_DIVADDVAL_5);
  SN_UART0->DLM  = 0;
  SN_UART0->DLL  = 1;
	
	SN_UART0->LC &= ~(UART_DIVISOR_EN);		//Disable divisor latch

	//===FIFO Control===
	SN_UART0->FIFOCTRL =(UART_FIFO_ENABLE					//Enable USART FIFOs
											|	UART_RXFIFO_RESET				//RX FIFO Reset
											|	UART_TXFIFO_RESET				//TX FIFO Reset
											|	UART_RXTRIGGER_LEVEL1);		//RX Trigger Level(1 characters)

 	//===Interrupt Enable===
	UART0_InterruptEnable();

	//===UART Control===
	SN_UART0->CTRL =(UART_EN										//Enable UART0
									|	UART_RX_EN									//Enable RX
									| UART_TX_EN);								//Enable TX
	//===NVIC===
	NVIC_EnableIRQ(UART0_IRQn);			//Enable USART0 INT

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
	SN_SYS0->SWDCTRL_b.SWDDIS = 0;
	SN_GPIO3->MODE = 0x1;
	
	while (1)
	{
		SN_GPIO3->DATA_b.DATA0 ^= 1;
	}
//	NVIC_SystemReset();
}
