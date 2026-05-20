# GIẢI THÍCH CODE — ĐỒNG HỒ SN32F407

> Tài liệu này giải thích từng phần code bằng ngôn ngữ đơn giản,
> dành cho người đọc code lần đầu hoặc cần trình bày với hội đồng.

---

## PHẦN 1 — KHỞI TẠO HỆ THỐNG

### 1.1 Tại sao cần `SystemInit()` và `SystemCoreClockUpdate()`?

```c
SystemInit();
SystemCoreClockUpdate();
```

SN32F407 sau khi bật nguồn cần cấu hình nguồn clock nội bộ (IHRC = 12 MHz).
`SystemInit()` viết vào các thanh ghi hệ thống để chọn nguồn clock.
`SystemCoreClockUpdate()` cập nhật biến `SystemCoreClock` — biến này được các
thư viện CMSIS dùng để tính toán delay, baudrate, v.v.
**Thiếu 2 dòng này → toàn bộ timing sai.**

---

### 1.2 PFPA — Tại sao cần gán chân?

```c
PFPA_Init();
```

SN32F407 cho phép một ngoại vi có thể ra nhiều chân khác nhau (pin mux).
Ví dụ: I2C0 có thể dùng P0.6/P0.7 hoặc P0.10/P0.11.
`PFPA_Init()` nói với MCU: "I2C0 dùng P0.10+P0.11, buzzer PWM dùng P3.0".
**Không gán → ngoại vi hoạt động nhưng tín hiệu không ra chân vật lý.**

---

### 1.3 GPIO_Init() — Cấu hình chân vào/ra

```c
SN_GPIO0->MODE = 0xFF;    // P0.0~P0.7 output (8 đoạn 7-seg)
SN_GPIO1->MODE = 0xf<<4;  // P1.4~P1.7 input  (key ROW)
SN_GPIO1->MODE |= 0xf<<9; // P1.9~P1.12 output (4 COM)  ← dùng |= để giữ ROW
SN_GPIO2->MODE = 0xf<<4;  // P2.4~P2.7 input  (key COL)
SN_GPIO3->MODE_b.MODE8 = 1; // P3.8 output (LED D0)
SN_GPIO3->MODE_b.MODE9 = 1; // P3.9 output (LED D1)
```

**Lưu ý quan trọng:** Dòng COM phải dùng `|=` (OR bằng), không dùng `=`.
Nếu dùng `=` sẽ xóa trắng cấu hình ROW đã set trước → phím không hoạt động.

---

### 1.4 CT16B1_Init() — Timer 1 ms

```c
SN_CT16B1->MR9 = 11999;   // Đếm từ 0 đến 11999 = 12000 tick = 1ms tại 12MHz
```

CT16B1 là bộ đếm phần cứng. Cứ 1 ms nó đặt cờ `timer_1ms_flag = 1`.
Vòng `while(1)` kiểm tra cờ này để biết "đã qua 1 ms chưa".

**Tại sao không dùng delay?**
Hàm `UT_DelayNms(1)` sẽ **chặn** CPU trong 1 ms — không làm được gì khác.
Timer interrupt cho phép CPU làm nhiều việc xen kẽ trong 1 ms đó.

---

### 1.5 WDT_Init() — Chó canh nhà

```c
WDT_Init();
// Trong while(1):
__WDT_FEED_VALUE;
```

WDT (Watchdog Timer) đếm ngược. Nếu code không "cho chó ăn" trong ~250 ms,
WDT tự reset MCU. Điều này bảo vệ hệ thống khi:
- Code rơi vào vòng lặp vô hạn do lỗi
- I2C bị treo (EEPROM không phản hồi)
- Stack overflow

**WDT khởi tạo CUỐI CÙNG** — sau tất cả init — để tránh reset trong lúc
I2C đang chờ EEPROM phản hồi (có thể mất vài ms).

---

## PHẦN 2 — TIMER 1 MS VÀ CÁC TASK

### 2.1 Cấu trúc vòng lặp chính

```c
while (1)
{
    __WDT_FEED_VALUE;

    if (timer_1ms_flag)
    {
        timer_1ms_flag = 0;
        // Task 1, 2, 3, 4, 5...
    }
}
```

Vòng lặp chạy liên tục. Nhưng các task chỉ chạy **đúng mỗi 1 ms**
nhờ cờ `timer_1ms_flag`. Kết quả: mọi thứ đồng bộ, không bị lệch timing.

---

### 2.2 Task 1 — Digital_Scan() — Quét 7 đoạn

Màn hình 7-SEG của chúng ta có 4 chữ số nhưng chỉ dùng **8 chân segment chung**.
Không thể sáng 4 chữ cùng lúc vì sẽ hiện số giống nhau.

**Giải pháp: Multiplexing (quét tuần tự)**

```
Thời điểm 0ms: Bật COM0, gửi segment "H_tens"  → mắt thấy chữ số hàng chục giờ
Thời điểm 1ms: Bật COM1, gửi segment "H_units" → mắt thấy chữ số đơn vị giờ
Thời điểm 2ms: Bật COM2, gửi segment "M_tens"
Thời điểm 3ms: Bật COM3, gửi segment "M_units"
Thời điểm 4ms: Quay lại COM0...
```

