# BÁO CÁO ĐỒ ÁN
# THIẾT KẾ ĐỒNG HỒ SỐ CÓ BÁO THỨC TRÊN VI ĐIỀU KHIỂN SN32F407

---

**Sinh viên thực hiện:** *(Điền tên)*
**Lớp / Khóa:** *(Điền lớp)*
**Giáo viên hướng dẫn:** *(Điền tên)*
**Ngày hoàn thành:** Tháng 5, 2026

---

## I. ĐẶT VẤN ĐỀ

Đồng hồ số là một trong những ứng dụng nhúng cơ bản và thiết thực nhất, yêu cầu tích hợp nhiều ngoại vi trên cùng một vi điều khiển: bộ định thời, hiển thị, giao tiếp người dùng và lưu trữ. Đồ án này xây dựng một đồng hồ số hoàn chỉnh trên kit SN32F407, với mục tiêu:

- Hiển thị giờ theo định dạng 24h trên màn hình LED 7-đoạn 4 chữ số
- Cho phép người dùng cài đặt giờ và báo thức thông qua bàn phím ma trận
- Lưu trữ cài đặt báo thức vào EEPROM, không mất dữ liệu khi mất điện
- Tích hợp buzzer báo thức và LED chỉ thị trạng thái

---

## II. GIỚI THIỆU PHẦN CỨNG

### 2.1 Vi điều khiển SN32F407

| Thông số | Giá trị |
|----------|---------|
| Kiến trúc | ARM Cortex-M0 |
| Xung nhịp | 12 MHz (IHRC nội bộ) |
| Flash ROM | 128 KB |
| SRAM | 8 KB |
| GPIO | 4 port (P0~P3), tối đa 56 chân |
| Timer | CT16B0~CT16B5 (16-bit) |
| Giao tiếp | I2C, SPI, UART |
| Đặc điểm | PFPA (Pin Function Pin Assignment) — gán linh hoạt chân ngoại vi |

### 2.2 Sơ đồ kết nối phần cứng

```
SN32F407
  P0.0~P0.7 ──────────── SEG A~G, DP (7-đoạn, active-HIGH)
  P1.9~P1.12 ─────────── COM0~COM3    (qua transistor, active-HIGH)
  P1.4~P1.7  ─────────── KEY ROW 1~4  (input pull-up)
  P2.4~P2.7  ─────────── KEY COL 1~4  (input pull-up)
  P3.8       ─────────── LED D0        (active-LOW, qua điện trở)
  P3.9       ─────────── LED D1        (active-LOW, qua điện trở)
  P3.0       ─────────── Buzzer        (PWM qua transistor)
  P0.10      ─────────── EEPROM SCL    (I2C0, PFPA option 2)
  P0.11      ─────────── EEPROM SDA    (I2C0, PFPA option 2)
```

### 2.3 Bàn phím 4×4

Bàn phím được thiết kế theo kiểu số điện thoại + phím chức năng:

```
┌──────┬──────┬──────┬──────────────┐
│  7   │  8   │  9   │  Cài giờ     │  ROW1
├──────┼──────┼──────┼──────────────┤
│  4   │  5   │  6   │  Cài báo thức│  ROW2
├──────┼──────┼──────┼──────────────┤
│  1   │  2   │  3   │ Bật/Tắt BT  │  ROW3
├──────┼──────┼──────┼──────────────┤
│  −   │  0   │  +   │  Tắt màn hình│  ROW4
└──────┴──────┴──────┴──────────────┘
 COL1   COL2   COL3       COL4
```

---

## III. THIẾT KẾ HỆ THỐNG

### 3.1 Kiến trúc phần mềm

Phần mềm chia thành 3 lớp rõ ràng:

```
┌─────────────────────────────────────────┐
│  Lớp ứng dụng: main.c                   │
│  State machine, xử lý phím, buzzer      │
├───────────────────┬─────────────────────┤
│  Module:          │  Module:            │
│  Segment.c        │  KeyScan.c          │
│  EEPROM.c         │                     │
├───────────────────┴─────────────────────┤
│  Driver: GPIO, CT16B0/B1, I2C0, WDT     │
├─────────────────────────────────────────┤
│  CMSIS / SN32F400 Register Access       │
└─────────────────────────────────────────┘
```

### 3.2 Sơ đồ trạng thái (State Machine)

Hệ thống có 4 trạng thái hoạt động:

