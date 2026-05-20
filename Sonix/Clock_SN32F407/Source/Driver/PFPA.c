/*==========================================================================
 * PFPA.c — Pin Function Pin Assignment (Gán chức năng chân)
 *==========================================================================
 * File    : PFPA.c
 * Project : Clock_SN32F407
 * Board   : SN32F407_EVK
 *
 * Mục đích:
 *   SN32F407 hỗ trợ chọn chân vật lý cho từng ngoại vi qua thanh ghi PFPA.
 *   File này cấu hình hai ngoại vi cần gán chân:
 *     1. I2C0  : SCL0 = P0.10,  SDA0 = P0.11   (dùng để đọc/ghi EEPROM)
 *     2. CT16B0: PWM0 = P3.0                   (dùng để điều khiển buzzer)
 *
 *--------------------------------------------------------------------------
 * BẢNG LỰA CHỌN CHÂN (PIN MUX OPTIONS)
 *--------------------------------------------------------------------------
 *
 *  I2C0 — thanh ghi SN_PFPA->I2C0 (bits[3:0]):
 *    Bits[3:2] = SCL0 :  00=P0.6   01=P1.4   10=P0.10  11=P1.1
 *    Bits[1:0] = SDA0 :  00=P0.7   01=P1.5   10=P0.11  11=P1.2
 *    → Chọn option 2 cho cả hai: PFPA_I2C0_VAL = 0b00001010 = 0x0A
 *       SCL0 = P0.10  (bits[3:2] = 10)
 *       SDA0 = P0.11  (bits[1:0] = 10)
 *
 *  CT16B0 — thanh ghi SN_PFPA->CT16B0 (bits[1:0] cho PWM0):
 *    Bits[1:0] = PWM0 :  00=P0.2   01=P3.0   10=P2.6   11=P0.0
 *    → Chọn option 1: PFPA_CT16B0_VAL = 0b00000001 = 0x01
 *       PWM0 = P3.0  (bits[1:0] = 01)
 *
 *  Ghi chú:
 *    - Trong main.c, sau khi gọi CT16B0_Init(), cần thêm:
 *        SN_PFPA->CT16B0_b.PWM0 = 1;
 *      để đặt chân PWM0 = P3.0 (vì CT16B0_Init() có thể ghi đè PFPA).
 *    - Các ngoại vi không dùng (UART0, UART1, SPI0, CT16B1, CT16B5, CMP)
 *      được đặt về 0 (disable / pin mux giữ default).
 *==========================================================================*/

#include <SN32F400.h>

/*--------------------------------------------------------------------------
 *  Bật/tắt từng ngoại vi (1 = có gán chân, 0 = không dùng)
 *--------------------------------------------------------------------------*/
#define UART0_EN     0   /* UART0 không dùng trong project clock            */
#define UART1_EN     0   /* UART1 không dùng                                */
#define SPI0_EN      0   /* SPI0  không dùng                                */
#define I2C0_EN      1   /* I2C0  dùng để giao tiếp EEPROM ← BẬT           */
#define CT16B0_EN    1   /* CT16B0 dùng làm PWM buzzer       ← BẬT          */
#define CT16B1_EN    0   /* CT16B1 là timer 1ms, không cần gán chân ra      */
#define CT16B5_EN    0   /* CT16B5 không dùng                               */
#define CMP_EN       0   /* Comparator không dùng                           */

/*--------------------------------------------------------------------------
 *  Giá trị gán cho từng thanh ghi PFPA
 *--------------------------------------------------------------------------*/
#define PFPA_UART0_VAL    0x00000000   /* Không dùng                       */
#define PFPA_UART1_VAL    0x00000000   /* Không dùng                       */
#define PFPA_SPI0_VAL     0x00000000   /* Không dùng                       */

/* I2C0: SCL0=P0.10 (opt2, bits[3:2]=10), SDA0=P0.11 (opt2, bits[1:0]=10)
 * 0x0A = 0b00001010 */
#define PFPA_I2C0_VAL     0x0000000A

/* CT16B0: PWM0=P3.0 (option 1, bits[1:0]=01)
 * Được ghi lại trong main.c sau CT16B0_Init() để đảm bảo đúng thứ tự */
#define PFPA_CT16B0_VAL   0x00000001

#define PFPA_CT16B1_VAL   0x00000000   /* Không cần gán chân ra            */
#define PFPA_CT16B5_VAL   0x00000000   /* Không dùng                       */
#define PFPA_CMP_VAL      0x00000000   /* Không dùng                       */

/*==========================================================================
 *  PFPA_Init()
 *  Gọi từ main() trước tất cả các Init ngoại vi khác.
 *  Gán giá trị lựa chọn chân cho từng thanh ghi PFPA tương ứng.
 *==========================================================================*/
void PFPA_Init(void)
{
    SN_PFPA->UART0  = PFPA_UART0_VAL;
    SN_PFPA->UART1  = PFPA_UART1_VAL;
    SN_PFPA->SPI0   = PFPA_SPI0_VAL;
    SN_PFPA->I2C0   = PFPA_I2C0_VAL;    /* SCL=P0.10, SDA=P0.11           */
    SN_PFPA->CT16B0 = PFPA_CT16B0_VAL;  /* PWM0=P3.0 (có thể bị ghi đè   */
                                         /*   bởi CT16B0_Init, xem main.c) */
    SN_PFPA->CT16B1 = PFPA_CT16B1_VAL;
    SN_PFPA->CT16B5 = PFPA_CT16B5_VAL;
    SN_PFPA->CMP    = PFPA_CMP_VAL;
}
