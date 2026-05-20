===========================================================================
 DIGITAL CLOCK - SN32F407_EVK
 Hướng dẫn thiết lập project trong Keil MDK-ARM
===========================================================================

1. CẤU TRÚC THƯ MỤC
---------------------------------------------------------------------------
Clock_SN32F407/
├── README.txt                    ← File này
├── Source/
│   ├── UserAPP/
│   │   └── main.c               ← ★ CODE CHÍNH (đã viết)
│   ├── Driver/
│   │   ├── PFPA.c               ← ★ File riêng cho clock (đã viết)
│   │   ├── GPIO.h               ← Sao chép từ exp10
│   │   ├── GPIO.c               ← Sao chép từ exp10
│   │   ├── CT16B0.h             ← Sao chép từ exp8
│   │   ├── CT16B0.c             ← Sao chép từ exp8
│   │   ├── CT16B1.h             ← Sao chép từ exp10
│   │   ├── CT16B1.c             ← Sao chép từ exp10
│   │   ├── CT16.h               ← Sao chép từ exp10
│   │   ├── I2C.h                ← Sao chép từ exp10
│   │   ├── I2C0.c               ← Sao chép từ exp10
│   │   ├── WDT.h                ← Sao chép từ exp10
│   │   ├── WDT.c                ← Sao chép từ exp10
│   │   ├── Utility.h            ← Sao chép từ exp10
│   │   ├── Utility.c            ← Sao chép từ exp10
│   │   ├── SysTick.h            ← Sao chép từ exp10
│   │   └── SysTick.c            ← Sao chép từ exp10
│   └── Module/
│       ├── Segment.h            ← Sao chép từ exp10
│       ├── Segment.c            ← Sao chép từ exp10
│       ├── KeyScan.h            ← Sao chép từ exp10
│       ├── KeyScan.c            ← Sao chép từ exp10
│       ├── EEPROM.h             ← Sao chép từ exp10
│       └── EEPROM.c             ← Sao chép từ exp10

Thư mục nguồn để sao chép:
  exp8  = C:\Sonix\MCU Sonix Document\407\SN32F407_EVK_DEMO_KeilV6\exp8_adjust_pitch_of_the_buzzer\
  exp10 = C:\Sonix\MCU Sonix Document\407\SN32F407_EVK_DEMO_KeilV6\exp10_R&W_EEPROM_with_button_matrix\

2. TẠO PROJECT TRONG KEIL MDK-ARM
---------------------------------------------------------------------------
Cách nhanh nhất: sao chép toàn bộ exp10, rồi thay file:

Bước 1: Copy thư mục exp10 → đổi tên thành Clock_SN32F407
Bước 2: Mở Clock_SN32F407.uvprojx bằng Keil MDK
Bước 3: Trong Project Explorer:
         - Thay Source/UserAPP/main.c bằng file main.c đã viết
         - Thêm Source/Driver/CT16B0.h/c từ exp8 (cho buzzer)
         - Thay Source/Driver/PFPA.c bằng PFPA.c đã viết
         - Xóa PFPA.c cũ của exp10 ra khỏi project
Bước 4: Project → Options for Target → Output:
         - Đặt tên output file: Clock_SN32F407
Bước 5: Build (F7) → kiểm tra 0 errors, 0 warnings

3. CẤU HÌNH KẾT NỐI FILE TRONG KEIL
---------------------------------------------------------------------------
Trong Project Explorer, cần có các nhóm sau:

  [UserAPP]
    main.c

  [Driver]
    GPIO.c
    CT16B0.c     ← từ exp8
    CT16B1.c     ← từ exp10
    I2C0.c       ← từ exp10
    WDT.c
    Utility.c
    SysTick.c
    PFPA.c       ← file mới đã viết

  [Module]
    Segment.c
    KeyScan.c
    EEPROM.c

  [RTE/Device]
    system_SN32F400.c   ← từ RTE folder của exp10

4. MÔ PHỎNG BẰNG KEIL SIMULATOR
---------------------------------------------------------------------------
Bước 1: Build project thành công (0 errors)
Bước 2: Debug → Start/Stop Debug Session (Ctrl+F5)
         Lần đầu sẽ hỏi về debugger → chọn "Use Simulator"
Bước 3: Project → Options for Target → Debug
         → chọn "Use Simulator" (không cần phần cứng)
Bước 4: Trong Simulator:
         - View → Watch Windows → Watch 1: thêm g_hour, g_min, g_sec
         - View → Watch Windows → Watch 2: thêm g_state, g_alarm_hour, g_alarm_min
         - View → Peripheral → GPIO để xem trạng thái chân
         - Run (F5) → đồng hồ bắt đầu chạy
Bước 5: Thay đổi giá trị GPIO giả lập để kiểm tra phím

5. KIỂM TRA CHỨC NĂNG
---------------------------------------------------------------------------
  Chức năng          Phím       Kiểm tra trong Watch Window
  ---------------    -------    ------------------------------------
  Chạy đồng hồ       -          g_sec tăng mỗi giây
  Vào chỉnh giờ      SW3        g_state = 1 (SET_HOUR)
  Tăng giờ           SW6        g_hour tăng
  Vào chỉnh phút     SW3        g_state = 2 (SET_MIN)
  Xác nhận           SW3        g_state = 0 (NORMAL)
  Vào chỉnh alarm    SW16       g_state = 3 (SET_ALARM_HOUR)
  Xác nhận alarm     SW16       g_state = 0, EEPROM được ghi
  Timeout 30s        -          g_state tự trở về 0 sau 30s

6. LƯU Ý QUAN TRỌNG
---------------------------------------------------------------------------
  - HCLK mặc định = 12 MHz (IHRC), không cần thay đổi
  - Không cần thay đổi system_SN32F400.c
  - Timer 1ms được cung cấp bởi CT16B1 (MR9 = 11999)
  - Buzzer trên P3.0 qua CT16B0 PWM0 (PFPA option 1)
  - I2C0 trên P0.10(SCL) / P0.11(SDA) (PFPA option 2, = 0x0A)
  - LED D0 báo alarm: P3.8, active-LOW (0=sáng)
  - EEPROM địa chỉ 0xA0 (write) / 0xA1 (read)
===========================================================================
