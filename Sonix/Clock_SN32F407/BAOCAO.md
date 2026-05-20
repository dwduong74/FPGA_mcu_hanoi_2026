# BÁO CÁO ĐỒ ÁN: ĐỒNG HỒ KỸ THUẬT SỐ TRÊN VI ĐIỀU KHIỂN SN32F407

---

## 1. TỔNG QUAN DỰ ÁN

| Mục | Nội dung |
|-----|----------|
| Tên đồ án | Đồng hồ kỹ thuật số có báo thức |
| Vi điều khiển | SN32F407 (ARM Cortex-M0, 12 MHz) |
| Board | SN32F407_EVK (Evaluation Kit) |
| Công cụ | Keil MDK v5, ARM Compiler 6, CMSIS-DAP |
| Ngôn ngữ | C (C99) |

---

## 2. PHẦN CỨNG SỬ DỤNG

### 2.1 Sơ đồ kết nối GPIO

| Ngoại vi | Chân MCU | Ghi chú |
|----------|----------|---------|
| 7-SEG Segment A~G, DP | P0.0 ~ P0.7 | Active-HIGH |
| 7-SEG COM0 (Chục giờ) | P1.9 | Active-HIGH qua transistor |
| 7-SEG COM1 (Đơn vị giờ) | P1.10 | + dấu chấm phân cách |
| 7-SEG COM2 (Chục phút) | P1.11 | |
| 7-SEG COM3 (Đơn vị phút) | P1.12 | |
| Key ROW 1~4 | P1.4 ~ P1.7 | Hàng ma trận phím |
| Key COL 1~4 | P2.4 ~ P2.7 | Cột ma trận phím |
| LED D0 | P3.8 | Active-LOW, báo trạng thái alarm |
| LED D1 | P3.9 | Active-LOW, nhấp nháy 1 Hz theo giây |
| Buzzer | P3.0 | CT16B0 PWM, tần số 600 Hz |
| EEPROM (I2C0) | P0.10 (SCL), P0.11 (SDA) | PFPA option 2 |

### 2.2 Bàn phím 4×4 (16 phím)

```
        COL1(P2.4) COL2(P2.5) COL3(P2.6) COL4(P2.7)
ROW1    SW1  = 7   SW2  = 8   SW3  = 9   SW4  = Cài giờ
ROW2    SW5  = 4   SW6  = 5   SW7  = 6   SW8  = Cài báo thức
ROW3    SW9  = 1   SW10 = 2   SW11 = 3   SW12 = Bật/Tắt báo thức
ROW4    SW13 = (−) SW14 = 0   SW15 = (+) SW16 = Tắt màn hình
```

---

## 3. TÍNH NĂNG HỆ THỐNG

### 3.1 Hiển thị đồng hồ
- Định dạng **HH.MM** (24 giờ), quét 4 LED 7-đoạn theo phương pháp multiplexing
- LED D1 nhấp nháy **1 Hz** để chỉ thị giây đang chạy (không hiển thị giây trên 7-SEG)

### 3.2 Cài đặt giờ (Cursor Mode)
1. Nhấn **SW4** → vào chế độ chỉnh giờ, con trỏ tại chữ số H_tens
2. Chữ số tại con trỏ **nhấp nháy** để báo hiệu đang chỉnh
3. Nhấn **số 0~9** → cài trực tiếp; giá trị tự clamp nếu vượt giới hạn (VD: ô H_tens nhấn 9 → kết quả 23)
4. Nhấn **(+)/(−)** → tăng/giảm có wrap-around theo bước tương ứng
5. Nhấn **SW4** tiếp → con trỏ tiến sang ô kế (H_tens → H_units → M_tens → M_units)
6. Sau ô cuối → tự về NORMAL (giây reset về 00)
7. Không nhấn phím **30 giây** → tự thoát về NORMAL

### 3.3 Báo thức
- Cài đặt tương tự cài giờ bằng **SW8**; xác nhận cuối lưu vào EEPROM và bật báo thức
- Khi đến giờ → buzzer kêu **ON/OFF 500 ms** liên tục, tối đa **60 giây**
- **Bất kỳ phím nào** → dừng tiếng báo thức ngay lập tức

### 3.4 Bật/Tắt báo thức (SW12)
| Trạng thái hiện tại | Hành động | Âm thanh |
|---------------------|-----------|----------|
| Báo thức đang TẮT | Bật lên | **2 tiếng bíp** |
| Báo thức đang BẬT | Tắt đi | **1 tiếng bíp** |
- Trạng thái được lưu vào EEPROM ngay lập tức

### 3.5 Lưu trữ EEPROM
| Địa chỉ | Dữ liệu |
|---------|---------|
| 0x00 | Giờ báo thức |
| 0x01 | Phút báo thức |
| 0x02 | Trạng thái bật/tắt (0x01 = bật, 0x00 = tắt) |

### 3.6 Startup Animation
Khi bật nguồn → màn hình **nhấp nháy giờ báo thức 2 lần** (×400 ms ON/OFF) trước khi chạy đồng hồ → giúp kiểm tra nhanh giá trị đã lưu.

### 3.7 Tắt màn hình (SW16)
- Tắt toàn bộ 7-SEG, LED D0, LED D1 để tiết kiệm điện
- Bất kỳ phím nào → bật lại về NORMAL

---

## 4. CÁC KỸ THUẬT LẬP TRÌNH ỨNG DỤNG

### 4.1 Ngắt Timer (CT16B1) — Khung thời gian 1 ms
CT16B1 tạo ngắt mỗi 1 ms (MR9 = 11999 tại 12 MHz). ISR đặt cờ `timer_1ms_flag`. Vòng `while(1)` kiểm tra cờ và thực hiện các task tuần tự trong cùng chu kỳ 1 ms:

