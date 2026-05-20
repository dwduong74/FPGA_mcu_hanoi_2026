/*
 ******************************************************************************************
 * @file      apb2_master_demo.c
 * @author    GowinSemiconductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Demo APB2 master.
 ******************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "demo.h"


/* Definitions ---------------------------------------------------------------*/

#if APB2_MASTER_DEMO

/* Includes: */
#include "uart.h"
#include <stdio.h>


/* Definitions: */

//Type definition
typedef struct
{
  __IO   uint32_t  MULTIPLIER;        /* Offset: 0x000 (R/W) [7:0]  */
  __IO   uint32_t  MULTIPLICAND;      /* Offset: 0x004 (R/W) [7:0]  */
  __IO   uint32_t  CMD;               /* Offset: 0x008 (R/W) [1:0]  */
  __I    uint32_t  RESULT;            /* Offset: 0x00C (R/ ) [15:0] */
}MULTIPLE_TypeDef;

//Base address
#define MULTIPLE_BASE   APB2MASTER1_BASE

//Mapping
#define MULTIPLE        ((MULTIPLE_TypeDef   *) MULTIPLE_BASE)

//Bit definition
#define MUL_MULTIPLIER    ((uint32_t) 0x000000FF)
#define MUL_MULTIPLICAND  ((uint32_t) 0x000000FF)
#define CMD_START         ((uint32_t) 0x00000001)
#define STATUS_FINISHED   ((uint32_t) 0x00000010)
#define MUL_RESULT        ((uint32_t) 0x0000FFFF)

typedef enum
{
  FINISHED_STATUS = 0x0,
  NO_FINISHED_STATUS = 0x1
}STATUS;


void setMultiplier(uint32_t multi)
{
  MULTIPLE->MULTIPLIER = multi & MUL_MULTIPLIER;
}

uint32_t getMultiplier(void)
{
  return MULTIPLE->MULTIPLIER & MUL_MULTIPLIER;
}

void setMultiplicand(uint32_t multi)
{
  MULTIPLE->MULTIPLICAND = multi & MUL_MULTIPLICAND;
}

uint32_t getMultiplicand(void)
{
  return MULTIPLE->MULTIPLICAND & MUL_MULTIPLICAND;
}

uint32_t getMultipleResult(void)
{
  return MULTIPLE->RESULT & MUL_RESULT;
}

void startMultiple(void)
{
  MULTIPLE->CMD |= CMD_START;
}

void finishMultiple(void)
{
  MULTIPLE->CMD = 0;
}

uint32_t getMultipleCmd(void)
{
  return MULTIPLE->CMD;
}

STATUS getFinishStatus(void)
{
  if(((MULTIPLE->CMD&STATUS_FINISHED)>>1))
  {
    return FINISHED_STATUS;
  }
  else
  {
    return NO_FINISHED_STATUS;
  }
}

//Application entry function
int apb2_master_demo(void)
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

  printf("---------------------------------------------------\r\n");
  printf("Extended APB2 master peripherals in FPGA Fabric.\r\n");
  printf("---------------------------------------------------\r\n");
  printf("\r\n");

  printf("Initialization Status : \r\n");
  printf("--MULTIPLIER = %d\r\n",getMultiplier());
  printf("--MULTIPLICAND = %d\r\n",getMultiplicand());
  printf("--CMD = %d\r\n",getMultipleCmd());
  printf("--RESULT = %d\r\n",getMultipleResult());
  printf("\r\n");

  printf("Start first multiple\r\n");
  setMultiplier(20);
  setMultiplicand(40);
  startMultiple();
  printf("Compute Status : \r\n");
  printf("--Multiplier = %d\r\n",getMultiplier());
  printf("--Multiplicand = %d\r\n",getMultiplicand());
  printf("--CMD = %d\r\n",getMultipleCmd());

  while(getFinishStatus()==FINISHED_STATUS);
  finishMultiple();

  printf("Finished Status : \r\n");
  printf("--Multiplier = %d\r\n",getMultiplier());
  printf("--Multiplicand = %d\r\n",getMultiplicand());
  printf("--CMD = %d\r\n",getMultipleCmd());
  printf("--RESULT = %d\r\n",getMultipleResult());
  printf("Multiple first finished.\r\n");
  printf("\r\n");

  printf("Start second multiple\r\n");
  setMultiplier(30);
  setMultiplicand(50);
  startMultiple();
  printf("Compute Status : \r\n");
  printf("--Multiplier = %d\r\n",getMultiplier());
  printf("--Multiplicand = %d\r\n",getMultiplicand());
  printf("--CMD = %d\r\n",getMultipleCmd());

  while(getFinishStatus()==FINISHED_STATUS);
  finishMultiple();

  printf("Finished Status : \r\n");
  printf("--Multiplier = %d\r\n",getMultiplier());
  printf("--Multiplicand = %d\r\n",getMultiplicand());
  printf("--CMD = %d\r\n",getMultipleCmd());
  printf("--RESULT = %d\r\n",getMultipleResult());
  printf("Multiple second finished.\r\n");

  printf("\r\nEND!\r\n");

  while(1);
}

#endif