```
              ┌────────────────────────────┐
    ┌─────────│        NORMAL              │─────────┐
    │         │  Đồng hồ HH.MM chạy        │         │
    │         │  LED D1 nhấp nháy 1 Hz     │         │
    │         └─────┬──────────┬───────────┘         │
    │          SW4↓ │     SW8↓ │    SW16↓             │
    │         ┌─────┴─┐ ┌──────┴──┐ ┌──────────┐     │
    │         │SET_   │ │SET_ALARM│ │DISP_OFF  │     │
    │         │HOUR   │ │Cursor   │ │Tắt điện  │     │
    │         │Cursor │ │LED D0   │ │blink     │     │
    │         │blink  │ │blink    │ │          │     │
    │         └───────┘ └─────────┘ └──────────┘     │
    │         SW4×4      SW8×4+lưu    AnyKey          │
    │         →NORMAL   →NORMAL      →NORMAL          │
    │         Timeout30s Timeout30s                    │
    └─────────────────────── Báo thức → kêu 60s ──────┘
```

### 3.3 Cơ chế định thời 1 ms

CT16B1 được cấu hình với `MR9 = 11999` tại HCLK 12 MHz, tạo ngắt chính xác mỗi 1 ms. Trong mỗi ngắt, cờ `timer_1ms_flag` được đặt. Vòng `while(1)` phát hiện cờ và thực thi tuần tự các task:

| Task | Mô tả | Chu kỳ |
|------|-------|--------|
| 1 | Digital_Scan() — quét 1 COM | 1 ms |
| 2 | KeyScan() — đọc phím | 1 ms |
| 3 | Đếm ms → tick_second() | 1 ms (tick giây: 1000 ms) |
| 3b | Điều khiển LED D1 | 1 ms |
| 4 | Blink cursor + timeout | 1 ms |
| 5 | update_buzzer() | 1 ms |

### 3.4 Kỹ thuật hiển thị 7-đoạn Multiplexing

Thay vì điều khiển 4 digit đồng thời (cần 4×8 = 32 chân), multiplexing dùng 8+4 = 12 chân, sáng từng digit theo vòng tại tần số 250 Hz:

```
ms=0: COM0 ON → segment_buff[0] → HH (chục giờ)
ms=1: COM1 ON → segment_buff[1] → HH (đơn vị giờ) + DP
ms=2: COM2 ON → segment_buff[2] → MM (chục phút)
ms=3: COM3 ON → segment_buff[3] → MM (đơn vị phút)
ms=4: COM0 ON → ... (lặp lại)
```

Mắt người không phân biệt được trên 50 Hz → nhìn thấy 4 chữ số sáng liên tục.

---

## IV. CÁC TÍNH NĂNG VÀ THỰC HIỆN

### 4.1 Hiển thị đồng hồ
- Định dạng **HH.MM**, hệ 24 giờ
- Dấu chấm (DP) cố định ở giữa tạo phân cách giờ:phút
- LED D1 nhấp nháy 1 Hz: ON 500 ms, OFF 500 ms — chỉ thị hệ thống đang chạy

### 4.2 Cài đặt giờ — Cursor Mode

Giao diện nhập liệu theo kiểu **con trỏ di chuyển**:
- Con trỏ trỏ vào từng chữ số, chữ số đó nhấp nháy
- Nhập số 0~9 trực tiếp → cài ngay, tự clamp nếu vượt giới hạn
- Nhấn +/− → tăng/giảm có wrap-around
- Nhấn nút cài giờ → con trỏ tiến sang ô tiếp theo
- Hoàn tất 4 ô → lưu, giây reset về 00

### 4.3 Báo thức

- Cài đặt tương tự cài giờ
- Xác nhận → lưu EEPROM ngay lập tức → bền vững qua mất điện
- Khi đến giờ: buzzer kêu ON/OFF 500 ms liên tục, tối đa 60 giây
- Bấm bất kỳ phím nào → tắt tiếng

### 4.4 Bật/Tắt báo thức (SW12 — Toggle)

| Trạng thái | Hành động | Phản hồi âm thanh |
|------------|-----------|------------------|
| Đang TẮT | Bật lên | 2 tiếng bíp |
| Đang BẬT | Tắt đi | 1 tiếng bíp |

Trạng thái được lưu vào EEPROM ngay khi nhấn.

### 4.5 Startup Animation

