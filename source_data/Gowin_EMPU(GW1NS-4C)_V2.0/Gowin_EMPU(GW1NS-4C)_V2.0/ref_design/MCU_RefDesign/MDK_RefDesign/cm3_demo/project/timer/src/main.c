/*
 ******************************************************************************************
 * @file      main.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Main program body.
 ******************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "uart.h"
#include <stdio.h>


/* Definitions ---------------------------------------------------------------*/

volatile uint32_t counter;


//Initializes Timer0
void timer_init(TIMER_TypeDef* TIMERx)
{
  TIMERx->INTCLEAR = 0;
  TIMER0->RELOAD   = 25000026;					//Set the value of the Timer 1s
  TIMER0->VALUE    = TIMER0->RELOAD; 		//Set the initial value
  TIMER0->CTRL     = 0;  								//Timer interrupt
	
	printf("Initializes Timer0...\r\n");
}

//Timer0 interrupt handler function
void TIMER0_Handler(void)
{
  if(TIMER_GetIRQStatus(TIMER0) != RESET)
  {
    counter++;
    TIMER_ClearIRQ(TIMER0);
  }
}

//Application entry function
int main(void)
{
  int cnt_num = 0;

  SystemInit();       //Initializes system clock
  uart_init(UART0,    //Initializes UART0
	          38400,    //Baudrate
	          1,        //Tx
	          1,        //Rx
	          0,        //Tx interrupt
	          0,        //Rx interrupt
	          0,        //Tx overflow interrupt
	          0);       //Rx overflow interrupt
  timer_init(TIMER0); //Initializes Timer0

  counter = 0;
	
	//Enable NVIC and Timer0 interrupt
  nvic_irq_enable(TIMER0_IRQn, 0, 0);
  TIMER_EnableIRQ(TIMER0);

	//Start Time0
	printf("Start Timer0...\r\n\r\n");
  TIMER_StartTimer(TIMER0);

  while(1)
  {
    if(counter == 2)
    {
      counter = 0;
      printf("Counter number: %d\r\n", cnt_num);
      cnt_num++;
    }

    if(cnt_num == 60)
    {
      cnt_num = 0;
    }
  }
}
