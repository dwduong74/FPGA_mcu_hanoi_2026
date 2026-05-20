/*==========================================================================
 * DIGITAL CLOCK - SN32F407_EVK — PHIÊN BẢN MÔ PHỎNG KEIL SIMULATOR
 *==========================================================================
 * File    : main_sim.c
 * Dùng    : Keil MDK-ARM Simulator (KHÔNG cần phần cứng)
 * Khác với main.c ở điểm:
 *   1. Dùng SysTick thay CT16B1 → Keil Simulator mô phỏng được
 *   2. Bỏ Digital_Scan(), buzzer_on/off() (không có GPIO thật)
 *   3. Bỏ KeyScan() — phím được đặt trực tiếp qua Watch Window
 *   4. Giữ nguyên 100% logic: state machine, timing, EEPROM
 *
 *--------------------------------------------------------------------------
 * CÁCH MÔ PHỎNG TRONG KEIL
 *--------------------------------------------------------------------------
 *  Bước 1: Build project với file main_sim.c (thay main.c)
 *  Bước 2: Debug → Options for Target → Debug → [x] Use Simulator
 *  Bước 3: Ctrl+F5 → Start Debug Session
 *  Bước 4: Thêm vào Watch Window 1:
 *            g_hour, g_min, g_sec, g_state,
 *            g_alarm_hour, g_alarm_min, g_alarm_active
 *  Bước 5: Nhấn F5 (Run) → xem g_sec tăng mỗi giây (thời gian sim)
 *
 *  Giả lập nhấn phím:
 *    Trong Watch Window, gõ vào ô g_sim_key một trong các giá trị:
 *      KEY_SETUP = 0x14  (SW3  - SETUP)
 *      KEY_PLUS  = 0x22  (SW6  - PLUS)
 *      KEY_MINUS = 0x42  (SW10 - MINUS)
 *      KEY_ALARM = 0x88  (SW16 - ALARM)
 *    Code sẽ xử lý phím và reset g_sim_key về 0 tự động.
 *
 *  Theo dõi hiển thị:
 *    Xem segment_buff[0..3] trong Watch Window:
 *      [0] = tens of hours  (giá trị nhị phân của đoạn LED)
 *      [1] = units of hours + DP (bit 7 = dấu chấm)
 *      [2] = tens of minutes
 *      [3] = units of minutes
 *    Hoặc theo dõi trực tiếp g_hour, g_min (dễ đọc hơn)
 *==========================================================================*/

/*--------------------------------------------------------------------------
 *  INCLUDE
 *--------------------------------------------------------------------------*/
#include <SN32F400.h>
#include <SN32F400_Def.h>
#include "..\Driver\WDT.h"
#include "..\Driver\SysTick_1ms.h"   /* Thay CT16B1 — Keil sim hỗ trợ    */
#include "..\Module\Segment.h"        /* Chỉ dùng SEGMENT_TABLE[], segment_buff[] */
#include "..\Module\KeyScan.h"        /* Chỉ dùng hằng KEY_x               */
#include "..\Module\EEPROM.h"

/*--------------------------------------------------------------------------
 *  Kiểm tra package (bắt buộc)
 *--------------------------------------------------------------------------*/
#ifndef SN32F407
    #error "Vui long cai SONiX.SN32F4_DFP phien ban >= 0.0.18"
#endif
#define PKG  SN32F407

/*==========================================================================
 *  ĐỊNH NGHĨA TRẠNG THÁI
 *==========================================================================*/
#define STATE_NORMAL          0
#define STATE_SET_HOUR        1
#define STATE_SET_MIN         2
#define STATE_SET_ALARM_HOUR  3
#define STATE_SET_ALARM_MIN   4

/*==========================================================================
 *  ĐỊNH NGHĨA PHÍM
 *  Giá trị bitmask thực tế từ KeyScan.h (ROW | COL bits)
 *  Dùng trong Watch Window để giả lập nhấn phím
 *==========================================================================*/
#define KEY_SETUP    KEY_3    /* 0x14 - SW3  */
#define KEY_PLUS     KEY_6    /* 0x22 - SW6  */
#define KEY_MINUS    KEY_10   /* 0x42 - SW10 */
#define KEY_ALARM    KEY_16   /* 0x88 - SW16 */

