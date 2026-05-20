/*==========================================================================
 * DIGITAL CLOCK - SN32F407_EVK
 *==========================================================================
 * File    : main.c
 * Board   : SN32F407_EVK (Evaluation Kit)
 * MCU     : SN32F407 (ARM Cortex-M0, HCLK = 12 MHz nội bộ IHRC)
 *
 *--------------------------------------------------------------------------
 * SƠ ĐỒ CHÂN PHẦN CỨNG
 *--------------------------------------------------------------------------
 *  Chức năng              Chân            Ghi chú
 *  --------------------- --------------- --------------------------------
 *  7-SEG Segment A~G,DP   P0.0~P0.7       Active-HIGH
 *  7-SEG COM0 (H_tens)    P1.9            Active-HIGH (qua transistor)
 *  7-SEG COM1 (H_units)   P1.10           + dấu chấm DP (bit7)
 *  7-SEG COM2 (M_tens)    P1.11
 *  7-SEG COM3 (M_units)   P1.12
 *  Key ROW1~4             P1.4~P1.7
 *  Key COL1~4             P2.4~P2.7
 *  LED D0 (báo alarm)     P3.8            Active-LOW
 *  LED D1 (nhịp giây)     P3.9            Active-LOW
 *  Buzzer PWM             P3.0            CT16B0 PWM0
 *  I2C0 SCL/SDA           P0.10/P0.11     PFPA option 2
 *
 *--------------------------------------------------------------------------
 * BÀN PHÍM 4×4
 *--------------------------------------------------------------------------
 *         COL1    COL2    COL3    COL4
 *  ROW1   SW1=7   SW2=8   SW3=9   SW4=SetGiờ
 *  ROW2   SW5=4   SW6=5   SW7=6   SW8=SetBáoThức
 *  ROW3   SW9=1  SW10=2  SW11=3  SW12=TắtBáoThức
 *  ROW4  SW13=-  SW14=0  SW15=+  SW16=TắtMànHình
 *
 *--------------------------------------------------------------------------
 * CÀI ĐẶT GIỜ (cursor mode)
 *--------------------------------------------------------------------------
 *  [SW4] → vào chế độ chỉnh giờ, con trỏ ở chữ số 0 (H_tens)
 *  Nhấn số 0~9 → cài trực tiếp chữ số tại con trỏ, clamp nếu vượt max
 *  Nhấn [+]/[-]  → tăng/giảm giá trị tại vị trí con trỏ (có wrap)
 *  Nhấn [SW4] tiếp → con trỏ tiến sang ô tiếp theo
 *  Sau ô thứ 4 (M_units) → lưu và về NORMAL
 *  Không nhấn phím 30s → tự về NORMAL
 *
 *  Tương tự [SW8] cho báo thức (xác nhận cuối lưu EEPROM)
 *
 *--------------------------------------------------------------------------
 * MÀN HÌNH TẮT
 *--------------------------------------------------------------------------
 *  [SW16] → tắt tất cả LED 7-SEG + D0 + D1 (tiết kiệm điện)
 *  Bất kỳ phím nào → bật lại về NORMAL
 *==========================================================================*/

/*--------------------------------------------------------------------------
 *  INCLUDE
 *--------------------------------------------------------------------------*/
#include <SN32F400.h>
#include <SN32F400_Def.h>
#include "..\Driver\GPIO.h"
#include "..\Driver\CT16B0.h"
#include "..\Driver\CT16B1.h"
#include "..\Driver\WDT.h"
#include "..\Driver\I2C.h"
#include "..\Driver\Utility.h"
#include "..\Module\Segment.h"
#include "..\Module\KeyScan.h"
#include "..\Module\EEPROM.h"

/*--------------------------------------------------------------------------
 *  KIỂM TRA PACKAGE MCU
 *--------------------------------------------------------------------------*/
#ifndef SN32F407
    #error "Vui long cai SONiX.SN32F4_DFP phien ban >= 0.0.18"
#endif
#define PKG  SN32F407

