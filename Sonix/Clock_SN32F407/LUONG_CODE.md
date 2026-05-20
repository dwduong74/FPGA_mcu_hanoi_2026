# LUỒNG CODE — ĐỒNG HỒ SN32F407

---

## 1. KHỞI ĐỘNG (main)

```
SystemInit() → SystemCoreClockUpdate()
       ↓
PFPA_Init()          [Gán chân: I2C0→P0.10/11, PWM→P3.0]
NotPinOut_GPIO_init()
       ↓
GPIO_Init()          [SEG: P0 output | COM: P1.9-12 output | KEY: P1.4-7 + P2.4-7 input | LED: P3.8-9 output]
       ↓
CT16B0_Init()        [PWM buzzer 600 Hz trên CT16B0, chưa kêu]
CT16B1_Init()        [Timer ngắt 1 ms: MR9=11999, NVIC enable]
I2C0_Init()          [I2C0 enable, tốc độ chuẩn]
       ↓
load_settings_from_eeprom()
  ├─ Đọc 0x00 → g_alarm_hour   (validate < 24, else = 0)
  ├─ Đọc 0x01 → g_alarm_min    (validate < 60, else = 0)
  └─ Đọc 0x02 → g_alarm_enabled (0xFF→1, 0x00→0, khác→1)
       ↓
update_display()     [Điền segment_buff = 00.00]
WDT_Init()           [Timeout ~250 ms]
       ↓
━━━ STARTUP ANIMATION ━━━
  Hiển thị g_alarm_hour:g_alarm_min nhấp nháy 2 lần
  (ON 400ms → OFF 400ms) × 2, feed WDT mỗi vòng
       ↓
update_display()     [Trả về 00.00]
       ↓
     while(1)
```

---

## 2. VÒNG LẶP CHÍNH (while 1)

```
while(1)
{
    __WDT_FEED_VALUE                 [Feed dog ~mỗi vài µs]
    │
    ├─[timer_1ms_flag == 1]?─ NO ──→ (tiếp tục vòng lặp)
    │
    YES → timer_1ms_flag = 0
    │
    ├── Task 1: Digital_Scan()
    │     └── Quét 1 COM mỗi 1ms (COM0→1→2→3→0...)
    │
    ├── Task 2: key = KeyScan()
    │     └── key != 0? → process_key(key)
    │
    ├── Task 3: g_ms_cnt++
    │     └── g_ms_cnt >= 1000? → g_ms_cnt=0, tick_second()
    │
    ├── Task 3b: LED D1
    │     ├── STATE_DISPLAY_OFF → SET_LED1_OFF
    │     ├── g_ms_cnt < 500    → SET_LED1_ON
    │     └── g_ms_cnt >= 500   → SET_LED1_OFF
    │
    ├── Task 4: Blink & Timeout (chỉ khi SET_HOUR hoặc SET_ALARM)
    │     ├── g_blink_cnt++
    │     │     └── >= 500ms → toggle g_blink_on → update_display()
    │     └── g_timeout_cnt++
    │           └── >= 30000ms → STATE=NORMAL, start_pips(1), update_display()
    │
    └── Task 5: update_buzzer()
          ├── g_alarm_active? → đếm g_alarm_remain_ms
          │     └── == 0 → tắt, g_alarm_active=0
          ├── g_pip_ms > 0   → đếm, == 0 → buzzer_off()
          │     └── g_pip_remain > 0 → bắt đầu g_pip_gap_cnt
          └── g_pip_gap_cnt > 0 → đếm, == 0 → buzzer_on() tiếng bíp tiếp theo
}
```

---

## 3. TICK_SECOND()

```
tick_second()
    │
    ├── g_state != NORMAL? → return (đồng hồ dừng khi cài đặt)
    │
    g_sec++
    │
    └── g_sec >= 60?
          YES → g_sec = 0, g_min++, update_display()
                └── g_min >= 60?
                      YES → g_min = 0, g_hour++
                            └── g_hour >= 24? → g_hour = 0
                            └── alarm check:
                                  g_alarm_enabled AND
                                  g_hour == g_alarm_hour AND
                                  g_min  == g_alarm_min
                                  → start_alarm_beep()
```

---

## 4. PROCESS_KEY(key)

