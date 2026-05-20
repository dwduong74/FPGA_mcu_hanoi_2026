/*
 ******************************************************************************************
 * @file      main.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Main program body.
 ******************************************************************************************
 */

/* Includes ---------------------------------------------------------------------*/
#include "uart.h"
#include "gpio.h"
#include <stdio.h>

//FreeRTOS Library
#include "FreeRTOS.h"
#include "task.h"


/* Definitions ------------------------------------------------------------------*/

#define SW_STR_NAME						"FreerRTOS_V10.2.1"	//Software name
#define SW_STR_EDITION				"V2.0"							//Software version
#define SW_STR_AUTHOR					"GOWIN"							//Owner

#define LOGO_PRINT_ON
#ifdef LOGO_PRINT_ON
#include "logo.h"
#endif	//LOGO_PRINT_ON

#define TASK_DELAY_MS_TO_TICK(ms)		((ms) / (1000 / configTICK_RATE_HZ))

//Task 1
#define LED0_TASK_PRIO			1
#define LED0_STK_SIZE 			16
TaskHandle_t LED0Task_Handler;
volatile int led0_task_flag = 0;

//Task 2
#define LED1_TASK_PRIO			2
#define LED1_STK_SIZE 			16
TaskHandle_t LED1Task_Handler;
volatile int led1_task_flag = 0;


/* Declarations: */
extern void xPortSysTickHandler(void);
static void led0_task(void *pvParameters);
static void led1_task(void *pvParameters);
static void printf_str(const char *str);
static void stars_print(uint8_t n);
static void sw_edition_print(void);
static void sys_tick_init(void);

/* Functions ------------------------------------------------------------------*/
int main(void)
{
	SystemInit();      //Initializes system clock
	uart_init(UART0,   //Initializes UART0
	          38400,   //Baudrate
	          1,       //Tx
	          1,       //Rx
	          0,       //Tx interrupt
	          0,       //Rx interrupt
	          0,       //Tx overflow interrupt
	          0);      //Rx overflow interrupt
	gpio_init();       //Initializes GPIO
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	sw_edition_print();
	
#ifdef LOGO_PRINT_ON
	printf_str(LOGO);
#endif	//LOGO_PRINT_ON

	sys_tick_init();				//Initializes Systick
	
	taskENTER_CRITICAL();
	
	//led0_task
  xTaskCreate((TaskFunction_t )led0_task,
              (const char *   )"led0_task",
              (uint16_t       )LED0_STK_SIZE,
              (void *         )NULL,
              (UBaseType_t    )LED0_TASK_PRIO,
              (TaskHandle_t * )&LED0Task_Handler);

  //led1_task
  xTaskCreate((TaskFunction_t )led1_task,
              (const char *   )"led1_task",
              (uint16_t       )LED1_STK_SIZE,
              (void *         )NULL,
              (UBaseType_t    )LED1_TASK_PRIO,
              (TaskHandle_t * )&LED1Task_Handler);
				
  taskEXIT_CRITICAL();
	
  vTaskStartScheduler();
	
	while(1);
}

//Print string
static void printf_str(const char *str)
{
	UART_SendString(UART0, (char *)str);
}

//Print *
static void stars_print(uint8_t n)
{
	while (n--)
	{
		printf_str("*");
	}
}

//Print software information
static void sw_edition_print(void)
{
	printf_str("\r\n");
	stars_print(48);
	printf_str("\r\n");
	printf_str("************************************************\r\n");
	printf_str("Name:     "SW_STR_NAME"\r\n"
						 "Edition:  "SW_STR_EDITION"\r\n"
						 "Compiled: "__DATE__", "__TIME__"\r\n"
						 "Author:   "SW_STR_AUTHOR"\r\n");
	printf_str("************************************************\r\n");
	stars_print(48);
	printf_str("\r\n\r\n");
}

//Initializes Systick
static void sys_tick_init(void)
{
	uint32_t temp;
	
	//24-bit register, max value is 16777215
	//When SystemCoreClock is 25MHz, it is 671ms
	//Set value of reload register
	temp = (1000 / configTICK_RATE_HZ) * (SystemCoreClock / 1000) - 1;
	SysTick->LOAD = temp;
	
	SysTick->VAL = temp;	//Reset current counter value

	//Select clock source, enable interrupt, enable counter
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk; 
}

//Systick interrupt handler function
void SysTick_Handler(void)
{
	if(taskSCHEDULER_NOT_STARTED != xTaskGetSchedulerState())
    {
        xPortSysTickHandler();	
    }
}

//Task 1
static void led0_task(void *pvParameters)
{
  while (1)
	{
		if (0 == led0_task_flag)
		{
			GPIO_ResetBit(GPIO0, GPIO_Pin_0);
		}
		else
		{
			GPIO_SetBit(GPIO0, GPIO_Pin_0);
		}
		
		printf_str("0.task0\r\n");
		led0_task_flag = !led0_task_flag;
		vTaskDelay(TASK_DELAY_MS_TO_TICK(250));
	}
}

//Task 2
static void led1_task(void *pvParameters)
{
  while (1)
	{
		if (0 == led1_task_flag)
		{
			GPIO_ResetBit(GPIO0, GPIO_Pin_1);
		}
		else
		{
			GPIO_SetBit(GPIO0, GPIO_Pin_1);
		}
		
		printf_str("1.task1\r\n\r\n");
		led1_task_flag = !led1_task_flag;
		vTaskDelay(TASK_DELAY_MS_TO_TICK(1000));
	}
}