/*==========================================================================
 *  TRẠNG THÁI (STATES)
 *==========================================================================*/
#define STATE_NORMAL        0   /* Đồng hồ chạy bình thường                */
#define STATE_SET_HOUR      1   /* Chỉnh giờ hiện tại — cursor mode         */
#define STATE_SET_ALARM     2   /* Chỉnh báo thức — cursor mode             */
#define STATE_DISPLAY_OFF   3   /* Màn hình tắt — tiết kiệm điện            */

/*==========================================================================
 *  ĐỊNH NGHĨA PHÍM
 *==========================================================================*/
/* Số 0~9 */
#define KEY_NUM_0   KEY_14   /* ROW4/COL2 */
#define KEY_NUM_1   KEY_9    /* ROW3/COL1 */
#define KEY_NUM_2   KEY_10   /* ROW3/COL2 */
#define KEY_NUM_3   KEY_11   /* ROW3/COL3 */
#define KEY_NUM_4   KEY_5    /* ROW2/COL1 */
#define KEY_NUM_5   KEY_6    /* ROW2/COL2 */
#define KEY_NUM_6   KEY_7    /* ROW2/COL3 */
#define KEY_NUM_7   KEY_1    /* ROW1/COL1 */
#define KEY_NUM_8   KEY_2    /* ROW1/COL2 */
#define KEY_NUM_9   KEY_3    /* ROW1/COL3 */
/* Chức năng */
#define KEY_MINUS       KEY_13   /* ROW4/COL1 — giảm giá trị tại cursor    */
#define KEY_PLUS        KEY_15   /* ROW4/COL3 — tăng giá trị tại cursor    */
#define KEY_SET_HOUR    KEY_4    /* ROW1/COL4 — vào/chuyển bước chỉnh giờ  */
#define KEY_SET_ALARM   KEY_8    /* ROW2/COL4 — vào/xác nhận chỉnh báo thức*/
#define KEY_ALARM_OFF   KEY_12   /* ROW3/COL4 — tắt báo thức               */
#define KEY_DISP_OFF    KEY_16   /* ROW4/COL4 — tắt màn hình               */

/*==========================================================================
 *  EEPROM
 *==========================================================================*/
#define EEPROM_WRITE_ADDR   0xA0
#define EEPROM_READ_ADDR    0xA1
#define EEPROM_AL_HOUR      0x00
#define EEPROM_AL_MIN       0x01
#define EEPROM_AL_ENABLED   0x02   /* 0x01=bật, 0x00=tắt                   */

/*==========================================================================
 *  HẰNG SỐ THỜI GIAN
 *==========================================================================*/
#define BLINK_HALF_MS    500U
#define TIMEOUT_MS      30000U
#define PIP_MS           200U    /* Độ dài 1 tiếng bíp phản hồi (ms)        */
#define PIP_GAP_MS       200U    /* Khoảng lặng giữa các bíp                 */
#define ALARM_HALF_MS    500U
#define ALARM_TOTAL_MS  60000U   /* Báo thức kêu tối đa 60 giây             */

/*==========================================================================
 *  BUZZER
 *==========================================================================*/
#define HCLK_HZ         12000000UL
#define BUZZER_FREQ_HZ  600UL
#define BUZZER_PERIOD   (HCLK_HZ / BUZZER_FREQ_HZ)

/*==========================================================================
 *  MACRO LED & SEG
 *==========================================================================*/
#define LED_ALARM_ON()   (SN_GPIO3->BCLR = (1u << 8))
#define LED_ALARM_OFF()  (SN_GPIO3->BSET = (1u << 8))
#define SEG_DP           0x80u

/*==========================================================================
 *  BIẾN TOÀN CỤC
 *==========================================================================*/
static uint8_t  g_state  = STATE_NORMAL;
static uint8_t  g_cursor = 0;          /* 0=H_tens 1=H_units 2=M_tens 3=M_units */

static uint8_t  g_hour   = 0;
static uint8_t  g_min    = 0;
static uint8_t  g_sec    = 0;

