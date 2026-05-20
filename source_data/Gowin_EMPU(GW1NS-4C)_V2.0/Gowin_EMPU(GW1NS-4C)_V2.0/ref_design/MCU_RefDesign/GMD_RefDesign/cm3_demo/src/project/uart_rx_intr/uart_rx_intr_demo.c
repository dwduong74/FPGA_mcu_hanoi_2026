/*
 ******************************************************************************************
 * @file      uart_rx_intr_demo.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Demo UART RX interrupt.
 ******************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "demo.h"


/* Definitions ---------------------------------------------------------------*/

#if UART_RX_INTR_DEMO

/* Includes: */
#include "uart.h"
#include <stdio.h>


/* Definitions: */

//Application entry function
int uart_rx_intr_demo(void)
{
  SystemInit();       //Initializes system clock
  uart_init(UART0,    //Initializes UART0
	        38400,    //Baudrate
	        1,        //Tx
	        1,        //Rx
	        0,        //Tx interrupt
	        1,        //Rx interrupt
	        0,        //Tx overflow interrupt
	        0);       //Rx overflow interrupt

  printf("Please input a character to trigger UART receiving interrupt.\r\n\r\n");

  //Enable interrupt
  nvic_irq_enable(UART0_IRQn, 0, 0);

  while(1);
}

//UART0 interrupt handler function
void UART0_Handler(void)
{
  char num = '0';

  if(UART_GetRxIRQStatus(UART0) == SET)
  {
    num  = UART_ReceiveChar(UART0);
    printf("Receive a character '%c' in UART receiving interrupt...\r\n", num);
  }

  UART_ClearRxIRQ(UART0);
}

#endif