/*==========================================================================
 *  EEPROM
 *==========================================================================*/
#define EEPROM_WRITE_ADDR   0xA0
#define EEPROM_READ_ADDR    0xA1
#define EEPROM_AL_HOUR      0x00
#define EEPROM_AL_MIN       0x01

/*==========================================================================
 *  HẰNG SỐ THỜI GIAN
 *==========================================================================*/
#define BLINK_HALF_MS    500U
#define TIMEOUT_MS      30000U
#define PIP_MS           300U
#define ALARM_HALF_MS    500U
#define ALARM_TOTAL_MS  5000U

/*==========================================================================
 *  SEGMENT DP
 *==========================================================================*/
#define SEG_DP   0x80u

/*==========================================================================
 *  BIẾN GIẢ LẬP PHÍM (SIM KEY)
 *  Đặt giá trị từ Watch Window để giả lập nhấn phím.
 *  Main loop sẽ xử lý và reset về 0.
 *
 *  Ví dụ trong Watch Window:
 *    g_sim_key = 0x14   → giả lập nhấn SW3 (SETUP)
 *    g_sim_key = 0x22   → giả lập nhấn SW6 (PLUS+)
 *    g_sim_key = 0x42   → giả lập nhấn SW10 (MINUS-)
 *    g_sim_key = 0x88   → giả lập nhấn SW16 (ALARM)
 *==========================================================================*/
volatile uint16_t g_sim_key = 0;   /* Đặt từ Watch Window để giả lập phím */

/*==========================================================================
 *  BIẾN TOÀN CỤC (giống main.c)
 *==========================================================================*/
static uint8_t  g_state = STATE_NORMAL;

static uint8_t  g_hour  = 0;
static uint8_t  g_min   = 0;
static uint8_t  g_sec   = 0;

static uint8_t  g_alarm_hour = 0;
static uint8_t  g_alarm_min  = 0;

static uint16_t g_ms_cnt      = 0;
static uint16_t g_blink_cnt   = 0;
static uint16_t g_timeout_cnt = 0;
static uint8_t  g_blink_on    = 1;

static uint16_t g_pip_ms          = 0;
static uint8_t  g_alarm_active    = 0;
static uint16_t g_alarm_remain_ms = 0;
static uint16_t g_alarm_phase_ms  = 0;
static uint8_t  g_alarm_phase_on  = 0;
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);

static void process_key(uint16_t key);
static void update_display(void);
static void tick_second(void);
static void start_pip(void);
static void start_alarm_beep(void);
static void update_buzzer(void);
static void load_alarm_from_eeprom(void);
static void save_alarm_to_eeprom(void);

