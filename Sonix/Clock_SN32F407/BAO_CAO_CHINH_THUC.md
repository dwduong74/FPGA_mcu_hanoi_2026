# BÁO CÁO ĐỒ ÁN
# THIẾT KẾ ĐỒNG HỒ KỸ THUẬT SỐ CÓ BÁO THỨC
# TRÊN VI ĐIỀU KHIỂN SN32F407

---

| | |
|---|---|
| **Môn học** | Hệ thống nhúng / Lập trình vi điều khiển |
| **Vi điều khiển** | SN32F407 (ARM Cortex-M0, 12 MHz) |
| **Board thực nghiệm** | SN32F407_EVK |
| **Công cụ phát triển** | Keil MDK v5, ARM Compiler 6, CMSIS-DAP |

---

## I. ĐẶT VẤN ĐỀ

Đồng hồ kỹ thuật số là ứng dụng nền tảng trong lĩnh vực hệ thống nhúng, tích hợp đồng thời nhiều ngoại vi: hiển thị LED, bàn phím ma trận, lưu trữ không mất dữ liệu và tín hiệu âm thanh. Đây là bài toán lý tưởng để thực hành lập trình hệ thống nhúng vì đòi hỏi quản lý thời gian thực, điều phối nhiều ngoại vi đồng thời và thiết kế giao diện người dùng trực quan trên phần cứng tối thiểu.

**Mục tiêu đồ án:**
- Hiển thị giờ và phút theo định dạng 24 giờ trên 4 LED 7-đoạn
- Cho phép cài đặt giờ và báo thức thông qua bàn phím ma trận 4×4
- Lưu trữ cài đặt báo thức vào EEPROM để giữ lại sau khi mất điện
- Phát tín hiệu âm thanh qua buzzer khi đến giờ báo thức
- Cung cấp giao diện thân thiện với phản hồi âm thanh và hình ảnh

---

## II. PHÂN TÍCH YÊU CẦU

### 2.1 Yêu cầu chức năng

| STT | Chức năng | Mô tả |
|-----|-----------|-------|
| 1 | Hiển thị thời gian | Hiển thị HH.MM, cập nhật mỗi phút |
| 2 | Chỉ thị giây | LED D1 nhấp nháy 1 Hz |
| 3 | Cài đặt giờ | Nhập từng chữ số bằng phím, cursor nhấp nháy |
| 4 | Cài đặt báo thức | Tương tự cài giờ, lưu EEPROM |
| 5 | Bật/tắt báo thức | Phím toggle, phản hồi âm thanh khác nhau |
| 6 | Kêu báo thức | Buzzer 60 giây, tắt bằng phím bất kỳ |
| 7 | Thông báo khởi động | Nhấp nháy giờ báo thức 2 lần khi bật nguồn |
| 8 | Tắt màn hình | Tắt toàn bộ LED tiết kiệm điện |

### 2.2 Yêu cầu kỹ thuật

| Thông số | Giá trị |
|----------|---------|
| Tần số clock | 12 MHz (IHRC nội bộ) |
| Chu kỳ quét 7-SEG | 1 ms / COM → refresh 250 Hz |
| Độ chính xác thời gian | ±1 ms/s (dựa vào timer phần cứng) |
| Thời gian debounce phím | 50 ms |
| Timeout chế độ cài đặt | 30 giây |
| Thời lượng báo thức | 60 giây (có thể tắt sớm) |
| Giao tiếp EEPROM | I2C0, polling mode |

---

## III. THIẾT KẾ HỆ THỐNG

### 3.1 Sơ đồ khối hệ thống

```
┌─────────────────────────────────────────────────────────────────┐
│                        SN32F407 MCU                             │
│                                                                 │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐   ┌──────────┐  │
│  │ CT16B1   │    │ CT16B0   │    │  GPIO    │   │  I2C0    │  │
│  │ Timer1ms │    │ PWM600Hz │    │  P0~P3   │   │ P0.10/11 │  │
│  └────┬─────┘    └────┬─────┘    └────┬─────┘   └────┬─────┘  │
│       │               │               │               │         │
└───────┼───────────────┼───────────────┼───────────────┼─────────┘
        │               │               │               │
        ▼               ▼               ▼               ▼
   [1ms tick]       [Buzzer]    [7-SEG + LED]       [EEPROM]
   [KeyScan]         P3.0       P0: Segment          0xA0/A1
                               P1.9-12: COM
                               P1.4-7: Key ROW
                               P2.4-7: Key COL
                               P3.8: LED D0
                               P3.9: LED D1
```

### 3.2 Thiết kế State Machine

Hệ thống được thiết kế theo mô hình máy trạng thái 4 trạng thái:

