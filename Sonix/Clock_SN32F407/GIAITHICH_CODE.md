# GIẢI THÍCH CODE — ĐỒNG HỒ SỐ SN32F407

---

## MỤC LỤC
1. [Tổng quan kiến trúc](#1-tổng-quan-kiến-trúc)
2. [Driver/GPIO.c — Khởi tạo chân I/O](#2-drivergpioc)
3. [Driver/CT16B1.c — Timer 1 ms](#3-driverct16b1c)
4. [Driver/CT16B0.c — PWM Buzzer](#4-driverct16b0c)
5. [Driver/I2C0.c — Giao tiếp I2C](#5-driveri2c0c)
6. [Module/Segment.c — Hiển thị 7-đoạn](#6-modulesegmentc)
7. [Module/KeyScan.c — Quét phím](#7-modulekeyscanc)
8. [Module/EEPROM.c — Lưu trữ](#8-moduleeepromc)
9. [UserAPP/main.c — Logic chính](#9-userappmainc)

---

## 1. TỔNG QUAN KIẾN TRÚC

Chương trình chia thành 3 lớp:

```
┌─────────────────────────────────────┐
│         UserAPP / main.c            │  ← Logic ứng dụng (state machine,
│   State machine, xử lý phím,        │     xử lý phím, cập nhật giờ)
│   quản lý buzzer, EEPROM            │
├──────────────┬──────────────────────┤
│   Module/    │   Module/            │  ← Lớp module trừu tượng
│  Segment.c   │  KeyScan.c           │
│  EEPROM.c    │                      │
├──────────────┴──────────────────────┤
│   Driver/ (GPIO, CT16B0/1, I2C0,    │  ← Lớp driver phần cứng
│   WDT, PFPA, Utility)               │
├─────────────────────────────────────┤
│   CMSIS / SN32F400 HAL              │  ← Lớp truy cập thanh ghi
└─────────────────────────────────────┘
```

**Nguyên tắc thiết kế:**
- CT16B1 tạo nhịp 1 ms → toàn bộ logic chạy trong `timer_1ms_flag` block
- Không dùng `delay()` trong main loop → hệ thống luôn phản hồi nhanh
- WDT bảo vệ: nếu code bị treo → reset sau 250 ms

---

## 2. Driver/GPIO.c

### Mục đích
Cấu hình chế độ (input/output) và trạng thái ban đầu cho tất cả chân GPIO.

### Giải thích từng dòng

```c
SN_GPIO1->MODE = 0xf << 4;   // P1.4~P1.7: input (KEY ROW)
SN_GPIO2->MODE = 0xf << 4;   // P2.4~P2.7: input (KEY COL)
```
> Phím ma trận cần chân input để đọc mức logic khi quét.

```c
SN_GPIO0->MODE = 0xff;        // P0.0~P0.7: output (SEG A~G,DP)
SN_GPIO0->BCLR = 0xff;        // Tắt tất cả segment ban đầu
```
> 8 bit tương ứng 8 đoạn a,b,c,d,e,f,g,dp. BCLR = "Bit CLeaR" → kéo xuống 0.

```c
SN_GPIO1->MODE |= 0x0f << 9;  // P1.9~P1.12: output (COM0~COM3)
SN_GPIO1->BCLR  = 0x0f << 9;  // COM ban đầu = LOW (tất cả digit tắt)
```
> Dùng `|=` thay vì `=` để **giữ nguyên** cấu hình ROW (P1.4~P1.7) đã set ở trên.
> ⚠️ Đây là lỗi phổ biến: nếu dùng `=` sẽ xóa cấu hình ROW, phím không hoạt động.

```c
SN_GPIO3->MODE_b.MODE8 = 1;   // P3.8: output → LED D0
SN_GPIO3->MODE_b.MODE9 = 1;   // P3.9: output → LED D1
SET_LED0_OFF;  SET_LED1_OFF;  // Active-LOW → HIGH = tắt
```

---

## 3. Driver/CT16B1.c

### Mục đích
Tạo ngắt mỗi **1 ms** — nhịp tim của toàn bộ hệ thống.

### Cách hoạt động

```
HCLK = 12 MHz → 12,000,000 clock/giây
Muốn ngắt mỗi 1ms = 1000 lần/giây
→ MR9 = 12,000,000 / 1000 − 1 = 11,999
```

```c
SN_CT16B1->MR9 = 11999;       // Đếm từ 0 đến 11999 → reset (12000 ticks = 1ms)
// Bật ngắt khi MR9 match + reset bộ đếm
SN_CT16B1->MCTRL |= (1<<29) | (1<<30);
NVIC_EnableIRQ(CT16B1_IRQn);
```

### ISR (Interrupt Service Routine)

```c
void CT16B1_IRQHandler(void)
{
    uint32_t ris = SN_CT16B1->RIS;   // Đọc cờ ngắt
    if (ris & (1<<5))                 // Bit 5 = MR9 match
    {
        timer_1ms_flag = 1;           // Báo hiệu cho main loop
        SN_CT16B1->IC = (1<<5);       // Xóa cờ ngắt
    }
}
```

> **Tại sao không xử lý trực tiếp trong ISR?**
> ISR chỉ đặt cờ `timer_1ms_flag = 1`. Xử lý thực tế (quét phím, cập nhật display...) diễn ra trong `while(1)`. Cách này giữ ISR ngắn gọn, tránh ảnh hưởng đến timing của các ngắt khác.

---

## 4. Driver/CT16B0.c

### Mục đích
Phát xung PWM tần số **600 Hz** điều khiển buzzer qua chân P3.0.

### Tính toán thông số

```
Tần số buzzer = 600 Hz
Chu kỳ (period) = HCLK / f = 12,000,000 / 600 = 20,000 ticks
→ MR9 (reload) = 20,000 − 1 = 19,999

Duty cycle 50% (âm thanh cân xứng, to nhất):
→ MR0 (compare) = 10,000 − 1 = 9,999
```

### Bật/Tắt buzzer trong main.c

```c
// Bật: cài MR0 = nửa chu kỳ
void buzzer_on(void) {
    SN_CT16B0->MR9 = 19999;
    SN_CT16B0->MR0 = 9999;
    SN_CT16B0->TMRCTRL = 1;   // Start timer
}

// Tắt: MR0 = 0 → PWM luôn LOW → không có xung → im
void buzzer_off(void) {
    SN_CT16B0->MR0 = 0;
}
```

---

## 5. Driver/I2C0.c

### Mục đích
Giao tiếp I2C với EEPROM ngoài (địa chỉ 0xA0/0xA1).

### Cơ chế polling (busy-wait)

```c
void I2C0_Start(void) {
    SN_I2C0->STAT = 1 << 15;     // Xóa cờ trạng thái
    SN_I2C0->CTRL_b.STA = 1;     // Phát tín hiệu START
    while((SN_I2C0->STAT & (1<<15)) == 0); // Chờ đến khi START xong
}
```

> Mỗi hàm I2C đều có vòng `while(...)` chờ phần cứng hoàn tất.
> ⚠️ **Rủi ro:** Nếu EEPROM không phản hồi → treo vĩnh viễn tại đây.
> ✅ **Giải pháp:** WDT sẽ reset MCU sau 250ms nếu điều này xảy ra.

### Chuỗi giao tiếp đọc EEPROM

```
Master → [START] [0xA0 W] [reg_addr] [RESTART] [0xA1 R] → [DATA] [NACK] [STOP]
                   ↑                                           ↑
              Ghi địa chỉ ô                             Đọc dữ liệu
```

---

## 6. Module/Segment.c

### Mục đích
Quản lý bảng mã 7-đoạn và hàm quét multiplexing.

### Bảng mã SEGMENT_TABLE

```c
// Mỗi phần tử là 1 byte: bit0=A, bit1=B, ..., bit6=G, bit7=DP
const uint8_t SEGMENT_TABLE[] = {
    0x3F,  // 0: A,B,C,D,E,F sáng
    0x06,  // 1: B,C sáng
    0x5B,  // 2: A,B,D,E,G sáng
    ...
};
```

### Digital_Scan() — Quét multiplexing

```
Mỗi lần gọi (1ms):
┌─────────────────────────────────────────┐
│ 1. Tắt toàn bộ SEG (P0 = 0)            │
│ 2. Tắt toàn bộ COM (P1.9~12 = 0)       │
│ 3. com_scan++ (0→1→2→3→0)              │
│ 4. Bật COM[com_scan] HIGH               │
│ 5. Ghi segment_buff[com_scan] ra P0     │
└─────────────────────────────────────────┘
```

> **Tại sao không flicker?**
> 4 COM × 1ms/COM = chu kỳ quét 4ms = **250 Hz**.
> Mắt người không phân biệt được trên 50 Hz → nhìn thấy 4 digit sáng liên tục.

> **Tại sao tắt SEG trước khi đổi COM?**
> Nếu đổi COM khi SEG còn sáng → "bóng ma" (ghost): digit sai thoáng hiện.

---

## 7. Module/KeyScan.c

### Mục đích
Đọc ma trận phím 4×4 với chống rung (debounce).

### Thuật toán quét 2 bước

**Bước 1 — Phát hiện cột:**
```
Set P1.4~P1.7 (ROW) → output LOW
Đọc P2.4~P2.7 (COL) → nếu có bit = 0: có phím nhấn ở cột đó
```

**Bước 2 — Xác định hàng:**
```
Set P2.4~P2.7 (COL) → output LOW
Đọc P1.4~P1.7 (ROW) → xác định hàng nào bị kéo xuống
```

**Kết hợp:** `key = key_col | key_row` → mã phím duy nhất 8-bit

### Debounce

```
key_debounce đếm mỗi 1ms:
  0~49ms:  chưa xác nhận (có thể là rung)
  50ms:    GHI NHẬN PHÍM (KEY_SHORT_PUSH_TIME = 50)
  51~200ms: đã xử lý, bỏ qua
  > 200ms: reset (KEY_DEBOUNCE_MAX_TIME = 200)
```

> Debounce 50ms đủ để lọc rung cơ học thông thường (5~20ms).

---

## 8. Module/EEPROM.c

### Mục đích
Đọc/ghi dữ liệu vào EEPROM I2C (AT24Cxx hoặc tương đương).

### eeprom_write()

```c
void eeprom_write(uint8_t addr, uint8_t reg, uint8_t *dat, uint16_t len)
{
    I2C0_Start();
    I2C_write_byte(addr);   // 0xA0 = địa chỉ slave + Write bit
    I2C_write_byte(reg);    // Địa chỉ ô nhớ EEPROM (0x00~0xFF)
    while(len--) I2C_write_byte(*dat++);  // Ghi từng byte
    I2C0_Stop();
}
```

> EEPROM cần thời gian ghi nội bộ ~5ms sau khi nhận STOP.
> Không nên đọc lại ngay → trong code có `UT_DelayNms()` nếu cần.

### Layout bộ nhớ EEPROM trong project

```
Địa chỉ  │ Nội dung              │ Khi nào ghi
─────────┼───────────────────────┼────────────────────
  0x00   │ Giờ báo thức (0~23)   │ Xác nhận SET_ALARM
  0x01   │ Phút báo thức (0~59)  │ Xác nhận SET_ALARM
  0x02   │ Alarm enabled (0/1)   │ SW12 toggle + SET_ALARM
```

---

## 9. UserAPP/main.c

### 9.1 Biến trạng thái chính

```c
g_state     // Trạng thái hiện tại: NORMAL/SET_HOUR/SET_ALARM/DISPLAY_OFF
g_cursor    // Vị trí con trỏ (0~3): H_tens/H_units/M_tens/M_units
g_hour      // Giờ hiện tại (0~23)
g_min       // Phút hiện tại (0~59)
g_sec       // Giây hiện tại (0~59)
g_ms_cnt    // Bộ đếm ms trong giây hiện tại (0~999)
g_blink_on  // 1=hiển thị, 0=ẩn → dùng cho nhấp nháy cursor
g_blink_cnt // Đếm ms cho nhấp nháy (0~499)
g_timeout_cnt // Đếm ms cho timeout 30s SET mode
```

### 9.2 set_cursor_digit() — Nhập số trực tiếp

```c
// VD: cursor=0 (H_tens), digit=2, g_hour=15
// → h = 2×10 + (15%10) = 20 + 5 = 25 → clamp → h = 23
set_cursor_digit(0, 2, &g_hour, &g_min);
```

**Ví dụ clamp thực tế:**

| cursor | Nhấn | Trước | Sau | Lý do |
|--------|------|-------|-----|-------|
| 0 (H_tens) | 9 | 05 | 23 | 95 > 23 → clamp |
| 1 (H_units) | 8 | 20 | 23 | 28 > 23 → clamp |
| 2 (M_tens) | 7 | 12 | 59 | 72 > 59 → clamp |
| 0 (H_tens) | 1 | 07 | 17 | 17 ≤ 23 → OK |

### 9.3 inc_cursor() / dec_cursor() — Tăng giảm có wrap

```c
// cursor=0: bước ±10 giờ với wrap tại 24
// g_hour=22 + 10 = 32 → 32 % 24 = 8
inc_cursor(0, &g_hour, &g_min);  // 22:xx → 08:xx

// cursor=1: bước ±1 giờ
// g_hour=0 - 1 → (0+24-1)%24 = 23
dec_cursor(1, &g_hour, &g_min);  // 00:xx → 23:xx
```

### 9.4 update_buzzer() — Chuỗi bíp

```
start_pips(2) được gọi:
  g_pip_remain = 1  (cần thêm 1 bíp sau)
  g_pip_ms     = 200ms
  buzzer_on()
        ↓ 200ms sau
  buzzer_off()
  g_pip_gap_cnt = 200ms  (khoảng lặng)
        ↓ 200ms sau
  g_pip_remain = 0
  g_pip_ms     = 200ms
  buzzer_on()
        ↓ 200ms sau
  buzzer_off()  ← Kết thúc chuỗi 2 bíp
```

### 9.5 Tại sao WDT được init SAU I2C?

```
Thứ tự:
  CT16B0_Init() → CT16B1_Init() → I2C0_Init() → load_eeprom() → WDT_Init()

Lý do: I2C dùng busy-wait (không có timeout).
Nếu WDT init TRƯỚC và EEPROM không phản hồi:
  → I2C treo → WDT reset → lại treo → WDT reset → vòng lặp vô hạn
  → Board không bao giờ khởi động được!

Giải pháp: Init WDT SAU khi I2C hoàn tất xong.
Sau đó WDT bảo vệ main loop (không bảo vệ init).
```