static uint8_t  g_alarm_hour    = 0;
static uint8_t  g_alarm_min     = 0;
static uint8_t  g_alarm_enabled = 1;   /* 1=báo thức bật, 0=tắt            */

static uint16_t g_ms_cnt      = 0;
static uint16_t g_blink_cnt   = 0;
static uint16_t g_timeout_cnt = 0;
static uint8_t  g_blink_on    = 1;

static uint16_t g_pip_ms          = 0;   /* Đếm ms tiếng bíp hiện tại       */
static uint8_t  g_pip_remain      = 0;   /* Số bíp còn lại cần phát         */
static uint16_t g_pip_gap_cnt     = 0;   /* Đếm ms khoảng lặng giữa bíp     */
static uint8_t  g_alarm_active    = 0;
static uint16_t g_alarm_remain_ms = 0;
static uint16_t g_alarm_phase_ms  = 0;
static uint8_t  g_alarm_phase_on  = 0;

/*==========================================================================
 *  KHAI BÁO HÀM
 *==========================================================================*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);

static void process_key(uint16_t key);
static void update_display(void);
static void tick_second(void);
static void buzzer_on(void);
static void buzzer_off(void);
static void start_pips(uint8_t n);   /* Phát n tiếng bíp ngắn liên tiếp    */
static void start_alarm_beep(void);
static void update_buzzer(void);
static void load_settings_from_eeprom(void);
static void save_alarm_to_eeprom(void);

/* Cursor digit helpers */
static void set_cursor_digit(uint8_t cur, uint8_t digit, uint8_t *ph, uint8_t *pm);
static void inc_cursor(uint8_t cur, uint8_t *ph, uint8_t *pm);
static void dec_cursor(uint8_t cur, uint8_t *ph, uint8_t *pm);

/*==========================================================================
 *  main()
 *==========================================================================*/
