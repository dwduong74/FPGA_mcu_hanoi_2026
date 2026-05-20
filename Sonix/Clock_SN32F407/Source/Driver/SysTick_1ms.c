/*==========================================================================
 * SysTick_1ms.c — Triển khai timer 1ms dùng ARM SysTick
 *==========================================================================
 * SysTick là bộ đếm 24-bit giảm dần tích hợp trong lõi ARM Cortex-M.
 * Khi bộ đếm về 0, nó tự nạp lại giá trị RELOAD và kích ISR SysTick_Handler.
 *
 * Công thức tính RELOAD:
 *   RELOAD = (HCLK / Tần_số_ngắt) - 1
 *   RELOAD = (12,000,000 / 1,000) - 1 = 11,999
 *
 * Keil Simulator: mô phỏng SysTick dựa trên số chu kỳ CPU thực thi,
 * nên tốc độ thời gian thực có thể khác so với phần cứng thật,
 * nhưng logic hoàn toàn đúng.
 *==========================================================================*/

#include "SysTick_1ms.h"

/*--------------------------------------------------------------------------
 *  Định nghĩa HCLK (phải khớp với cấu hình trong system_SN32F400.c)
 *  Mặc định SN32F407_EVK dùng IHRC 12 MHz
 *--------------------------------------------------------------------------*/
#define SYSTICK_HCLK_HZ   12000000UL   /* 12 MHz */
#define SYSTICK_FREQ_HZ       1000UL   /* 1000 Hz = 1ms/lần ngắt */

/* RELOAD = HCLK/FREQ - 1 = 11999 */
#define SYSTICK_RELOAD    ((SYSTICK_HCLK_HZ / SYSTICK_FREQ_HZ) - 1UL)

/*--------------------------------------------------------------------------
 *  Biến cờ 1ms (volatile vì được ghi từ ISR, đọc từ main loop)
 *--------------------------------------------------------------------------*/
volatile uint8_t timer_1ms_flag = 0;

/*==========================================================================
 *  SysTick_Handler()
 *  ISR của SysTick — tự động gọi mỗi 1ms khi bộ đếm về 0
 *  Nhiệm vụ duy nhất: đặt cờ timer_1ms_flag để main loop xử lý
 *
 *  Tên "SysTick_Handler" là tên chuẩn CMSIS ARM — KHÔNG được đổi tên
 *==========================================================================*/
void SysTick_Handler(void)
{
    timer_1ms_flag = 1;   /* Báo cho main loop biết đã qua 1ms */
}

/*==========================================================================
 *  SysTick_1ms_Init()
 *  Cấu hình và khởi động SysTick ở chế độ ngắt 1ms
 *
 *  Các bước:
 *    1. Nạp giá trị RELOAD = 11999 vào SysTick->LOAD
 *    2. Reset bộ đếm hiện tại về 0
 *    3. Chọn nguồn clock = HCLK (processor clock, không chia)
 *    4. Bật SysTick và bật ngắt SysTick
 *==========================================================================*/
void SysTick_1ms_Init(void)
{
    SysTick->CTRL = 0;                  /* Tắt SysTick trước khi cấu hình  */
    SysTick->LOAD = SYSTICK_RELOAD;     /* Đặt giá trị nạp lại = 11999     */
    SysTick->VAL  = 0;                  /* Reset bộ đếm về 0                */

    /* CTRL register:
     *   Bit 2 (CLKSOURCE) = 1 → dùng processor clock (HCLK = 12MHz)
     *   Bit 1 (TICKINT)   = 1 → bật ngắt SysTick_Handler
     *   Bit 0 (ENABLE)    = 1 → bật SysTick
     * Giá trị = 0b111 = 0x07                                               */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk   /* HCLK, không chia đôi  */
                  | SysTick_CTRL_TICKINT_Msk      /* Bật ngắt              */
                  | SysTick_CTRL_ENABLE_Msk;      /* Bật SysTick           */
}