```
                      Bật nguồn
                          │
                     [Animation]
                    Nhấp nháy báo thức 2 lần
                          │
                          ▼
              ┌───────────────────────┐
         ┌────│     NORMAL (0)        │────┐
         │    │  Đồng hồ chạy bình   │    │
         │    │  thường, LED D1 blink │    │
         │    └───────────────────────┘    │
      SW4│         │SW8       │SW16        │Báo thức đến giờ
         ▼         ▼          ▼            ▼
    ┌─────────┐ ┌──────────┐ ┌──────────┐ ┌──────────────┐
    │SET_HOUR │ │SET_ALARM │ │DISP_OFF  │ │ALARM ACTIVE  │
    │ (1)     │ │ (2)      │ │ (3)      │ │Buzzer kêu    │
    │Cursor   │ │Cursor    │ │Tất cả    │ │60 giây       │
    │nhấp nháy│ │nhấp nháy │ │tắt       │ │              │
    │LED D0=0 │ │LED D0=1  │ │          │ │              │
    └─────────┘ └──────────┘ └──────────┘ └──────────────┘
    SW4×4→NRM  SW8×4→NRM    AnyKey→NRM   AnyKey→stop
    Timeout30s +saveEEPROM
```

### 3.3 Bàn phím — Layout và chức năng

```
    ┌────────┬────────┬────────┬────────────────┐
    │  7     │  8     │  9     │  Cài giờ [SW4] │
    ├────────┼────────┼────────┼────────────────┤
    │  4     │  5     │  6     │  Cài BT  [SW8] │
    ├────────┼────────┼────────┼────────────────┤
    │  1     │  2     │  3     │ Bật/Tắt BT[SW12]│
    ├────────┼────────┼────────┼────────────────┤
    │ (−)    │  0     │ (+)    │  Tắt MH [SW16] │
    └────────┴────────┴────────┴────────────────┘
```

### 3.4 Giao diện cài đặt giờ (Cursor Mode)

```
Màn hình:   [ 1 ][ 4 ].[ 3 ][ 0 ]
                              ↑
                         cursor đây (nhấp nháy)

Nhấn "5" → [ 1 ][ 4 ].[ 3 ][ 5 ]    ← cài trực tiếp
Nhấn "+" → [ 1 ][ 4 ].[ 3 ][ 6 ]    ← tăng 1 phút
Nhấn SW4→ cursor dịch sang ô tiếp theo
```

---

## IV. CÁC KỸ THUẬT LẬP TRÌNH ỨNG DỤNG

### 4.1 Timer Interrupt + Task Scheduling

**Vấn đề:** Cần điều phối đồng thời: quét 7-SEG (1 ms), quét phím (1 ms),
đếm giây (1000 ms), nhấp nháy (500 ms), timeout (30000 ms), buzzer.

**Giải pháp:** CT16B1 tạo ngắt phần cứng mỗi 1 ms, đặt cờ `timer_1ms_flag`.
Tất cả task chạy tuần tự trong cùng khung 1 ms — không cần RTOS, không xung đột.

```c
// ISR CT16B1 (mỗi 1ms):
timer_1ms_flag = 1;

// while(1):
if (timer_1ms_flag) {
    timer_1ms_flag = 0;
    Digital_Scan();      // 1ms
    KeyScan();           // 1ms
    g_ms_cnt++;          // đếm giây
    update_buzzer();     // quản lý âm thanh
}
```

### 4.2 7-Segment Multiplexing

Kỹ thuật chia sẻ bus segment giữa 4 chữ số, quét tuần tự với tần số đủ nhanh
(250 Hz) để mắt người không nhận ra sự nhấp nháy.

**Công thức refresh:** f_refresh = f_scan / số_digit = 1000Hz / 4 = 250 Hz > 60 Hz (ngưỡng mắt người)

### 4.3 Key Matrix Scanning với Debounce

Phát hiện phím trong ma trận 4×4 bằng phương pháp quét 2 chiều (set ROW → đọc COL,
set COL → đọc ROW). Chống rung cơ học bằng bộ đếm 50 ms.

### 4.4 State Machine với Cursor UI

Thiết kế UI nhập liệu dựa trên con trỏ (cursor) từng chữ số — trực quan hơn
phương pháp +/- toàn phần và tận dụng bàn phím số sẵn có.

### 4.5 Non-volatile Storage (EEPROM qua I2C)

Lưu trữ cấu hình báo thức vào EEPROM AT24Cxx qua I2C. Chiến lược ghi thông minh:
chỉ ghi khi có thay đổi từ người dùng, tránh hao mòn EEPROM.

**EEPROM Map:**

| Địa chỉ | Dữ liệu | Kích thước |
|---------|---------|------------|
| 0x00 | Giờ báo thức | 1 byte |
| 0x01 | Phút báo thức | 1 byte |
| 0x02 | Trạng thái bật/tắt | 1 byte |

### 4.6 PWM Buzzer với Pip Sequence

