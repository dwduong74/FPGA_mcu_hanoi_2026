/******************** (C) COPYRIGHT 2023 SONiX *******************************
* COMPANY:      SONiX
* DATE:         2023/11
* DESCRIPTION:  ADC driver - single channel, polling mode (for potentiometer)
*               AIN0 = P2.0 = biến trở VR1 trên EVK
*****************************************************************************/
#ifndef __ADC_H
#define __ADC_H

#include <SN32F400.h>

/*_____ M A C R O S ________________________________________________________*/
/* Enable / Disable HCLK cho ADC */
#define __ADC_ENABLE_HCLK    (SN_SYS1->AHBCLKEN_b.ADCCLKEN = 1)
#define __ADC_DISABLE_HCLK   (SN_SYS1->AHBCLKEN_b.ADCCLKEN = 0)

/* Độ phân giải ADC */
#define ADC_ADLEN_8BIT   0
#define ADC_ADLEN_12BIT  1

/* Độ chia clock ADC (PCLK/N) */
#define ADC_ADCKS_DIV1   0
#define ADC_ADCKS_DIV2   1
#define ADC_ADCKS_DIV4   2
#define ADC_ADCKS_DIV8   3
#define ADC_ADCKS_DIV16  4
#define ADC_ADCKS_DIV32  5

/* Full scale của ADC 12-bit */
#define ADC_FULL_SCALE   4096U   /* 2^12 */

/*_____ F U N C T I O N S __________________________________________________*/
void     ADC_Init(void);
uint16_t ADC_ReadBlocking(void);   /* Blocking: ~2µs tại 12MHz */

#endif /* __ADC_H */
