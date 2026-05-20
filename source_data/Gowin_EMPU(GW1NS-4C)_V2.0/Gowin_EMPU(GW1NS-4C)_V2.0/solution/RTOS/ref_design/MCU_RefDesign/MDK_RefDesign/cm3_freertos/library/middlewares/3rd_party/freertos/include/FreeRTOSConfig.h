
#ifndef	__FREERTOS_CONFIG_H_
#define	__FREERTOS_CONFIG_H_

#include "gw1ns4c.h"


#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

#define INCLUDE_xTaskGetCurrentTaskHandle 1
	
#define vPortSVCHandler				SVC_Handler
#define xPortPendSVHandler		PendSV_Handler
//#define xPortSysTickHandler		SysTick_Handler

#define configUSE_PREEMPTION		0
#define configUSE_IDLE_HOOK			0
#define configUSE_TICK_HOOK			0
#define configCPU_CLOCK_HZ			( ( unsigned long ) (25000000) )
#define configTICK_RATE_HZ			( ( TickType_t ) 100 )
#define configMAX_PRIORITIES		( 3 )
#define configMINIMAL_STACK_SIZE	( ( unsigned short ) 16 )
#define configTOTAL_HEAP_SIZE		( ( size_t ) ( 1 * 1024 ) )
#define configMAX_TASK_NAME_LEN		( 16 )
#define configUSE_TRACE_FACILITY	0
#define configUSE_16_BIT_TICKS		0
#define configIDLE_SHOULD_YIELD		0
#define configUSE_MUTEXES			0

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 		0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_xTaskGetSchedulerState  1
#define INCLUDE_vTaskPrioritySet		0
#define INCLUDE_uxTaskPriorityGet		0
#define INCLUDE_vTaskDelete				0
#define INCLUDE_vTaskCleanUpResources	0
#define INCLUDE_vTaskSuspend			0
#define INCLUDE_vTaskDelayUntil			0
#define INCLUDE_vTaskDelay				1


#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS       					__NVIC_PRIO_BITS
#else
#define configPRIO_BITS       					3
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY			7
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	2
#define configKERNEL_INTERRUPT_PRIORITY 		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )


#ifdef __cplusplus
}  //extern "C"
#endif //__cplusplus

#endif                                                                  /* __FREERTOS_CONFIG_H_ */
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