int main(void)
{
    /*----------------------------------------------------------------------
     *  Khởi động hệ thống
     *--------------------------------------------------------------------*/
    SystemInit();
    SystemCoreClockUpdate();

    PFPA_Init();
    NotPinOut_GPIO_init();
    SN_SYS0->EXRSTCTRL_b.RESETDIS = 0;

    WDT_Init();

    /*----------------------------------------------------------------------
     *  Khởi động SysTick 1ms (thay CT16B1)
     *  SysTick_Handler() sẽ set timer_1ms_flag = 1 mỗi 1ms
     *  Keil Simulator mô phỏng SysTick dựa theo số chu kỳ CPU
     *--------------------------------------------------------------------*/
    SysTick_1ms_Init();

    /*  GHI CHÚ SIMULATION:
     *  - I2C0_Init() và EEPROM bị bỏ qua (Keil sim không có I2C thật)
     *  - Giá trị báo thức mặc định = 00:00 (hoặc đặt trực tiếp bên dưới)
     *  - Để test: đặt g_alarm_hour = 0, g_alarm_min = 1 trong Watch Window
     *    rồi chạy 1 phút để xem alarm kích hoạt                           */
    g_alarm_hour = 0;
    g_alarm_min  = 0;

    g_blink_on = 1;
    update_display();

    /*======================================================================
     *  VÒNG LẶP CHÍNH
     *====================================================================*/
    while (1)
    {

			__WDT_FEED_VALUE;

        /*==================================================================
         *  Khối xử lý 1ms (SysTick thay CT16B1)
         *================================================================*/
        if (timer_1ms_flag)
        {
            timer_1ms_flag = 0;

            /*  NOTE: Digital_Scan() bị bỏ qua trong bản sim
             *  Thay vào đó, theo dõi g_hour, g_min, g_state trong Watch Window */

            /* Bộ đếm 1 giây */
            g_ms_cnt++;
            if (g_ms_cnt >= 1000U)
            {
                g_ms_cnt = 0;
                tick_second();
            }

            /* Nhấp nháy và timeout ở chế độ SET */
            if (g_state != STATE_NORMAL)
            {
                g_blink_cnt++;
                if (g_blink_cnt >= BLINK_HALF_MS)
                {
                    g_blink_cnt = 0;
                    g_blink_on  = !g_blink_on;
                    update_display();
                }

                g_timeout_cnt++;
                if (g_timeout_cnt >= TIMEOUT_MS)
                {
                    g_timeout_cnt = 0;
                    g_blink_on    = 1;
                    g_state       = STATE_NORMAL;
                    start_pip();
                    update_display();
                }
            }
            else
            {
                g_blink_on = 1;
            }

            /* Cập nhật buzzer (logic giữ nguyên, chỉ không có âm thanh) */
            update_buzzer();

        } /* end if (timer_1ms_flag) */

        /*==================================================================
         *  Đọc phím giả lập từ g_sim_key (đặt từ Watch Window)
         *  Người dùng đặt g_sim_key = KEY_SETUP/PLUS/MINUS/ALARM
         *  Code xử lý xong thì reset về 0

         *================================================================*/
        if (g_sim_key != 0)
        {
            uint16_t k = g_sim_key;
            g_sim_key  = 0;           /* Reset ngay để không xử lý 2 lần   */
            process_key(k);
        }

    } /* end while(1) */
}

/*==========================================================================
 *  tick_second() — giống main.c hoàn toàn
 *==========================================================================*/
static void tick_second(void)
{
    if (g_state != STATE_NORMAL)
        return;

    g_sec++;
    if (g_sec >= 60)
    {
        g_sec = 0;
        g_min++;
        update_display();

        if (g_min >= 60)
        {
            g_min = 0;
            g_hour++;
            if (g_hour >= 24) g_hour = 0;
        }

        if (g_hour == g_alarm_hour && g_min == g_alarm_min)
        {
            start_alarm_beep();
        }
    }
}

/*==========================================================================
 *  process_key() — giống main.c hoàn toàn
 *==========================================================================*/
static void process_key(uint16_t key)
{
    start_pip();
    g_timeout_cnt = 0;
    g_blink_cnt   = 0;
    g_blink_on    = 1;

    switch (g_state)
    {
        case STATE_NORMAL:
            if (key == KEY_SETUP)       g_state = STATE_SET_HOUR;
            else if (key == KEY_ALARM)  g_state = STATE_SET_ALARM_HOUR;
            break;

        case STATE_SET_HOUR:
            if (key == KEY_SETUP)
                g_state = STATE_SET_MIN;
            else if (key == KEY_PLUS)
                g_hour = (g_hour + 1) % 24;
            else if (key == KEY_MINUS)
                g_hour = (g_hour == 0) ? 23 : (g_hour - 1);
            break;

        case STATE_SET_MIN:
            if (key == KEY_SETUP)
            {
                g_sec = 0; g_ms_cnt = 0;
                g_state = STATE_NORMAL;
            }
            else if (key == KEY_PLUS)
            {
                g_min = (g_min + 1) % 60;
                g_sec = 0; g_ms_cnt = 0;
            }
            else if (key == KEY_MINUS)
            {
                g_min = (g_min == 0) ? 59 : (g_min - 1);
                g_sec = 0; g_ms_cnt = 0;
            }
            break;

        case STATE_SET_ALARM_HOUR:
            if (key == KEY_ALARM)
                g_state = STATE_SET_ALARM_MIN;
            else if (key == KEY_PLUS)
                g_alarm_hour = (g_alarm_hour + 1) % 24;
            else if (key == KEY_MINUS)
                g_alarm_hour = (g_alarm_hour == 0) ? 23 : (g_alarm_hour - 1);
            break;

        case STATE_SET_ALARM_MIN:
            if (key == KEY_ALARM)
            {
                /* NOTE sim: không gọi save_alarm_to_eeprom()
                 * vì không có I2C thật. Giá trị vẫn được lưu trong RAM. */
                g_state = STATE_NORMAL;
            }
            else if (key == KEY_PLUS)
                g_alarm_min = (g_alarm_min + 1) % 60;
            else if (key == KEY_MINUS)
                g_alarm_min = (g_alarm_min == 0) ? 59 : (g_alarm_min - 1);
            break;

        default: break;
    }

    update_display();
}