Khi bật nguồn → hiển thị **giờ báo thức đã lưu nhấp nháy 2 lần** (1,6 giây) trước khi chạy đồng hồ. Mục đích: người dùng có thể kiểm tra nhanh giá trị đã lưu mà không cần vào menu cài đặt.

### 4.6 Tắt màn hình (SW16)
- Tắt toàn bộ LED: 7-SEG, D0, D1
- Giảm tiêu thụ điện khi không sử dụng
- Bất kỳ phím nào → bật lại

---

## V. KỸ THUẬT XỬ LÝ NỔI BẬT

### 5.1 Chống rung phím (Debounce)
Debounce phần mềm 50 ms: phím phải ổn định liên tục 50 lần kiểm tra (mỗi 1 ms) mới được ghi nhận. Loại bỏ hoàn toàn hiện tượng rung tiếp điểm cơ học.

### 5.2 Chuỗi bíp tự động (Pip Sequence)
Buzzer state machine quản lý chuỗi N tiếng bíp liên tiếp mà không dùng delay:
- `g_pip_ms`: thời gian bíp đang phát
- `g_pip_remain`: số bíp còn lại
- `g_pip_gap_cnt`: khoảng lặng 200 ms giữa các bíp

### 5.3 Bảo vệ Watchdog Timer
WDT cấu hình timeout 250 ms. Feed trong `while(1)` → đảm bảo hệ thống luôn phản hồi. Trường hợp I2C treo hoặc ngoại lệ phần cứng → WDT tự reset MCU trong 250 ms.

### 5.4 EEPROM — Chiến lược ghi thông minh
Chỉ ghi EEPROM khi người dùng **xác nhận** cài đặt, không ghi định kỳ. EEPROM có tuổi thọ ~100.000 lần ghi/ô — chiến lược này kéo dài tuổi thọ hàng chục năm.

---

## VI. KẾT QUẢ THỰC NGHIỆM

| Tính năng | Kết quả | Ghi chú |
|-----------|---------|---------|
| Hiển thị HH.MM | ✅ Đúng | Multiplexing 250 Hz, không flicker |
| LED D1 nhấp nháy 1 Hz | ✅ Đúng | Đồng bộ chính xác với g_ms_cnt |
| Cài giờ cursor mode | ✅ Đúng | Clamp hoạt động đúng |
| Wrap-around +/− | ✅ Đúng | 23→0, 0→23 cho giờ |
| Báo thức lưu EEPROM | ✅ Đúng | Còn sau khi rút điện cắm lại |
| Báo thức kêu 60s | ✅ Đúng | Dừng đúng sau 60s |
| Tắt tiếng bằng phím | ✅ Đúng | Phản hồi < 2ms |
| Toggle alarm 1/2 bíp | ✅ Đúng | |
| Startup animation | ✅ Đúng | 1,6 giây khi bật nguồn |
| Tắt màn hình SW16 | ✅ Đúng | Tất cả LED tắt |
| WDT bảo vệ | ✅ Hoạt động | Test bằng cách short I2C |

---

## VII. KẾT LUẬN

Đồ án đã xây dựng thành công đồng hồ số hoàn chỉnh trên nền tảng SN32F407, tích hợp đầy đủ các ngoại vi: timer ngắt, PWM, GPIO, I2C, và các kỹ thuật phần mềm quan trọng: state machine, multiplexing, debounce, buzzer sequence, và EEPROM persistence.

Điểm nổi bật của thiết kế là **kiến trúc task-based 1 ms** — tất cả công việc được đồng bộ hóa qua một nhịp timer duy nhất, đảm bảo hệ thống hoạt động ổn định, dễ mở rộng và bảo trì.

---

## PHỤ LỤC — CẤU TRÚC DỰ ÁN

```
Clock_SN32F407/
├── Source/
│   ├── UserAPP/main.c       ← Logic chính
│   ├── Driver/
│   │   ├── GPIO.c/h
│   │   ├── CT16B0.c/h       ← PWM 600 Hz
│   │   ├── CT16B1.c/h       ← Timer 1 ms
│   │   ├── I2C0.c / I2C.h
│   │   ├── WDT.c/h
│   │   ├── PFPA.c/h
│   │   └── Utility.c/h
│   └── Module/
│       ├── Segment.c/h
│       ├── KeyScan.c/h
│       └── EEPROM.c/h
├── BAOCAO_CHINH_THUC.md     ← File này
├── GIAITHICH_CODE.md
├── LUONG_CODE.md
└── Clock_SN32F407.uvprojx
```