Mắt người không nhận ra sự thay đổi nhanh (refresh 250 Hz) → trông như sáng đồng thời.

**Vì sao phải tắt segment trước khi đổi COM?**
Nếu đổi COM khi segment vẫn sáng → chữ số "nhòe" sang COM kế tiếp trong khoảnh khắc.

---

### 2.3 Task 2 — KeyScan() — Quét phím ma trận

**Vấn đề với 16 phím:** Nếu mỗi phím một chân → cần 16 chân GPIO.
**Giải pháp ma trận 4×4:** Chỉ cần 4 ROW + 4 COL = 8 chân.

```
Bước 1: Set ROW output LOW, đọc COL
        → Nếu COL nào LOW tức là có phím ở cột đó đang bấm

Bước 2: Đổi chiều — Set COL output LOW, đọc ROW
        → Xác định hàng nào đang bấm

Kết hợp ROW + COL → biết chính xác phím nào
```

**Debounce — chống rung phím:**
Nút nhấn cơ học bị "rung" khi tiếp xúc, tạo ra nhiều tín hiệu ON/OFF nhanh.
Code yêu cầu phím phải ổn định **50 ms** liên tục mới tính là 1 lần nhấn.

---

### 2.4 Task 3 — Đếm giây

```c
g_ms_cnt++;
if (g_ms_cnt >= 1000U)
{
    g_ms_cnt = 0;
    tick_second();   // 1000 ms = 1 giây
}
```

Đơn giản: đếm 1000 lần 1ms = 1 giây. Không cần RTC (Real Time Clock).

**Hạn chế:** Không có pin RTC → khi rút điện, giờ mất về 00:00.
Giải pháp trong project: khởi động từ 00:00, người dùng cài giờ thủ công.

---

### 2.5 Task 3b — LED D1 nhấp nháy 1 Hz

```c
if (g_ms_cnt < 500U)
    SET_LED1_ON;   // Nửa giây đầu: bật
else
    SET_LED1_OFF;  // Nửa giây sau: tắt
```

`g_ms_cnt` chạy từ 0→999 rồi reset. Chỉ cần so sánh với 500 là có LED nhấp nháy
**chính xác 1 Hz**, đồng bộ với đồng hồ, không cần thêm biến.

---

### 2.6 Task 4 — Nhấp nháy cursor và timeout

**Nhấp nháy cursor:**
```c
g_blink_cnt++;
if (g_blink_cnt >= 500)    // Mỗi 500ms
{
    g_blink_cnt = 0;
    g_blink_on = !g_blink_on;   // Toggle
    update_display();
}
```

`update_display()` kiểm tra `g_blink_on`: nếu false → set `segment_buff[g_cursor] = 0`
(tắt chữ số đang chỉnh) → tạo hiệu ứng nhấp nháy **chỉ tại vị trí con trỏ**.

**Timeout 30s:**
Nếu người dùng bỏ quên ở chế độ SET, sau 30 giây tự thoát.
Mỗi lần nhấn phím: `g_timeout_cnt = 0` → reset đếm ngược.

---

## PHẦN 3 — XỬ LÝ PHÍM (process_key)

### 3.1 Thứ tự ưu tiên xử lý

```
1. Báo thức đang kêu? → Dừng buzzer, thoát ngay (nuốt phím)
2. Màn hình tắt?      → Bật màn hình, thoát ngay
3. Phím chức năng     → Xử lý (DISP_OFF, ALARM_OFF, SET_HOUR, SET_ALARM)
4. Phím số / +/−      → Chỉ có tác dụng khi đang SET
```

Thứ tự này đảm bảo: khi báo thức đang kêu, bấm bất kỳ phím nào cũng tắt ngay
mà không vô tình kích hoạt chức năng khác.

---

### 3.2 SW12 — Toggle báo thức

```c
if (g_alarm_enabled)          // Đang BẬT
{
    g_alarm_enabled = 0;
    start_pips(1);            // 1 bíp = tắt
}
else                          // Đang TẮT
{
    g_alarm_enabled = 1;
    start_pips(2);            // 2 bíp = bật
}
eeprom_write(...);            // Lưu ngay vào EEPROM
```

Âm thanh phản hồi khác nhau giúp người dùng biết trạng thái **mà không cần nhìn màn hình**.

---

### 3.3 Cursor Mode — Nhập số trực tiếp

Khi cursor ở vị trí 0 (chục giờ) và người dùng nhấn phím "5":

```c
// set_cursor_digit(cursor=0, digit=5, &g_hour, &g_min)
h = 5 * 10 + (g_hour % 10);  // Giữ đơn vị giờ cũ
// Ví dụ: giờ cũ là 14 → h = 50 + 4 = 54
if (h > 23) h = 23;           // 54 > 23 → clamp về 23
g_hour = 23;
```

