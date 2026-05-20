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
#include ".\Utility\Utility.h"
#include ".\Interface\CT\CT16.h"
#include ".\Interface\CT\CT16B0.h"
#include ".\Interface\CT\CT16B1.h"
#include ".\Interface\CT\CT16B5.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);
void MN_CtDemoCase0(void);
void MN_CtDemoCase1(void);
void MN_CtDemoCase7(void);
void MN_CtDemoCase8(void);
void MN_CtDemoCase9(void);
void MN_CtDemoCase10(void);

/*_____ D E F I N I T I O N S ______________________________________________*/
#ifndef	SN32F407					//Do NOT Remove or Modify!!!
	#error Please install SONiX.SN32F4_DFP.1.0.1.pack or version >= 1.0.1
#endif
#define	PKG						SN32F407				//User SHALL modify the package on demand (SN32F407)

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

/*****************************************************************************
* Function		: main
* Description	: Demo codes of CT timers. 
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

	//Select the timer demo case
	MN_CtDemoCase0();			//Use CT16B0 to demo MR0 match and stop TC function for 1-ms. 
//	MN_CtDemoCase1();			//Use CT16B1 to demo MR1 match and reset TC function for 1-ms period.
//	MN_CtDemoCase7();			//Use CT16B5 to demo PWM function.	
//	MN_CtDemoCase8();			//Use CT16B0 to demo PWM waveforms with External Match method.	
//	MN_CtDemoCase9();			//Use CT16B1 to demo the Counter function by using P1.0.	
//	MN_CtDemoCase10();		//CT16B0/CT16B1: Demo the Capture function(CT16B0) by PWM output(CT16B1).	

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
* Function		: MN_CtDemoCase0
* Description	: Use CT16B0 to demo MR0 match and stop TC function for 1-ms. 
*							  And use P1.0 to observe the duration between TC starting counting and stop.
*								User can observe almost a high pulse with 1ms on P1.0@HCLK=12MHz.
*								Also, after MR0 matching and stopping TC, no more TC counts.
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void MN_CtDemoCase0(void)
{
	CT16B0_Init();
	
	//Set MR0 value for 1ms period ==> count value = 1000*12 = 12000
	SN_CT16B0->MR0 = 12000;

	//Set MR0 match as TC stop, and enable MR0 interrupt
	SN_CT16B0->MCTRL = (mskCT16_MR0STOP_EN | mskCT16_MR0IE_EN);

	//Set CT16B0 as the up-counting mode.
	SN_CT16B0->TMRCTRL = (mskCT16_CRST | mskCT16_CM_EDGE_UP);

	//Wait until timer reset done.
	while (SN_CT16B0->TMRCTRL & mskCT16_CRST);
	
	//Use P1.0 to indicate CT16B0 MR0 matches and TC stop. 
	SN_GPIO1->MODE = 0x01;	// Set P1.0 as output.
	SN_GPIO1->DATA = 0x01;	// Default set P1.0 as output high.

	//Let TC start counting.
	SN_CT16B0->TMRCTRL |= mskCT16_CEN_EN;
	
	//Enable CT16B0's NVIC interrupt.
	CT16B0_NvicEnable();
	
	while (1)
	{
		if (iwCT16B0_IrqEvent == mskCT16_MR0IF)	//Check if MR0 match interrupt occurs
		{
			iwCT16B0_IrqEvent = 0;		//Clear MR0 match interrupt variable.
			SN_GPIO1->DATA = 0x00;		//Set P1.0 as output low.
			break;
		}
	}
	while (SN_CT16B0->TC == 12000);//Pass: TC should be always equal to 12000.
	while (1);										 //Fail: Can't run here.
}

/*****************************************************************************
* Function		: MN_CtDemoCase1
* Description	: Use CT16B1 to demo MR1 match and reset TC function for 1-ms period. 
*							  And toggle P1.0 when MR1 match interrupt occurs.
*								User can observe almost 500KHz waveform on P1.0@HCLK = 12MHz.
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void MN_CtDemoCase1(void)
{
	CT16B1_Init();
	
	//Set MR1 value for 1ms period ==> count value = 1000*12 = 12000
	SN_CT16B1->MR1 = 12000;	

	//Set MR1 match as TC RESET, and enable MR1 interrupt
	SN_CT16B1->MCTRL = (mskCT16_MR1RST_EN | mskCT16_MR1IE_EN);

	//Set CT16B0 as the up-counting mode.
	SN_CT16B1->TMRCTRL = (mskCT16_CRST | mskCT16_CM_EDGE_UP);

	//Wait until timer reset done.
	while (SN_CT16B1->TMRCTRL & mskCT16_CRST);
	
	//Let TC start counting.
	SN_CT16B1->TMRCTRL |= mskCT16_CEN_EN;
	
	//Enable CT16B1's NVIC interrupt.
	CT16B1_NvicEnable();
	
	//Use P1.0 toggle when CT16B1 MR1 matches and TC reset. 
	SN_GPIO1->MODE = 0x01;	//Set P1.0 as output.
	SN_GPIO1->DATA = 0x00;	//Default set P1.0 as output low.
	
	while (1)
	{
		if (iwCT16B1_IrqEvent == mskCT16_MR1IF)	//Check if MR1 match interrupt occurs
		{
			iwCT16B1_IrqEvent = 0;		//Clear MR1 match interrupt variable.
			SN_GPIO1->DATA ^= 0x01;		//Toggle P1.0.
		}
	}
}

/*****************************************************************************
* Function		: MN_CtDemoCase7
* Description	: Use CT16B5 to demo PWM function. 
*							  The total PWM period 1ms@HCLK = 12MHz is determined by MR9 setting with 12000.
*               And PWM0/PWM1 has the duty of 10% and 30% respectively.
*								PWM0/PWM1 pins will output to P1.1 and P1.2.
*								User can observe the PWM waveforms on those I/O pins.
*								Also, demo case will toggle P1.0 in main loop once MR9 match interrupt occurs. 
* Input			: None
* Output		: None
* Return		: None
* Note			: Demo waveforms are as below:
*
*								<------ 1ms -------><------ 1ms -------><------ 1ms ------->
*		P1.0				____________________--------------------____________________
*																	10%									10%									10%
*		PWM0(P1.1)  __________________--__________________--__________________--
*																	30%									30%									30%
*		PWM1(P1.2)	______________------______________------______________------
*
*****************************************************************************/
void MN_CtDemoCase7(void)
{
	CT16B5_Init();

	//Set MR9 value for 1ms PWM period ==> count value = 1000*12 = 12000
	SN_CT16B5->MR9 = 12000;
	
	//Set MR0 value for 10% duty ==> count value = 12000 - (10%*12000) = 10800
	SN_CT16B5->MR0 = 10800;

	//Set MR1 value for 30% duty ==> count value = 12000 - (30%*12000) = 8400
	SN_CT16B5->MR1 = 8400;
	
	//Enable PWM function, IOs and select the PWM modes
	SN_CT16B5->PWMCTRL = 
		(mskCT16_PWM0EN_EN | mskCT16_PWM0MODE_1 | mskCT16_PWM0IOEN_EN)|	//Enable PWM0 function, IO and select as PWM mode 1
		(mskCT16_PWM1EN_EN | mskCT16_PWM1MODE_1 | mskCT16_PWM1IOEN_EN);	//Enable PWM1 function, IO and select as PWM mode 1

	//Set MR9 match interrupt and TC rest 
	SN_CT16B5->MCTRL = (mskCT16_MR9IE_EN | mskCT16_MR9RST_EN);

	//Set CT16B3 as the up-counting mode.
	SN_CT16B5->TMRCTRL = (mskCT16_CRST | mskCT16_CM_EDGE_UP);

	//Wait until timer reset done.
	while (SN_CT16B5->TMRCTRL & mskCT16_CRST);
	
	//Let TC start counting.
	SN_CT16B5->TMRCTRL |= mskCT16_CEN_EN;
	
	//Enable CT16B5's NVIC interrupt.
	CT16B5_NvicEnable();
	
	//Use P1.0 to indicate CT16B5 MR9 match interrupt.
	SN_GPIO1->MODE = 0x01;	//Set P1.0 as output.
	SN_GPIO1->DATA = 0x00;	//Default set P1.0 as output low.
	
	while (1)
	{
		if (iwCT16B5_IrqEvent == mskCT16_MR9IF) //Check if MR9 match interrupt occurs
		{
			iwCT16B5_IrqEvent = 0;		//Clear MR9 match interrupt variable.
			SN_GPIO1->DATA ^= 0x01;		//Toggle P1.0
		}	
	}
}