int main(void)
{
    SystemInit();
    SystemCoreClockUpdate();

    PFPA_Init();
    NotPinOut_GPIO_init();
    SN_SYS0->EXRSTCTRL_b.RESETDIS = 0;

    GPIO_Init();

    CT16B0_Init();
    SN_PFPA->CT16B0_b.PWM0 = 1;   /* PWM0 = P3.0 */
    buzzer_off();

    CT16B1_Init();

    I2C0_Init();
    load_settings_from_eeprom();   /* Đọc giờ + báo thức từ EEPROM          */

    g_blink_on = 1;
    update_display();

    WDT_Init();

    /*----------------------------------------------------------------------
     *  STARTUP: Nhấp nháy giờ báo thức 2 lần để BTC thấy giá trị đã lưu
     *    ON 400ms → OFF 400ms → ON 400ms → OFF 400ms → bắt đầu chạy
     *--------------------------------------------------------------------*/
    {
        uint8_t  n_half  = 0;    /* số half-period đã qua (cần 4 = 2 blinks) */
        uint16_t ms      = 0;
        uint8_t  show    = 1;

        /* Hiển thị giờ báo thức lên segment_buff */
        segment_buff[0] = SEGMENT_TABLE[g_alarm_hour / 10];
        segment_buff[1] = SEGMENT_TABLE[g_alarm_hour % 10] | SEG_DP;
        segment_buff[2] = SEGMENT_TABLE[g_alarm_min  / 10];
        segment_buff[3] = SEGMENT_TABLE[g_alarm_min  % 10];

        while (n_half < 4u)
        {
            __WDT_FEED_VALUE;
            if (timer_1ms_flag)
            {
                timer_1ms_flag = 0;
                Digital_Scan();
                ms++;
                if (ms >= 400u)
                {
                    ms    = 0;
                    show  = !show;
                    n_half++;
                    if (show)
                    {
                        segment_buff[0] = SEGMENT_TABLE[g_alarm_hour / 10];
                        segment_buff[1] = SEGMENT_TABLE[g_alarm_hour % 10] | SEG_DP;
                        segment_buff[2] = SEGMENT_TABLE[g_alarm_min  / 10];
                        segment_buff[3] = SEGMENT_TABLE[g_alarm_min  % 10];
                    }
                    else
                    {
                        segment_buff[0] = 0x00u;
                        segment_buff[1] = 0x00u;
                        segment_buff[2] = 0x00u;
                        segment_buff[3] = 0x00u;
                    }
                }
            }
        }

        /* Trả về hiển thị bình thường (00:00) */
        update_display();
    }

    while (1)
    {
        __WDT_FEED_VALUE;

        if (timer_1ms_flag)
        {
            timer_1ms_flag = 0;

            /* Task 1: Quét 7-SEG */
            Digital_Scan();

            /* Task 2: Quét phím */
            {
                uint16_t key = KeyScan();
                if (key != 0)
                    process_key(key);
            }

            /* Task 3: Đếm giây */
            g_ms_cnt++;
            if (g_ms_cnt >= 1000U)
            {
                g_ms_cnt = 0;
                tick_second();
            }

            /* Task 3b: LED D1 nhấp nháy 1Hz (tắt khi màn hình off) */
            if (g_state == STATE_DISPLAY_OFF)
            {
                SET_LED1_OFF;
            }
            else if (g_ms_cnt < 500U)
            {
                SET_LED1_ON;
            }
            else
            {
                SET_LED1_OFF;
            }

            /* Task 4: Nhấp nháy cursor + timeout (chỉ khi đang SET) */
            if (g_state == STATE_SET_HOUR || g_state == STATE_SET_ALARM)
            {
                /* Nhấp nháy chữ số tại cursor: toggle mỗi 500ms */
                g_blink_cnt++;
                if (g_blink_cnt >= BLINK_HALF_MS)
                {
                    g_blink_cnt = 0;
                    g_blink_on  = !g_blink_on;
                    update_display();
                }

                /* Timeout 30s: về NORMAL nếu không nhấn phím */
                g_timeout_cnt++;
                if (g_timeout_cnt >= TIMEOUT_MS)
                {
                    g_timeout_cnt = 0;
                    g_blink_on    = 1;
                    g_state       = STATE_NORMAL;
                    start_pips(1);
                    update_display();
                }
            }
            else
            {
                g_blink_on = 1;
            }

            /* Task 5: Cập nhật buzzer */
            update_buzzer();

        } /* end if (timer_1ms_flag) */

    } /* end while(1) */
}

/*==========================================================================
 *  tick_second()
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

        if (g_min >= 60)
        {
            g_min = 0;
            g_hour++;
            if (g_hour >= 24)
                g_hour = 0;
        }

        update_display();   /* Cập nhật sau khi rollover xong — tránh hiển thị XX:60 */

        if (g_alarm_enabled && g_hour == g_alarm_hour && g_min == g_alarm_min)
            start_alarm_beep();
    }
}

/*==========================================================================
 *  set_cursor_digit()
 *  Cài trực tiếp một chữ số tại vị trí cursor, sau đó clamp giá trị hợp lệ
 *==========================================================================*/
static void set_cursor_digit(uint8_t cur, uint8_t digit, uint8_t *ph, uint8_t *pm)
{
    uint8_t h = *ph, m = *pm;
    switch (cur)
    {
        case 0: h = digit * 10 + (h % 10); break;   /* H_tens */
        case 1: h = (h / 10) * 10 + digit; break;   /* H_units */
        case 2: m = digit * 10 + (m % 10); break;   /* M_tens */
        case 3: m = (m / 10) * 10 + digit; break;   /* M_units */
        default: break;
    }
    if (h > 23) h = 23;
    if (m > 59) m = 59;
    *ph = h;
    *pm = m;
}

/*==========================================================================
 *  inc_cursor() / dec_cursor()
 *  Tăng/giảm giá trị tại vị trí cursor với wrap-around
 *  cursor 0: bước ±10 giờ  | cursor 1: bước ±1 giờ
 *  cursor 2: bước ±10 phút | cursor 3: bước ±1 phút
 *==========================================================================*/
