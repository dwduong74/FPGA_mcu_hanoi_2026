/*
 ******************************************************************************************		
 * @file      board.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Board supported.
 ******************************************************************************************
 */

#include <stdint.h>
#include <rthw.h>
#include <rtthread.h>
#include "gpio.h"
#include "uart.h"


#define _SCB_BASE       (0xE000E010UL)
#define _SYSTICK_CTRL   (*(rt_uint32_t *)(_SCB_BASE + 0x0))
#define _SYSTICK_LOAD   (*(rt_uint32_t *)(_SCB_BASE + 0x4))
#define _SYSTICK_VAL    (*(rt_uint32_t *)(_SCB_BASE + 0x8))
#define _SYSTICK_CALIB  (*(rt_uint32_t *)(_SCB_BASE + 0xC))
#define _SYSTICK_PRI    (*(rt_uint8_t  *)(0xE000ED23UL))

// Updates the variable SystemCoreClock and must be called 
// whenever the core clock is changed during program execution.
extern void SystemCoreClockUpdate(void);

// Holds the system core clock, which is the system clock 
// frequency supplied to the SysTick timer and the processor 
// core clock.
extern uint32_t SystemCoreClock;

static uint32_t _SysTick_Config(rt_uint32_t ticks)
{
    if ((ticks - 1) > 0xFFFFFF)
    {
        return 1;
    }
    
    _SYSTICK_LOAD = ticks - 1; 
    _SYSTICK_PRI = 0xFF;
    _SYSTICK_VAL  = 0;
    _SYSTICK_CTRL = 0x07;  
    
    return 0;
}

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
#define RT_HEAP_SIZE 1024
static uint32_t rt_heap[RT_HEAP_SIZE];     // heap default size: 4K(1024 * 4)
RT_WEAK void *rt_heap_begin_get(void)
{
    return rt_heap;
}

RT_WEAK void *rt_heap_end_get(void)
{
    return rt_heap + RT_HEAP_SIZE;
}
#endif

/**
 * This function will initial your board.
 */
void rt_hw_board_init()
{
  SystemInit();      //Initializes system clock
  _SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);
	gpio_init();       //Initializes GPIO
	uart_init(UART0,   //Initializes UART0
	          38400,   //Baudrate
	          1,       //Tx
	          1,       //Rx
	          0,       //Tx interrupt
	          0,       //Rx interrupt
	          0,       //Tx overflow interrupt
	          0);      //Rx overflow interrupt

  /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif
}

//SysTick_Handler interrupt handler function
void SysTick_Handler(void)
{
  /* enter interrupt */
  rt_interrupt_enter();

  rt_tick_increase();

  /* leave interrupt */
  rt_interrupt_leave();
}

//RT console output
void rt_hw_console_output(const char *str)
{
  rt_size_t i = 0, size = 0;
  char a = '\r';

  size = rt_strlen(str);
  for (i = 0; i < size; i++)
  {
    if (*(str + i) == '\n')
    {
			UART_SendChar(UART0,(uint8_t)a);
    }
		UART_SendChar(UART0, (uint8_t)(*(str + i)));
  }
}

//RT console input
char rt_hw_console_getchar(void)
{
	return UART_ReceiveChar(UART0);
}