/*****************************************************************************
* Function		: MN_CtDemoCase8
* Description	: Use CT16B0 to demo PWM waveforms with External Match method. 
*							  The total PWM period 1ms@HCLK = 12MHz is determined by MR9 setting with 12000.
*               And let PWM0/PWM1 output to generate the phase delay of 0% and 30%.
*								PWM0/PWM1 pins will output to P0.8 and P0.9.
*								User can observe the PWM waveforms on those I/O pins.
*								Also, this demo case will toggle P1.0 in main loop once MR3 match interrupt occurs. 
* Input			: None
* Output		: None
* Return		: None
* Note			: Demo waveforms are as below:
*
*								<------ 1ms -------><------ 1ms -------><------ 1ms -------><------ 1ms ------->
*		P1.0				____________________--------------------____________________--------------------
*																	 0%									 0%									0%
*		PWM0(P0.8) --------------------____________________--------------------____________________
*																	30%									30%									30%
*		PWM1(P0.9)	______--------------------____________________--------------------______________
*
*****************************************************************************/
void MN_CtDemoCase8(void)
{
	CT16B0_Init();

	//Set MR9 value for 1ms PWM period ==> count value = 1000*12 = 12000
	SN_CT16B0->MR9 = 12000;

	//Set MR0 value for 0% phase delay ==> count value = 0
	SN_CT16B0->MR0 = 0;

	//Set MR1 value for 30% phase delay ==> count value = 30%*12000 = 3600
	SN_CT16B0->MR1 = 3600;
	
	//Set External Match for EM0, EM1 as toggle
	SN_CT16B0->EM =
		(mskCT16_EMC0_TOGGLE)|	//Enable PWM0 External Match to toggle PWM0.
		(mskCT16_EMC1_TOGGLE);	//Enable PWM1 External Match to toggle PWM1.
	
	//Enable PWM External Match and IOs.
	SN_CT16B0->PWMCTRL = 
		(mskCT16_PWM0EN_EM0 | mskCT16_PWM0IOEN_EN)|	//Enable PWM0 External Match and IO.
		(mskCT16_PWM1EN_EM1 | mskCT16_PWM1IOEN_EN);	//Enable PWM1 External Match and IO.

	//Set MR3 match interrupt and TC rest 
	SN_CT16B0->MCTRL = (mskCT16_MR9IE_EN | mskCT16_MR9RST_EN);

	//Set CT16B4 as the up-counting mode.
	SN_CT16B0->TMRCTRL = (mskCT16_CRST | mskCT16_CM_EDGE_UP);

	//Wait until timer reset done.
	while (SN_CT16B0->TMRCTRL & mskCT16_CRST);			
	
	//Let TC start counting.
	SN_CT16B0->TMRCTRL |= mskCT16_CEN_EN;	

	//Enable CT16B0's NVIC interrupt.
	CT16B0_NvicEnable();
	
	//Use P1.0 to indicate CT16B0 MR9 match interrupt.
	SN_GPIO1->MODE = 0x01;	//Set P1.0 as output.
	SN_GPIO1->DATA = 0x00;	//Default set P1.0 as output low.
	
	while (1)
	{
		if (iwCT16B0_IrqEvent == mskCT16_MR9IF) //Check if MR9 match interrupt occurs
		{
			iwCT16B0_IrqEvent = 0;		//Clear MR9 match interrupt variable.
			SN_GPIO1->DATA ^= 0x01;		//Toggle P1.0
		}	
	}
}

