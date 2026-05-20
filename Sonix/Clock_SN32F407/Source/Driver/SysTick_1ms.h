/*==========================================================================
 * SysTick_1ms.h — Timer 1ms dùng ARM SysTick (thay thế CT16B1)
 *==========================================================================
 * Mục đích:
 *   Sử dụng SysTick (bộ đếm chuẩn ARM Cortex-M) để tạo ngắt 1ms.
 *   Keil MDK-ARM Simulator MÔ PHỎNG ĐƯỢC SysTick → dùng để chạy
 *   simulation mà không cần phần cứng thật.
 *
 * Ưu điểm so với CT16B1:
 *   - SysTick là chuẩn ARM CMSIS, Keil Simulator hỗ trợ đầy đủ
 *   - CT16B1 là ngoại vi SONiX, Keil Simulator KHÔNG mô phỏng được
 *
 * Cách dùng:
 *   1. Gọi SysTick_1ms_Init() trong main() (thay CT16B1_Init())
 *   2. Kiểm tra biến timer_1ms_flag trong main loop (giống CT16B1)
 *   3. Không cần thay đổi bất kỳ logic nào khác
 *==========================================================================*/

#ifndef __SYSTICK_1MS_H
#define __SYSTICK_1MS_H

#include <SN32F400.h>
#include <stdint.h>

/*--------------------------------------------------------------------------
 *  Biến cờ 1ms — được set = 1 bởi SysTick_Handler mỗi 1ms
 *  extern: khai báo ở SysTick_1ms.c, dùng được ở main.c
 *--------------------------------------------------------------------------*/
extern volatile uint8_t timer_1ms_flag;

/*--------------------------------------------------------------------------
 *  Khai báo hàm
 *--------------------------------------------------------------------------*/
void SysTick_1ms_Init(void);   /* Khởi tạo SysTick ở chế độ 1ms interrupt */

#endif /* __SYSTICK_1MS_H */
