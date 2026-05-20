/******************** (C) COPYRIGHT 2023 SONiX *******************************
* COMPANY:      SONiX
* DATE:         2023/11
* DESCRIPTION:  ADC driver - Single channel polling, AIN0 = P2.0 (biến trở)
*
*  Cấu hình:
*    - Kênh    : AIN0 (P2.0) — biến trở VR1
*    - Độ phân giải: 12-bit (0 ~ 4095)
*    - Vref    : nội bộ ~2V
*    - Mode    : Single channel, single conversion (polling EOC)
*    - Không dùng DMA, không dùng ngắt
*
*  Lưu ý: P2.0 không cần cấu hình GPIO — ADC tự bypass digital I/O block
*****************************************************************************/

/*_____ I N C L U D E S ____________________________________________________*/
#include <SN32F400_Def.h>
#include "ADC.h"
#include "Utility.h"

/*_____ F U N C T I O N S __________________________________________________*/

/*****************************************************************************
* Function    : ADC_Init
* Description : Khởi tạo ADC để đọc AIN0 (P2.0 - biến trở VR1)
*               12-bit, single channel, polling mode, Vref nội bộ ~2V
* Input       : None
* Output      : None
* Return      : None
*****************************************************************************/
void ADC_Init(void)
{
    /* 1. Cấp clock cho ADC */
    __ADC_ENABLE_HCLK;

    /* 2. Cấu hình ADM register */
    SN_ADC->ADM_b.AVREFHSEL = 0;   /* Vref cao = nội bộ                    */
    SN_ADC->ADM_b.VHS       = 4;   /* Mức Vref nội bộ ~2V + kết hợp AIN18 */
    SN_ADC->ADM_b.OVRMODE   = 1;   /* FIFO overwrite khi đầy               */
    SN_ADC->ADM_b.GCHS      = 1;   /* Kích hoạt global channel select       */
    SN_ADC->ADM_b.ADLEN     = ADC_ADLEN_12BIT;  /* Độ phân giải 12-bit     */
    SN_ADC->ADM_b.ADCKS     = ADC_ADCKS_DIV1;   /* ADC_CLK = PCLK = 12MHz  */

    /* 3. Chế độ đơn kênh, đơn lần chuyển đổi */
    SN_ADC->CONVCTRL_b.SCMODE = 0;  /* Single conversion mode               */
    SN_ADC->CONVCTRL_b.CH    = 0;   /* Single channel                       */

    /* 4. Bật ADC */
    SN_ADC->ADM_b.ADENB = 1;

    /* 5. Delay 100µs cho ADC ổn định sau khi bật */
    UT_DelayNx10us(10);

    /* 6. Calibration (bắt buộc để ADC chính xác) */
    SN_ADC->ADM1_b.ACS = 1;           /* Bắt đầu calibration              */
    while (SN_ADC->ADM1_b.ACS == 1);  /* Chờ calibration hoàn thành       */
    SN_ADC->ADM1_b.CALIVALENB = 1;    /* Áp dụng giá trị calibration      */

    /* 7. Chọn kênh AIN0 = P2.0 (biến trở VR1) */
    SN_ADC->CONVCTRL_b.CHS0 = 1;
}

/*****************************************************************************
* Function    : ADC_ReadBlocking
* Description : Thực hiện một lần chuyển đổi ADC và trả về kết quả 12-bit
*               Thời gian chờ ~2µs tại 12MHz — an toàn gọi trong timer ISR
* Input       : None
* Output      : None
* Return      : Giá trị ADC 12-bit (0 ~ 4095)
*****************************************************************************/
uint16_t ADC_ReadBlocking(void)
{
    uint32_t timeout = 20000U;

    SN_ADC->ADM_b.ADS = 1;                  /* Khởi động chuyển đổi         */
    while ((SN_ADC->ADM_b.EOC == 0) && (--timeout > 0U));  /* Chờ EOC flag */

    return (uint16_t)(SN_ADC->ADB & 0x0FFFU);  /* Lấy 12 bit thấp          */
}