static void inc_cursor(uint8_t cur, uint8_t *ph, uint8_t *pm)
{
    switch (cur)
    {
        case 0: *ph = (*ph + 10) % 24; break;
        case 1: *ph = (*ph +  1) % 24; break;
        case 2: *pm = (*pm + 10) % 60; break;
        case 3: *pm = (*pm +  1) % 60; break;
        default: break;
    }
}

static void dec_cursor(uint8_t cur, uint8_t *ph, uint8_t *pm)
{
    switch (cur)
    {
        case 0: *ph = (*ph + 24 - 10) % 24; break;
        case 1: *ph = (*ph + 24 -  1) % 24; break;
        case 2: *pm = (*pm + 60 - 10) % 60; break;
        case 3: *pm = (*pm + 60 -  1) % 60; break;
        default: break;
    }
}

/*==========================================================================
 *  process_key()
 *==========================================================================*/
static void process_key(uint16_t key)
{
    /*--- Báo thức đang kêu: bất kỳ phím nào → tắt tiếng ---*/
    if (g_alarm_active)
    {
        g_alarm_active    = 0;
        g_alarm_remain_ms = 0;
        buzzer_off();
        return;   /* Nuốt phím, không làm gì thêm                           */
    }

    /*--- Màn hình đang tắt: bất kỳ phím nào → thức dậy ---*/
    if (g_state == STATE_DISPLAY_OFF)
    {
        g_state    = STATE_NORMAL;
        g_blink_on = 1;
        update_display();
        return;
    }

    /* Phản hồi chung: 1 bíp */
    start_pips(1);
    g_timeout_cnt = 0;
    g_blink_cnt   = 0;
    g_blink_on    = 1;

    /*--- Phím chức năng (ưu tiên cao) ---*/
    if (key == KEY_DISP_OFF)
    {
        g_state = STATE_DISPLAY_OFF;
        update_display();
        return;
    }

    if (key == KEY_ALARM_OFF)
    {
        if (g_alarm_enabled)
        {
            /* Đang BẬT → TẮT: kêu 1 bíp */
            g_alarm_enabled = 0;
            start_pips(1);
        }
        else
        {
            /* Đang TẮT → BẬT: kêu 2 bíp */
            g_alarm_enabled = 1;
            start_pips(2);
        }
        /* Lưu trạng thái mới vào EEPROM */
        uint8_t val = g_alarm_enabled ? 0x01u : 0x00u;
        eeprom_write(EEPROM_WRITE_ADDR, EEPROM_AL_ENABLED, &val, 1);
        update_display();
        return;
    }

    if (key == KEY_SET_HOUR)
    {
        if (g_state == STATE_SET_HOUR)
        {
            /* Đang chỉnh → tiến cursor sang ô tiếp theo */
            g_cursor++;
            if (g_cursor > 3)
            {
                /* Xong tất cả 4 ô → về NORMAL */
                g_sec    = 0;
                g_ms_cnt = 0;
                g_cursor = 0;
                g_state  = STATE_NORMAL;
            }
        }
        else
        {
            /* Vào chế độ chỉnh giờ */
            g_state  = STATE_SET_HOUR;
            g_cursor = 0;
        }
        update_display();
        return;
    }

    if (key == KEY_SET_ALARM)
    {
        if (g_state == STATE_SET_ALARM)
        {
            g_cursor++;
            if (g_cursor > 3)
            {
                /* Xác nhận → lưu EEPROM, bật báo thức, về NORMAL */
                g_alarm_enabled = 1;
                save_alarm_to_eeprom();
                g_cursor = 0;
                g_state  = STATE_NORMAL;
            }
        }
        else
        {
            g_state  = STATE_SET_ALARM;
            g_cursor = 0;
        }
        update_display();
        return;
    }

    /*--- Phím số và +/- chỉ có tác dụng khi đang ở chế độ SET ---*/
    if (g_state != STATE_SET_HOUR && g_state != STATE_SET_ALARM)
        return;

    uint8_t *ph = (g_state == STATE_SET_ALARM) ? &g_alarm_hour : &g_hour;
    uint8_t *pm = (g_state == STATE_SET_ALARM) ? &g_alarm_min  : &g_min;

    /* Map phím số */
    int8_t digit = -1;
    switch (key)
    {
        case KEY_NUM_0: digit = 0; break;
        case KEY_NUM_1: digit = 1; break;
        case KEY_NUM_2: digit = 2; break;
        case KEY_NUM_3: digit = 3; break;
        case KEY_NUM_4: digit = 4; break;
        case KEY_NUM_5: digit = 5; break;
        case KEY_NUM_6: digit = 6; break;
        case KEY_NUM_7: digit = 7; break;
        case KEY_NUM_8: digit = 8; break;
        case KEY_NUM_9: digit = 9; break;
        default:        digit = -1; break;
    }

    if (digit >= 0)
    {
        set_cursor_digit(g_cursor, (uint8_t)digit, ph, pm);
    }
    else if (key == KEY_PLUS)
    {
        inc_cursor(g_cursor, ph, pm);
    }
    else if (key == KEY_MINUS)
    {
        dec_cursor(g_cursor, ph, pm);
    }

    update_display();
}