/*==========================================================================
 *  update_display()
 *  Ghi vào segment_buff[] để quan sát trong Watch Window
 *  Không gọi Digital_Scan() — không có GPIO thật trong simulator
 *==========================================================================*/
static void update_display(void)
{
    uint8_t h, m;
    uint8_t show_hh = 1;
    uint8_t show_mm = 1;

    if (g_state == STATE_SET_ALARM_HOUR || g_state == STATE_SET_ALARM_MIN)
    {
        h = g_alarm_hour;
        m = g_alarm_min;
    }
    else
    {
        h = g_hour;
        m = g_min;
    }

    if (!g_blink_on)
    {
        if (g_state == STATE_SET_HOUR || g_state == STATE_SET_ALARM_HOUR)
            show_hh = 0;
        else if (g_state == STATE_SET_MIN || g_state == STATE_SET_ALARM_MIN)
            show_mm = 0;
    }

    /*  Cập nhật segment_buff[] — có thể quan sát trong Watch Window
     *  Giá trị là bitmask của các đoạn LED (SEG_A..SEG_G, SEG_DP)     */
    segment_buff[0] = show_hh ? SEGMENT_TABLE[h / 10]             : 0x00u;
    segment_buff[1] = show_hh ? (SEGMENT_TABLE[h % 10] | SEG_DP)  : 0x00u;
    segment_buff[2] = show_mm ? SEGMENT_TABLE[m / 10]             : 0x00u;
    segment_buff[3] = show_mm ? SEGMENT_TABLE[m % 10]             : 0x00u;
}

/*==========================================================================
 *  Buzzer stubs — logic giữ nguyên, không phát âm thanh trong simulator
 *==========================================================================*/
static void start_pip(void)
{
    if (!g_alarm_active)
    {
        g_pip_ms = PIP_MS;
        /* buzzer_on() bị bỏ — không có CT16B0 PWM trong sim */
    }
}

static void start_alarm_beep(void)
{
    g_pip_ms          = 0;
    g_alarm_active    = 1;
    g_alarm_remain_ms = ALARM_TOTAL_MS;
    g_alarm_phase_ms  = ALARM_HALF_MS;
    g_alarm_phase_on  = 1;
    /* buzzer_on() bị bỏ */
}

static void update_buzzer(void)
{
    if (g_alarm_active)
    {
        if (g_alarm_remain_ms > 0)
        {
            g_alarm_remain_ms--;
            g_alarm_phase_ms--;
            if (g_alarm_phase_ms == 0)
            {
                g_alarm_phase_ms = ALARM_HALF_MS;
                g_alarm_phase_on = !g_alarm_phase_on;
                /* buzzer_on()/off() bị bỏ */
            }
        }
        else
        {
            g_alarm_active = 0;
            /* buzzer_off() bị bỏ */
        }
    }
    else if (g_pip_ms > 0)
    {
        g_pip_ms--;
    }
}

/*==========================================================================
 *  EEPROM stubs — không có I2C thật trong simulator
 *==========================================================================*/
static void load_alarm_from_eeprom(void)
{
    /* Trong simulator: không đọc EEPROM, dùng giá trị mặc định 00:00 */
    g_alarm_hour = 0;
    g_alarm_min  = 0;
}

static void save_alarm_to_eeprom(void)
{
    /* Trong simulator: không ghi EEPROM, giá trị chỉ tồn tại trong RAM */
    (void)0;
}

/*==========================================================================
 *  Các hàm hệ thống (giống main.c)
 *==========================================================================*/
void NotPinOut_GPIO_init(void)
{
    (void)0;
}

void HardFault_Handler(void)
{
    NVIC_SystemReset();
}