```
Task 1 → Quét 7-SEG (Digital_Scan)
Task 2 → Quét phím (KeyScan + xử lý)
Task 3 → Đếm ms → tick_second() mỗi 1000 ms
Task 3b→ Điều khiển LED D1 (ON < 500 ms, OFF ≥ 500 ms)
Task 4 → Nhấp nháy cursor + timeout SET mode
Task 5 → Cập nhật buzzer (pip sequence / alarm cycle)
```

**Lợi ích:** Tất cả ngoại vi cập nhật đúng chu kỳ, không bị lệch thời gian do xử lý tuần tự.

### 4.2 PWM Buzzer (CT16B0)
CT16B0 phát xung PWM 600 Hz ra P3.0:
- **MR9** = 19999 (chu kỳ), **MR0** = 9999 (duty 50%)
- Bật/tắt bằng cách ghi MR0 (0 = tắt, 9999 = bật)
- Chuỗi **pip sequence**: biến `g_pip_remain` + `g_pip_gap_cnt` quản lý 1 hoặc 2 tiếng bíp tự động trong `update_buzzer()`

### 4.3 Quét 7-Đoạn Multiplexing
4 COM quét tuần tự mỗi 1 ms → tần số refresh = 250 Hz (không flicker):
1. Tắt tất cả SEG và COM
2. Tăng `com_scan` (0→1→2→3→0)
3. Bật COM tương ứng HIGH
4. Ghi `segment_buff[com_scan]` ra P0

### 4.4 Quét Ma Trận Phím với Debounce
KeyScan() chạy mỗi 1 ms:
1. Set ROW output LOW, đọc COL → phát hiện cột có phím
2. Đổi chiều: set COL output LOW, đọc ROW → xác định hàng
3. Debounce: phím phải ổn định **50 ms** mới ghi nhận; xóa nếu giữ > 200 ms

### 4.5 State Machine 4 trạng thái

```
         ┌──────────────────────────────────┐
         │           NORMAL                 │
         │  Đồng hồ chạy, LED D1 blink      │
         └──┬───────┬───────────┬───────────┘
      SW4 ↓   SW8 ↓         SW16 ↓
    ┌──────┐ ┌──────────┐ ┌─────────────┐
    │SET_  │ │SET_ALARM │ │DISPLAY_OFF  │
    │HOUR  │ │Cursor    │ │Tất cả tắt  │
    └──────┘ └──────────┘ └─────────────┘
    SW4×4→  SW8×4→       Bất kỳ phím→
    NORMAL  NORMAL+SAVE  NORMAL
```

### 4.6 Cursor Mode — Nhập Số Trực Tiếp
- `g_cursor` (0~3) trỏ vào từng chữ số: H_tens, H_units, M_tens, M_units
- Nhấn phím số → `set_cursor_digit()`: thay thế chữ số, clamp kết quả
- Nhấn (+)/(−) → `inc_cursor()`/`dec_cursor()`: bước theo magnitude (±10h hoặc ±1h/phút) với wrap-around

### 4.7 I2C + EEPROM
Giao tiếp I2C0 polling (busy-wait) với EEPROM ngoài:
- `eeprom_write()`: START → địa chỉ slave W → địa chỉ ô → data → STOP
- `eeprom_read()`: START → addr W → reg → RESTART → addr R → read → NACK → STOP
- Dữ liệu validation khi đọc: giá trị 0xFF (EEPROM trắng) → dùng default an toàn

### 4.8 Watchdog Timer (WDT)
WDT reset sau ~250 ms nếu không feed. `__WDT_FEED_VALUE` đặt trong `while(1)` đảm bảo chạy liên tục. Nếu hệ thống treo (I2C hang, stack overflow...) → WDT tự reset MCU.

---

## 5. CẤU TRÚC FILE DỰ ÁN

```
Clock_SN32F407/
├── Source/
│   ├── UserAPP/
│   │   └── main.c          ← Logic chính: state machine, xử lý phím, tick
│   ├── Driver/
│   │   ├── GPIO.c/h        ← Khởi tạo chân I/O
│   │   ├── CT16B0.c/h      ← PWM buzzer 600 Hz
│   │   ├── CT16B1.c/h      ← Timer 1 ms ngắt
│   │   ├── I2C0.c / I2C.h  ← Giao tiếp I2C polling
│   │   ├── WDT.c/h         ← Watchdog Timer
│   │   ├── PFPA.c/h        ← Pin Function Assignment
│   │   └── Utility.c/h     ← Delay, tiện ích
│   └── Module/
│       ├── Segment.c/h     ← Bảng mã 7-đoạn, Digital_Scan()
│       ├── KeyScan.c/h     ← Ma trận phím 4×4, debounce
│       └── EEPROM.c/h      ← eeprom_read(), eeprom_write()
└── Clock_SN32F407.uvprojx  ← Keil project file
```

---

## 6. KẾT QUẢ

| Tính năng | Trạng thái |
|-----------|-----------|
| Hiển thị giờ HH.MM | ✅ |
| Nhấp nháy LED D1 theo giây | ✅ |
| Cài giờ bằng phím số / +/− | ✅ |
| Cài báo thức, lưu EEPROM | ✅ |
| Bật/tắt báo thức (SW12) | ✅ |
| Báo thức kêu 60s, tắt bằng phím | ✅ |
| Startup hiện giờ báo thức 2 lần | ✅ |
| Tắt màn hình tiết kiệm điện | ✅ |
| WDT bảo vệ hệ thống | ✅ |