/*==========================================================================
 *  update_display()
 *==========================================================================*/
static void update_display(void)
{
    /* --- Màn hình tắt --- */
    if (g_state == STATE_DISPLAY_OFF)
    {
        segment_buff[0] = 0x00u;
        segment_buff[1] = 0x00u;
        segment_buff[2] = 0x00u;
        segment_buff[3] = 0x00u;
        LED_ALARM_OFF();
        return;
    }

    /* --- Chọn nguồn dữ liệu --- */
    uint8_t h = (g_state == STATE_SET_ALARM) ? g_alarm_hour : g_hour;
    uint8_t m = (g_state == STATE_SET_ALARM) ? g_alarm_min  : g_min;

    /* --- Điền 4 chữ số bình thường --- */
    segment_buff[0] = SEGMENT_TABLE[h / 10];
    segment_buff[1] = SEGMENT_TABLE[h % 10] | SEG_DP;
    segment_buff[2] = SEGMENT_TABLE[m / 10];
    segment_buff[3] = SEGMENT_TABLE[m % 10];

    /* --- Nhấp nháy CHỈ chữ số tại cursor khi đang SET --- */
    if ((g_state == STATE_SET_HOUR || g_state == STATE_SET_ALARM) && !g_blink_on)
    {
        /* Xóa toàn bộ đoạn của ô đang chỉnh (kể cả DP nếu là ô 1) */
        segment_buff[g_cursor] = 0x00u;
    }

    /* --- LED D0: chỉ sáng/nhấp nháy khi SET_ALARM --- */
    if (g_state == STATE_SET_ALARM)
    {
        if (g_blink_on)
            LED_ALARM_ON();
        else
            LED_ALARM_OFF();
    }
    else
    {
        LED_ALARM_OFF();
    }
}

/*==========================================================================
 *  Buzzer functions
 *==========================================================================*/
static void buzzer_on(void)
{
    SN_CT16B0->MR9 = BUZZER_PERIOD - 1;
    SN_CT16B0->MR0 = (BUZZER_PERIOD - 1) >> 1;
    SN_CT16B0->TMRCTRL = 0;
    SN_CT16B0->TMRCTRL = 1;
}

static void buzzer_off(void)
{
    SN_CT16B0->MR0 = 0;
}

/*--------------------------------------------------------------------------
 *  start_pips(n): Phát n tiếng bíp ngắn liên tiếp (n=1 hoặc 2)
 *  Không làm gì nếu báo thức đang kêu.
 *--------------------------------------------------------------------------*/
static void start_pips(uint8_t n)
{
    if (g_alarm_active || n == 0) return;
    g_pip_remain  = n - 1u;   /* Tiếng 1 bắt đầu ngay, phần còn lại chờ */
    g_pip_gap_cnt = 0;
    g_pip_ms      = PIP_MS;
    buzzer_on();
}