Nhấn "2" tiếp (cursor tự động ở chỗ cũ hoặc người dùng ấn SW4 để sang ô kế):
```c
h = 2 * 10 + (23 % 10);      // = 20 + 3 = 23 ✓
```

**Wrap-around cho +/−:**
```c
// inc_cursor tại cursor=0 (bước ±10 giờ)
*ph = (*ph + 10) % 24;        // 20 + 10 = 30 % 24 = 6 (wrap về 06:xx)
```

---

## PHẦN 4 — HIỂN THỊ (update_display)

### 4.1 Bảng mã 7-đoạn

```
Chữ số  Đoạn bật          Giá trị hex
   0    A,B,C,D,E,F       0x3F
   1    B,C               0x06
   2    A,B,D,E,G         0x5B
   ...
   9    A,B,C,D,F,G       0x6F
```

`SEGMENT_TABLE[số]` tra bảng → ra byte gửi vào P0.

**Dấu chấm phân cách (HH.MM):**
```c
segment_buff[1] = SEGMENT_TABLE[h%10] | 0x80;   // | 0x80 bật bit DP
```
Chỉ ô COM1 (đơn vị giờ) mới có dấu chấm → hiển thị "HH.MM".

### 4.2 Nhấp nháy cursor

```c
// Khi g_blink_on == 0 (nửa chu kỳ tắt):
segment_buff[g_cursor] = 0x00;   // Tắt toàn bộ đoạn của ô đang chỉnh
```

Các ô khác giữ nguyên → chỉ ô cursor "biến mất" rồi "hiện lại" mỗi 500 ms.

---

## PHẦN 5 — BUZZER

### 5.1 PWM 600 Hz

```
CT16B0 đếm từ 0 đến 19999 = 20000 tick tại 12 MHz
  → 12,000,000 / 20,000 = 600 Hz ✓

Duty 50%: MR0 = 9999
  Khi đếm ≤ 9999: chân P3.0 HIGH (bật buzzer)
  Khi đếm > 9999: chân P3.0 LOW  (tắt buzzer)
```

### 5.2 Chuỗi 2 tiếng bíp

```
g_pip_remain = 1 (còn 1 bíp sau bíp đầu)
g_pip_ms     = 200 ms → buzzer ON

Sau 200ms: buzzer OFF, g_pip_gap_cnt = 200 ms (khoảng lặng)
Sau 200ms gap: g_pip_remain-- = 0, buzzer ON lần 2
Sau 200ms: buzzer OFF, xong

Timeline: [BÍP 200ms][YÊN 200ms][BÍP 200ms] = tổng 600ms
```

### 5.3 Báo thức kêu

```
ON 500ms → OFF 500ms → ON 500ms → OFF 500ms → ... (tối đa 60,000 ms)

Biến g_alarm_phase_ms đếm ngược 500ms mỗi pha.
Biến g_alarm_remain_ms đếm tổng thời gian còn lại.
Bấm phím bất kỳ: g_alarm_active = 0 → buzzer_off() → dừng ngay.
```

---

## PHẦN 6 — EEPROM

### 6.1 Giao thức I2C Write

```
MCU → [START] → [0xA0 W] → [Địa chỉ ô 0x00] → [Data] → [STOP]
                    ↑
              Địa chỉ EEPROM (7-bit + bit W=0)
```

### 6.2 Giao thức I2C Read

```
MCU → [START] → [0xA0 W] → [Địa chỉ ô] → [RESTART] → [0xA1 R] → [đọc data] → [NACK] → [STOP]
                                                ↑                                  ↑
                                        Cần restart để đổi chiều           NACK = không đọc nữa
```

### 6.3 Tại sao không lưu giờ hiện tại?

EEPROM chịu được **~100,000 lần ghi**. Nếu lưu mỗi giây:
```
100,000 lần / (3600 giây/giờ × 24 giờ) = 1.15 ngày → EEPROM hỏng sau hơn 1 ngày!
```

Chỉ lưu khi người dùng **chủ động cài đặt** → EEPROM tồn tại hàng chục năm.

---

## PHẦN 7 — STARTUP ANIMATION

```c
// Hiển thị giờ báo thức nhấp nháy 2 lần trước khi chạy
uint8_t n_half = 0;   // Số half-period đã qua (cần 4 = 2 blinks)
while (n_half < 4)
{
    __WDT_FEED_VALUE;        // Feed WDT để không bị reset trong lúc animation
    if (timer_1ms_flag)
    {
        Digital_Scan();      // Vẫn phải quét 7-SEG để màn hình sáng
        ms++;
        if (ms >= 400)       // Mỗi 400ms đổi trạng thái
        {
            ms = 0; n_half++;
            show = !show;    // Toggle hiện/ẩn
            // Cập nhật segment_buff...
        }
    }
}
```

**Tại sao vẫn phải gọi `Digital_Scan()` trong animation?**
7-SEG dùng multiplexing — nếu không quét liên tục, màn hình sẽ tắt hoặc chỉ sáng 1 ô.
