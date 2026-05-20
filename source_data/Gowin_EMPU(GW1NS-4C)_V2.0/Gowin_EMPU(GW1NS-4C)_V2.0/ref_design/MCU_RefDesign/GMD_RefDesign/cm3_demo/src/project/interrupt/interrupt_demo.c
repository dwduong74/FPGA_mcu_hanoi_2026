/*
 ******************************************************************************************
 * @file      interrupt_demo.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Demo interrupt.
 ******************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "demo.h"


/* Definitions ---------------------------------------------------------------*/

#if INTERRUPT_DEMO

/* Includes: */
#include "uart.h"
#include <stdio.h>


/* Definitions: */

typedef unsigned char bool;
#define false 0
#define true  1

volatile uint32_t counter;
bool timer_flag = false;
bool counter_flag = false;


//Initializes Timer0
void timer_init(TIMER_TypeDef* TIMERx)
{
  TIMER0->INTCLEAR = 0;
  TIMER0->RELOAD   = 25000026;				//Set the value of the Timer 1s
  TIMER0->VALUE    = TIMER0->RELOAD; 	//Set the initialized value
  TIMER0->CTRL     = 0;  							//Timer interrupt

	printf("Initializes Timer0...\r\n");
}

//Application entry function
int interrupt_demo(void)
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
  timer_init(TIMER0); //Initializes Timer0

	//Enable interrupt
  nvic_irq_enable(TIMER0_IRQn, 1, 0);
	nvic_irq_enable(UART0_IRQn, 0, 0);

  TIMER_EnableIRQ(TIMER0);

	//Start Timer0
	TIMER_StartTimer(TIMER0);

	timer_flag = true;
  int num = 0;

	while(1)
   {
     if(counter == 2)
     {
       counter = 0;
			 printf("Counter number is %d.\r\n", num);
       num++;
     }

     if(num == 60)
	   {
			 num = 0;
		 }
  }
}

//UART0 interrupt handler function
void UART0_Handler(void)
{
	if(UART_GetRxIRQStatus(UART0) == SET)
	{
		printf("\r\nUART0 receives a value '%c' in interrupt.\r\n", UART_ReceiveChar(UART0));

		if(timer_flag == true)
		{
			counter_flag = true;
		}

		UART_ClearRxIRQ(UART0);

		if(timer_flag == false)
		{
			TIMER_StartTimer(TIMER0);	//Start Timer0
			timer_flag = true;
		}
	}
}

//Timer0 interrupt handler function
void TIMER0_Handler(void)
{
  if(TIMER_GetIRQStatus(TIMER0) != RESET)
  {
    while(1)
    {
		  counter ++;
		  printf("Timer0 interrupt counter is %d.\r\n", counter);

			if(counter_flag == true)
			{
				break;
			}
    }

		TIMER_ClearIRQ(TIMER0);
		TIMER_StopTimer(TIMER0);
		counter = 0;
		timer_flag = false;

		counter_flag = false;
  }
}

#endif