/*****************************************************************************
* Function		: MN_CtDemoCase9
* Description	: Use CT16B1 to demo the Counter function by using P1.0. Before  
*								running this demo, user should manually connect P1.0 and the  
*								CAP0 input pin (P3.8)of CT16B1.
*							  This demo shows the function of Rising Edge Counter that P1.0  
*								toggles 100 times and is the rising source of CAP0 input. 
*               The code will check whether the toggle counter matches the CAP0
*								Counter value in TC register. 
*								Also, this demo case will enable MR0 match interrupt to double  
*								check that TC matches MR0 will trigger MR0 interrupt. This could
*								also be used for the application of CAP0 Counter threshold checking.
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void MN_CtDemoCase9(void)
{
	uint32_t wToggleCnt = 0;		//Indicates the I/O toggle count.
	
	//Let P1.0 be the CAP0 counter's input source.
	SN_GPIO1->MODE = 0x01;	//Set P1.0 as output mode.
	SN_GPIO1->DATA = 0x00;	//Set P1.0 as output low.
	
	CT16B1_Init();

	//Set MR0 match value as 100.
	SN_CT16B1->MR0 = 100;

	//Set Counter Control as rising edge counter mode.
	SN_CT16B1->CNTCTRL = mskCT16_CTM_CNTER_RISING;

	//Enable CAP0 function.
	SN_CT16B1->CAPCTRL = mskCT16_CAP0EN_EN;

	//Enable MR0 match interrupt.
	SN_CT16B1->MCTRL = mskCT16_MR0IE_EN;

	//Set CT16B1 to reset TC value.
	SN_CT16B1->TMRCTRL = mskCT16_CRST;

	//Wait until timer reset done.
	while (SN_CT16B1->TMRCTRL & mskCT16_CRST);			
	
	//Let TC start CAP0 Counter function.
	SN_CT16B1->TMRCTRL |= mskCT16_CEN_EN;						

	//Enable CT16B1's NVIC interrupt.
	CT16B1_NvicEnable();
		
	//Let P1.0 toggle 100 times to generate 100 times of rising edge.
	for (wToggleCnt = 0; wToggleCnt < 100; wToggleCnt++)
	{
		SN_GPIO1->BSET = 0x01; 			//Set P1.0 as output high.
		UT_DelayNx10us(1);					//Delay 10us
		SN_GPIO1->BCLR = 0x01; 			//Set P1.0 as output low.
		UT_DelayNx10us(1);					//Delay 10us
	}
	
	// Check whether TC matches MR0
	if (iwCT16B1_IrqEvent == mskCT16_MR0IF)
	{
		iwCT16B1_IrqEvent = 0;	//Clear MR0 match interrupt variable.
		//Check whether wToggleCnt is equal to TC value.
		if (SN_CT16B1->TC == wToggleCnt)
		{
			while (1);								//Pass point
		}
	}
	while (1);										//Fail point
}

/****************************************************************************
* Function		: MN_CtDemoCase10
* Description	: CT16B0/CT16B1: Demo the Capture function(CT16B0) by PWM output(CT16B1).
*								Before running this demo, user should manually connect CT16B0 input
*								pin (P3.9) and CT16B1 PWM0 output pin(P2.10).
*							  This demo will let PWM output 1KHz waveform to CAP0 pin. It will 
*               check whether timer counter capture values matches 1KHz toggle rate which
*								should be close to 12000@HCLK = 12MHz.
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void MN_CtDemoCase10(void)
{
	uint32_t wCnt = 0;				//Indicates capture count times.
	uint16_t wPreCap0 = 0;		//Indicates the previous  CAP0 value.
	uint16_t wCap0Buffer[20];	//Used to save delta CAP0 values of 20 times.
	
	/***** Step 1: Setup the 1KHz toggle in CT16B1. *****/
	CT16B1_Init();

	//Set MR0 match value for 1KHz toggle rate output by External Match toggle method.
	//Thus, the match value should be (12000 - 1)@HCLK = 12MHz.
	SN_CT16B1->MR0 = (12000 - 1);
  //Set External Match for EM0 toggle.
	SN_CT16B1->EM = mskCT16_EMC0_TOGGLE;
	//Enable PWM0 External Match and IO.
	SN_CT16B1->PWMCTRL = mskCT16_PWM0IOEN_EN;
	//Set MR0 match to reset TC. 
	SN_CT16B1->MCTRL = mskCT16_MR0RST_EN;
	//Set CT32B0 as the up-counting mode.
	SN_CT16B1->TMRCTRL = (mskCT16_CRST | mskCT16_CM_EDGE_UP);

	/***** Step 2: Setup CAP0 timer capture in CT16B0.*****/ 
	CT16B0_Init();

	//Set Counter Control as timer counter mode.
	SN_CT16B0->CNTCTRL = (mskCT16_CTM_TIMER);

	//Enable CAP0 function.
	SN_CT16B0->CAPCTRL = 
			(mskCT16_CAP0RE_EN)|	//Enable rising edge capture.
			(mskCT16_CAP0FE_EN)|	//Enable falling dge capture.
			(mskCT16_CAP0IE_EN)|	//Enable CAP0 interrupt.
			(mskCT16_CAP0EN_EN);	//Enable CAP0 input function.

	//Set CT16B0 to reset TC value.
	SN_CT16B0->TMRCTRL = mskCT16_CRST;
	
	//Wait until timer reset done.
	while (SN_CT16B0->TMRCTRL & mskCT16_CRST);
	
	//Let TC start CAP0 Counter function.
	SN_CT16B0->TMRCTRL |= mskCT16_CEN_EN;
	
	//Enable CT16B0's NVIC interrupt.
	//CT16B0_NvicEnable();

	/***** Step 3: Start to toggle 1KHz from CT16B1 PWM0. *****/
	//Wait until timer reset done.
	while (SN_CT16B1->TMRCTRL & mskCT16_CRST);

	//Let TC start PWM function.
	SN_CT16B1->TMRCTRL |= mskCT16_CEN_EN;

	/***** Step 4: Let CT16B0 CAP0 capture for 20 times *****/
	//Ignore the first time of CAP0 capture but save it first.
	while (1)
	{
		if (SN_CT16B0->RIS & mskCT16_CAP0IF)//Check whether CAP0 capture has done
		{
			SN_CT16B0->IC = mskCT16_CAP0IC;	//Clear CAP0 status.
			wPreCap0 = SN_CT16B0->CAP0;			//Save the CAP0 value.
			break;
		}
	}

	//Keep retrieving the delta value of each CAP0 status occurs for 20 times.
	while (1)
	{
		if (SN_CT16B0->RIS & mskCT16_CAP0IF)//Check whether CAP0 capture has done
		{
			wCap0Buffer[wCnt] = SN_CT16B0->CAP0 - wPreCap0;	//Save the delta value into wCap0Buffer
			wPreCap0 = SN_CT16B0->CAP0;			//Save the CAP0 value.
			SN_CT16B0->IC = mskCT16_CAP0IC;	//Clear CAP0 status.
			wCnt++;													//Increase loop count.
			if (wCnt == 20)									//Check if already retrieving 20 times
			{
				break;
			}
		}
	}
	//Check whether all CAP0 delta in wCap0Buffer matches 12000. 
	for (wCnt = 0; wCnt < 20; wCnt++)
	{
		if (wCap0Buffer[wCnt] != 12000)//Check if matches 12000.
		{
			while (1);								//Fail point
		}
	}
	while (1);										//Pass point
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