static void start_alarm_beep(void)
{
    g_pip_ms          = 0;
    g_alarm_active    = 1;
    g_alarm_remain_ms = ALARM_TOTAL_MS;
    g_alarm_phase_ms  = ALARM_HALF_MS;
    g_alarm_phase_on  = 1;
    buzzer_on();
}

static void update_buzzer(void)
{
    /* --- Báo thức đang kêu --- */
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
                if (g_alarm_phase_on) buzzer_on();
                else                  buzzer_off();
            }
        }
        else
        {
            /* Hết thời gian tối đa → tự tắt */
            g_alarm_active = 0;
            buzzer_off();
        }
        return;
    }

    /* --- Chuỗi bíp phản hồi --- */
    if (g_pip_ms > 0)
    {
        /* Đang kêu tiếng bíp hiện tại */
        g_pip_ms--;
        if (g_pip_ms == 0)
        {
            buzzer_off();
            if (g_pip_remain > 0)
            {
                /* Còn bíp tiếp theo → bắt đầu đếm khoảng lặng */
                g_pip_gap_cnt = PIP_GAP_MS;
            }
        }
    }
    else if (g_pip_gap_cnt > 0)
    {
        /* Đang trong khoảng lặng giữa hai bíp */
        g_pip_gap_cnt--;
        if (g_pip_gap_cnt == 0 && g_pip_remain > 0)
        {
            /* Phát tiếng bíp tiếp theo */
            g_pip_remain--;
            g_pip_ms = PIP_MS;
            buzzer_on();
        }
    }
}

/*==========================================================================
 *  EEPROM
 *==========================================================================*/
/*--------------------------------------------------------------------------
 *  load_settings_from_eeprom()
 *  Đọc toàn bộ cài đặt (giờ hiện tại + báo thức) từ EEPROM khi khởi động.
 *  EEPROM trắng (0xFF) → dùng giá trị mặc định an toàn.
 *--------------------------------------------------------------------------*/
static void load_settings_from_eeprom(void)
{
    uint8_t ena;

    /* Chỉ đọc báo thức — giờ hiện tại luôn bắt đầu từ 00:00 khi bật nguồn */
    eeprom_read(EEPROM_READ_ADDR, EEPROM_AL_HOUR,    &g_alarm_hour, 1);
    eeprom_read(EEPROM_READ_ADDR, EEPROM_AL_MIN,     &g_alarm_min,  1);
    eeprom_read(EEPROM_READ_ADDR, EEPROM_AL_ENABLED, &ena,          1);

    if (g_alarm_hour >= 24) g_alarm_hour = 0;
    if (g_alarm_min  >= 60) g_alarm_min  = 0;
    /* 0xFF = EEPROM trắng (lần đầu dùng) → mặc định bật báo thức */
    g_alarm_enabled = (ena == 0x00u) ? 0u : 1u;
}

/*--------------------------------------------------------------------------
 *  save_alarm_to_eeprom()
 *  Gọi khi người dùng xác nhận xong cài đặt báo thức (hoặc tắt báo thức).
 *--------------------------------------------------------------------------*/
static void save_alarm_to_eeprom(void)
{
    uint8_t ena = g_alarm_enabled ? 0x01u : 0x00u;
    eeprom_write(EEPROM_WRITE_ADDR, EEPROM_AL_HOUR,    &g_alarm_hour, 1);
    eeprom_write(EEPROM_WRITE_ADDR, EEPROM_AL_MIN,     &g_alarm_min,  1);
    eeprom_write(EEPROM_WRITE_ADDR, EEPROM_AL_ENABLED, &ena,          1);
}

/*==========================================================================
 *  NotPinOut_GPIO_init / HardFault_Handler
 *==========================================================================*/
void NotPinOut_GPIO_init(void)
{
    (void)0;   /* SN32F407F: mặc định đã pull-up, không cần cấu hình thêm */
}

void HardFault_Handler(void)
{
    NVIC_SystemReset();
}