```
process_key(key)
    │
    ├─ g_alarm_active?
    │     YES → stop alarm (g_alarm_active=0, buzzer_off()), return
    │
    ├─ STATE == DISPLAY_OFF?
    │     YES → STATE=NORMAL, update_display(), return
    │
    start_pips(1)              [Phản hồi 1 bíp]
    g_timeout_cnt = g_blink_cnt = 0
    g_blink_on = 1
    │
    ├─ KEY_DISP_OFF  → STATE=DISPLAY_OFF, update_display(), return
    │
    ├─ KEY_ALARM_OFF →
    │     g_alarm_enabled == 1? → set 0, start_pips(1)
    │     g_alarm_enabled == 0? → set 1, start_pips(2)
    │     eeprom_write(EEPROM_AL_ENABLED), return
    │
    ├─ KEY_SET_HOUR →
    │     STATE == SET_HOUR? → g_cursor++
    │     │     g_cursor > 3? → STATE=NORMAL, g_sec=0, g_ms_cnt=0
    │     STATE != SET_HOUR?  → STATE=SET_HOUR, g_cursor=0
    │     update_display(), return
    │
    ├─ KEY_SET_ALARM →
    │     STATE == SET_ALARM? → g_cursor++
    │     │     g_cursor > 3? → g_alarm_enabled=1,
    │     │                     save_alarm_to_eeprom(), STATE=NORMAL
    │     STATE != SET_ALARM? → STATE=SET_ALARM, g_cursor=0
    │     update_display(), return
    │
    ├─ STATE != SET_HOUR AND != SET_ALARM? → return (nuốt phím)
    │
    ph = &g_hour (SET_HOUR) hoặc &g_alarm_hour (SET_ALARM)
    pm = &g_min  (SET_HOUR) hoặc &g_alarm_min  (SET_ALARM)
    │
    ├─ KEY_NUM_0~9 → set_cursor_digit(g_cursor, digit, ph, pm)
    │                  clamp: h>23→23, m>59→59
    ├─ KEY_PLUS    → inc_cursor(g_cursor, ph, pm)  [wrap-around]
    └─ KEY_MINUS   → dec_cursor(g_cursor, ph, pm)  [wrap-around]
    │
    update_display()
```

---

## 5. UPDATE_DISPLAY()

```
update_display()
    │
    ├─ STATE == DISPLAY_OFF?
    │     → segment_buff[0..3] = 0x00, LED_ALARM_OFF(), return
    │
    h = g_hour   (NORMAL/SET_HOUR)
    h = g_alarm_hour (SET_ALARM)
    m tương tự
    │
    segment_buff[0] = SEGMENT_TABLE[h/10]
    segment_buff[1] = SEGMENT_TABLE[h%10] | 0x80  (DP)
    segment_buff[2] = SEGMENT_TABLE[m/10]
    segment_buff[3] = SEGMENT_TABLE[m%10]
    │
    ├─ SET_HOUR hoặc SET_ALARM AND g_blink_on == 0?
    │     → segment_buff[g_cursor] = 0x00  (tắt chữ số đang chỉnh)
    │
    └─ SET_ALARM?
          g_blink_on=1 → LED_ALARM_ON()
          g_blink_on=0 → LED_ALARM_OFF()
       else → LED_ALARM_OFF()
```

---

## 6. CURSOR MODE — ĐỘ PHÂN GIẢI TỪNG VỊ TRÍ

```
g_cursor │ Đối tượng │ Bước (+/−) │ Phạm vi hợp lệ
─────────┼───────────┼────────────┼────────────────
    0    │  H_tens   │   ±10 giờ  │  giờ: 0~23
    1    │  H_units  │   ±1 giờ   │  giờ: 0~23
    2    │  M_tens   │   ±10 phút │  phút: 0~59
    3    │  M_units  │   ±1 phút  │  phút: 0~59

Nhấn số N tại cursor C:
  C=0: hour = N×10 + (hour%10)  → clamp ≤ 23
  C=1: hour = (hour/10)×10 + N  → clamp ≤ 23
  C=2: min  = N×10 + (min%10)   → clamp ≤ 59
  C=3: min  = (min/10)×10 + N   → clamp ≤ 59
```

---

## 7. SƠ ĐỒ STATE MACHINE

```
                    ┌──────────────────────┐
         ┌──────────│      NORMAL (0)       │──────────┐
         │          │  Đồng hồ HH.MM chạy  │          │
         │          │  LED D1 blink 1Hz     │          │
         │          └──────────────────────┘          │
         │           │         │         │             │
      SW4↓        SW8↓      SW16↓    Alarm fire       │
  ┌──────────┐ ┌──────────┐ ┌──────────┐              │
  │SET_HOUR  │ │SET_ALARM │ │DISP_OFF  │              │
  │  (1)     │ │  (2)     │ │  (3)     │              │
  │Cursor    │ │Cursor    │ │All LEDs  │              │
  │blinks    │ │blinks    │ │  OFF     │              │
  │LED D0 OFF│ │LED D0 ON │ │          │              │
  └──────────┘ └──────────┘ └──────────┘              │
  SW4×4→NRM   SW8×4→NRM   AnyKey→NRM                 │
  Timeout30s  +saveEEPROM                             │
                                                start_alarm_beep()
                                                Buzzer ON/OFF 500ms
                                                tối đa 60 giây
                                                AnyKey→ stop
```

---

## 8. BUZZER STATE MACHINE (update_buzzer)

```
Mỗi 1ms:
┌─────────────────────────────────────────────────────┐
│  g_alarm_active == 1?                               │
│    YES: g_alarm_remain_ms--                         │
│         g_alarm_phase_ms--                          │
│         phase_ms==0 → toggle buzzer (500ms ON/OFF)  │
│         remain==0   → g_alarm_active=0, buzzer_off  │
│    NO:                                              │
│      g_pip_ms > 0?                                  │
│        YES: g_pip_ms--                              │
│             ==0 → buzzer_off()                      │
│                   g_pip_remain>0 → g_pip_gap_cnt=200│
│      g_pip_gap_cnt > 0?                             │
│        YES: g_pip_gap_cnt--                         │
│             ==0 && g_pip_remain>0                   │
│               → g_pip_remain--, g_pip_ms=200        │
│                 buzzer_on()  ← tiếng bíp tiếp theo  │
└─────────────────────────────────────────────────────┘
```
