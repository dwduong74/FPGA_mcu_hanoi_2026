/******************** (C) COPYRIGHT 2023 SONiX *******************************
* COMPANY:		SONiX
* DATE:				2023/11
* AUTHOR:			SA1
* IC:					SN32F400
*____________________________________________________________________________
*	REVISION	Date				User		Description
*	1.0				2023/11/06	SA1			1. First version released
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
#include "..\Driver\GPIO.h"
#include "..\Driver\WDT.h"
#include "..\Driver\SPI.h"
#include "..\Driver\CT16B1.h"
#include "..\Driver\Utility.h"
#include "..\Module\sst_flash.h"
#include "..\Module\Segment.h"
#include "..\Module\KeyScan.h"
/*_____ D E C L A R A T I O N S ____________________________________________*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);


/*_____ D E F I N I T I O N S ______________________________________________*/
#ifndef	SN32F407					//Do NOT Remove or Modify!!!
	#error Please install SONiX.SN32F4_DFP.0.0.18.pack or version >= 0.0.18
#endif
#define	PKG						SN32F407				//User SHALL modify the package on demand (SN32F407)

	
#define	EEPROM_WRITE_ADDR			0xa0
#define	EEPROM_READ_ADDR			0xa1	
/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/
uint32_t flash_addr = 0;
uint8_t write_flash_data[64];
uint8_t read_flash_data[64];
/*****************************************************************************
* Function		: main
* Description	: uart0 send the received data
* Input			: None
* Output		: None
* Return		: None
* Note			: Connect P3.1(UTXD0) and P3.2(URXD0)
*****************************************************************************/
int	main(void)
{
	uint16_t key_code;
	uint16_t addr;
	volatile uint16_t flash_id;
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
	//enable reset pin function
	SN_SYS0->EXRSTCTRL_b.RESETDIS = 0;	
	
	GPIO_Init();								//initial gpio
	
	WDT_Init();									//Set WDT reset overflow time ~ 250ms
	
	SPI0_Init();
	//select spi0 pin
	SN_PFPA->SPI0 = 2 | 2 <<2 | 2 << 4 | 2 <<6;
	CT16B1_Init();


// write data initial
	for(key_code=0;key_code<64;key_code++)
		write_flash_data[key_code] = key_code;
		
	flash_id = flash_read_id();
		
	//flash id should be 0xBF8D
	
	
	Digital_DisplayDEC(flash_addr);
	while (1)
	{
		__WDT_FEED_VALUE;
		if(timer_1ms_flag)
		{
			timer_1ms_flag = 0;
			Digital_Scan();
			key_code = KeyScan();
			
			if(key_code)
			{
				addr = flash_addr;
				switch(key_code & 0xff)
				{
					case KEY_1:   //ROW1  COL1   
						addr = flash_addr*10 + 7;
						break;
					case KEY_2:   //ROW1  COL2
						addr = flash_addr*10 + 8;
						break;
					case KEY_3:   //ROW1  COL3
						addr = flash_addr*10 + 9;
						break;
					case KEY_5:   //ROW2  COL1
						addr = flash_addr*10 + 4;
						break;
					case KEY_6:   //ROW2  COL2
						addr = flash_addr*10 + 5;
						break;
					case KEY_7:   //ROW2  COL3
						addr = flash_addr*10 + 6;
						break;
					case KEY_9:   //ROW3  COL1
						addr = flash_addr*10 + 1;
						break;
					case KEY_10:  //ROW3  COL2
						addr = flash_addr*10 + 2;
						break;
					case KEY_11:  //ROW3  COL3
						addr = flash_addr*10 + 3;
						break;
          case KEY_14:  //ROW4  COL2
						addr = flash_addr*10 + 0;
						break;
          
					case KEY_13:  //ROW4 COL1  clear addr
						addr = 0;
						break;  
          
					case KEY_4:
						flash_erase_sector(flash_addr);
						UT_DelayNms(20);
						flash_write(flash_addr,64,write_flash_data);
						break;

					case KEY_8:
            flash_read(flash_addr,64,read_flash_data);
						
						break;
				}
				if(addr != flash_addr)
				{
					if(addr <= 9999)
					{
						flash_addr = addr;
					}
					Digital_DisplayDEC(flash_addr);
				}
				
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