CT16B0 tạo xung PWM 600 Hz (tần số dễ nghe). Máy trạng thái buzzer
trong `update_buzzer()` quản lý tự động:
- **1 bíp:** phản hồi thao tác thông thường
- **2 bíp:** xác nhận bật báo thức
- **Kêu liên tục:** báo thức (ON 500ms / OFF 500ms)

### 4.7 Watchdog Timer

WDT bảo vệ hệ thống tự phục hồi khi gặp sự cố phần mềm. Vị trí khởi tạo
WDT được đặt **sau toàn bộ init** để tránh reset giả do I2C polling mất thời gian.

---

## V. KẾT QUẢ THỰC NGHIỆM

### 5.1 Kiểm thử chức năng

| STT | Chức năng kiểm thử | Kết quả |
|-----|-------------------|---------|
| 1 | Hiển thị HH.MM, cập nhật đúng mỗi phút | ✅ Đạt |
| 2 | LED D1 nhấp nháy 1 Hz đều đặn | ✅ Đạt |
| 3 | Cài giờ: nhập số trực tiếp, cursor nhấp nháy | ✅ Đạt |
| 4 | Cài giờ: clamp tự động (VD: ô H_tens nhấn 9 → 23) | ✅ Đạt |
| 5 | Cài giờ: +/− wrap-around | ✅ Đạt |
| 6 | Cài giờ: timeout 30s tự thoát | ✅ Đạt |
| 7 | Báo thức lưu EEPROM, giữ sau rút điện | ✅ Đạt |
| 8 | SW12: toggle bật/tắt, âm thanh 1/2 bíp | ✅ Đạt |
| 9 | Báo thức kêu đúng giờ, tắt bằng phím | ✅ Đạt |
| 10 | Startup animation hiện giờ báo thức 2 lần | ✅ Đạt |
| 11 | SW16: tắt toàn bộ LED, phím bật lại | ✅ Đạt |
| 12 | WDT không reset trong điều kiện bình thường | ✅ Đạt |

### 5.2 Thông số đo được

| Thông số | Giá trị đo | Yêu cầu |
|----------|-----------|---------|
| Tần số refresh 7-SEG | 250 Hz | > 60 Hz |
| Độ trễ phản hồi phím | < 50 ms | < 100 ms |
| Tần số buzzer | 600 Hz | 400~1000 Hz |
| Flash size | ~12 KB / 128 KB | < 80% |
| RAM sử dụng | ~0.5 KB / 8 KB | < 80% |

---

## VI. KẾT LUẬN

Đồ án đã hoàn thành đầy đủ các mục tiêu đề ra, xây dựng thành công một hệ thống
đồng hồ kỹ thuật số hoàn chỉnh trên vi điều khiển SN32F407. Các kỹ thuật lập trình
nhúng quan trọng được vận dụng thực tiễn:

- **Timer interrupt** điều phối đa nhiệm không cần RTOS
- **Multiplexing** tối ưu sử dụng GPIO
- **State machine** thiết kế hệ thống điều khiển rõ ràng
- **EEPROM I2C** lưu trữ dữ liệu không mất khi mất điện
- **WDT** đảm bảo độ tin cậy hệ thống

**Hướng phát triển tiếp theo:**
- Thêm module RTC DS3231 để giữ giờ chính xác khi mất điện
- Thêm cảm biến nhiệt độ hiển thị xen kẽ với giờ
- Giao tiếp UART để cài đặt giờ từ máy tính

---

## PHỤ LỤC — CẤU TRÚC FILE NGUỒN

```
Clock_SN32F407/
├── Source/
│   ├── UserAPP/main.c        — Logic chính (~700 dòng)
│   ├── Driver/
│   │   ├── GPIO.c/h          — Cấu hình chân vào/ra
│   │   ├── CT16B0.c/h        — Buzzer PWM 600 Hz
│   │   ├── CT16B1.c/h        — Timer ngắt 1 ms
│   │   ├── I2C0.c / I2C.h    — Giao tiếp I2C polling
│   │   ├── WDT.c/h           — Watchdog Timer
│   │   ├── PFPA.c/h          — Gán chân ngoại vi
│   │   └── Utility.c/h       — Hàm delay
│   └── Module/
│       ├── Segment.c/h       — Mã hóa 7-đoạn, quét COM
│       ├── KeyScan.c/h       — Quét ma trận phím, debounce
│       └── EEPROM.c/h        — Đọc/ghi EEPROM qua I2C
├── BAOCAO.md                 — Báo cáo tóm tắt
├── BAO_CAO_CHINH_THUC.md     — Báo cáo chính thức (file này)
├── LUONG_CODE.md             — Sơ đồ luồng code
├── GIAI_THICH_CODE.md        — Giải thích code chi tiết
└── KICH_BAN_VIDEO.md         — Kịch bản quay video demo
```
