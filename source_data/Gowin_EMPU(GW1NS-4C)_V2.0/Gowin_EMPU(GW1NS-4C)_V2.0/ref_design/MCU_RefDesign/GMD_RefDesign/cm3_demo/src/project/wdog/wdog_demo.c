/*
 ******************************************************************************************
 * @file      wdog_demo.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Demo Watch Dog.
 ******************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "demo.h"


/* Definitions ---------------------------------------------------------------*/

#if WDOG_DEMO

/* Includes: */
#include "uart.h"
#include <stdio.h>


/* Definitions: */

//Update watchdog counter
void watchdog_set(unsigned int cycle)
{
  WDOG_UnlockWriteAccess();
  WDOG->CTRL = 2;
  WDOG_RestartCounter(cycle);
  WDOG_SetResetEnable();
  WDOG_LockWriteAccess();
}

//Initializes watchdog
/* type = 0 : No action */
/* type = 1 : Interrupt */
/* type = 2 : Reset     */
void watchdog_init(unsigned int cycle, int type)
{
  printf("Initializes watchdog.\r\n");

  WDOG_UnlockWriteAccess();
  WDOG_RestartCounter(cycle);

  if (type == 0)
  {
	printf("No action.\r\n");
    WDOG->CTRL = 0;
  }
  else if (type == 1)
  {
	printf("NMI interrupt.\r\n");
    WDOG_SetIntEnable();
  }
  else
  {
	printf("Reset.\r\n");
    WDOG_SetResetEnable();
    WDOG_SetIntEnable();
  }

  WDOG_LockWriteAccess();
}

//Application entry function
int wdog_demo(void)
{
  SystemInit();				//Initializes system clock
  uart_init(UART0,			//Initializes UART0
            38400,			//Baudrate
	        1,				//Tx
	        1,				//Rx
	        0,				//Tx interrupt
	        0,				//Rx interrupt
	        0,				//Tx overflow interrupt
	        0);				//Rx overflow interrupt
  watchdog_init(86666666, 2);	//Initializes watchdog

  while(1)
  {
    //Add monitor codes
    //Feed dogs
    printf("Watchdog feed!\r\n");
    watchdog_init(86666666, 2);
  }
}

#endif
