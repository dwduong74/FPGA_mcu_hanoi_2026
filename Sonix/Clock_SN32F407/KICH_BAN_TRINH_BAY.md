# KỊCH BẢN TRÌNH BÀY SLIDE
# Đồng hồ kỹ thuật số SN32F407

**Thời lượng dự kiến:** 10 – 12 phút  
**Ghi chú:** Mỗi slide ~40–60 giây. Câu in *nghiêng* là lời nói trực tiếp.

---

## SLIDE 1 — Trang bìa
### "THIẾT KẾ ĐỒNG HỒ SỐ CÓ BÁO THỨC"

> *"Xin chào thầy/cô và các bạn. Hôm nay nhóm em xin trình bày đồ án:
> Thiết kế đồng hồ kỹ thuật số có báo thức, chạy trên vi điều khiển SN32F407
> của hãng Sonix — một chip ARM Cortex-M0 32-bit.
> Toàn bộ dự án được lập trình bằng ngôn ngữ C trên Keil MDK v5,
> theo kiến trúc bare-metal — tức là không dùng hệ điều hành,
> điều khiển phần cứng trực tiếp qua thanh ghi."*

**[Chuyển slide]**

---

## SLIDE 2 — Mục tiêu kỹ thuật hệ thống

> *"Trước tiên, em xin điểm qua 5 mục tiêu kỹ thuật cốt lõi của hệ thống."*

> *"Thứ nhất — Hiển thị thời gian thực: định dạng 24 giờ HH.MM qua 4 LED 7 đoạn."*

> *"Thứ hai — Giao diện điều khiển: người dùng cài đặt trực tiếp qua bàn phím ma trận 4×4
> với 16 phím — bao gồm numpad số và các phím chức năng."*

> *"Thứ ba — Lưu trữ non-volatile: giờ báo thức được lưu vào EEPROM I2C,
> đảm bảo không mất dữ liệu khi mất điện."*

> *"Thứ tư — Hệ thống cảnh báo: buzzer phát xung PWM tần số 600Hz tự động
> khi đến giờ báo thức."*

> *"Và thứ năm — Bảo vệ hệ thống bằng Watchdog Timer, tự động reset MCU
> nếu firmware bị treo vì bất kỳ lý do gì."*

**[Chuyển slide]**

---

## SLIDE 3 — Cốt lõi xử lý: SN32F407

> *"Đây là vi điều khiển trung tâm — SN32F407."*

> *"Chip sử dụng lõi ARM Cortex-M0 32-bit RISC, chạy ở tần số 12 MHz
> từ bộ dao động nội IHRC — không cần thạch anh ngoài."*

> *"Bộ nhớ gồm 128KB Flash để lưu firmware và 8KB SRAM cho dữ liệu runtime."*

> *"Chip có 2 timer 16-bit quan trọng:
> CT16B0 — dùng để phát PWM cho buzzer,
> và CT16B1 — tạo ngắt định kỳ 1 mili-giây làm nhịp tim cho toàn hệ thống."*

> *"Đặc biệt, chip có cơ chế PFPA — Pin Function Pin Assignment —
> cho phép gán chức năng linh hoạt cho từng chân GPIO.
> Ở đây em dùng để kết nối I2C0 ra chân P0.10 và P0.11."*

**[Chuyển slide]**

---

## SLIDE 4 — Sơ đồ kiến trúc phần cứng

> *"Slide này mô tả tổng thể cách MCU kết nối với các ngoại vi."*

> *"LED 7 đoạn: 8 bit segment A đến G cộng DP đi ra cổng P0.
> 4 chân COM điều khiển từng chữ số qua P1.9 đến P1.12,
> thông qua transistor khuếch đại dòng vì GPIO không đủ dòng để kéo trực tiếp."*

> *"Bàn phím ma trận 4×4: 4 hàng ROW ở P1.4 đến P1.7,
> 4 cột COL ở P2.4 đến P2.7."*

> *"EEPROM AT24C02 giao tiếp qua I2C0, địa chỉ slave 0xA0."*

> *"Buzzer kết nối với chân P3.0, điều khiển bằng PWM từ CT16B0."*

> *"Hai LED chỉ thị D0 và D1 ở P3.8 và P3.9 — active LOW —
> D0 báo trạng thái báo thức, D1 nhấp nháy theo giây."*

**[Chuyển slide]**

---

## SLIDE 5 — Giao diện nhập liệu: ma trận phím

> *"Đây là layout bàn phím 4×4 mà nhóm em thiết kế."*

> *"Phần numpad bên trái hoạt động như bàn phím số thông thường:
> hàng trên cùng là 7, 8, 9 — tương ứng SW1, SW2, SW3.
> Hàng giữa là 4, 5, 6 — hàng tiếp là 1, 2, 3.
> Và hàng cuối: SW13 là dấu trừ, SW14 là số 0, SW15 là dấu cộng."*

> *"Cột ngoài cùng bên phải là 4 phím chức năng được highlight màu cam:
> SW4 — Cài giờ,
> SW8 — Cài báo thức,
> SW12 — Bật/Tắt báo thức,
> SW16 — Tắt màn hình tiết kiệm điện."*

> *"Thiết kế này cho phép người dùng nhập số tự nhiên như máy tính bỏ túi,
> đồng thời tăng giảm từng chữ số bằng phím cộng trừ."*

**[Chuyển slide]**

---

## SLIDE 6 — Kiến trúc phần mềm C99 Bare-metal

> *"Phần mềm được tổ chức thành 3 tầng rõ ràng."*

> *"Tầng trên cùng — Application Layer — là file main.c,
> chứa toàn bộ logic: vòng lặp chính, state machine, xử lý phím và đếm giờ."*

> *"Tầng giữa — Module Layer — gồm 3 module độc lập:
> Segment.c phụ trách quét LED 7 đoạn,
> KeyScan.c xử lý ma trận phím với debounce,
> và EEPROM.c đóng gói giao tiếp I2C."*

> *"Tầng dưới cùng — Driver Layer — là các driver cấp thấp:
> CT16B.c cho timer và PWM, GPIO.c, I2C0.c và WDT.c."*

> *"Kiến trúc phân tầng này giúp code dễ bảo trì và mỗi module
> có thể test độc lập."*

**[Chuyển slide]**

---

## SLIDE 7 — Logic điều khiển: State Machine 4 trạng thái

> *"Toàn bộ logic điều khiển được mô hình hóa bằng state machine 4 trạng thái."*

> *"Trạng thái trung tâm là NORMAL — đồng hồ chạy bình thường,
> LED D1 nhấp nháy 1Hz để chỉ thị giây đang đếm."*

> *"Từ NORMAL, nhấn SW4 → vào SET_HOUR để chỉnh giờ.
> Nhấn SW8 → vào SET_ALARM để cài báo thức.
> Nhấn SW16 → vào DISPLAY_OFF, tắt toàn bộ LED tiết kiệm điện."*

> *"Ở SET_HOUR và SET_ALARM: sau khi nhập đủ 4 chữ số,
> hoặc không nhấn phím trong 30 giây,
> hệ thống tự động quay về NORMAL."*

> *"Riêng DISPLAY_OFF thì đơn giản hơn — nhấn bất kỳ phím nào là màn hình bật lại."*

> *"Còn SW12 — bật/tắt báo thức — không chuyển trạng thái,
> nó xử lý ngay trong NORMAL và phát tiếng bíp xác nhận."*

**[Chuyển slide]**

---

## SLIDE 8 — Lập lịch tác vụ: khung thời gian 1ms

> *"Đây là kỹ thuật quan trọng nhất trong kiến trúc phần mềm của dự án."*

> *"CT16B1 tạo ngắt mỗi 1 mili-giây — MR9 = 11999 tại 12MHz.
> ISR chỉ đơn giản bật một cờ timer_1ms_flag."*

> *"Vòng while(1) liên tục polling cờ này. Mỗi khi cờ bật,
> hệ thống thực hiện tuần tự 5 task trong cùng 1ms đó:
> Feed Watchdog, quét LED, quét phím, cập nhật thời gian, và cập nhật buzzer."*

> *"Ưu điểm của cách này: không dùng delay() gây block CPU,
> tất cả ngoại vi được phục vụ đúng chu kỳ,
> và toàn bộ timing được đồng bộ về một nguồn duy nhất là timer 1ms."*

> *"Đây là pattern cơ bản của lập lịch cooperative trong bare-metal,
> thường thấy trong các hệ thống nhúng công nghiệp."*

**[Chuyển slide]**

---

## SLIDE 9 — Hiển thị động Multiplexing & chống nhiễu

> *"4 LED 7 đoạn không thể sáng cùng lúc — MCU không đủ chân và dòng.
> Thay vào đó, ta dùng kỹ thuật multiplexing."*

> *"Mỗi 1ms, hệ thống lần lượt hiển thị từng chữ số theo 4 bước:
> Bước 1 — tắt toàn bộ segment trước, BCLR = 0xFF, để tránh ghosting —
> hiện tượng bóng ma khi chuyển COM.
> Bước 2 — tắt COM cũ.
> Bước 3 — chuyển sang COM mới.
> Bước 4 — bật segment mới lên."*

> *"Với chu kỳ 4ms cho mỗi vòng quét, tần số refresh đạt 250Hz —
> cao hơn nhiều so với ngưỡng 50Hz mà mắt người cảm nhận được flicker.
> Kết quả là màn hình trông sáng và ổn định hoàn toàn."*

**[Chuyển slide]**

---

## SLIDE 10 — Cơ chế cài đặt: Cursor Mode

> *"Đây là cơ chế nhập liệu em thiết kế, gọi là Cursor Mode."*

> *"Khi nhấn SW4 hoặc SW8, một con trỏ ảo xuất hiện tại chữ số đầu tiên —
> chữ số đó nhấp nháy với chu kỳ 500ms ON / 500ms OFF để báo hiệu đang chọn."*

> *"Người dùng có thể nhập theo 2 cách:
> Cách 1 — nhấn phím số 0-9 để gán trực tiếp giá trị vào vị trí con trỏ.
> Cách 2 — nhấn SW15 tăng hoặc SW13 giảm từng bước."*

> *"Con trỏ đi qua 4 vị trí: chục giờ → đơn vị giờ → chục phút → đơn vị phút.
> Sau ô cuối cùng, hệ thống tự thoát."*

> *"Data Boundary Logic đảm bảo giá trị luôn hợp lệ:
> tăng qua 23 giờ sẽ wrap về 00,
> giảm từ 00 sẽ wrap về 23.
> Tương tự với phút, wrap tại 59."*

**[Chuyển slide]**

---

## SLIDE 11 — Non-volatile storage: I2C EEPROM

> *"Hệ thống lưu 3 byte vào EEPROM AT24C02 qua giao tiếp I2C polling."*

> *"Địa chỉ 0x00 lưu giờ báo thức, 0x01 lưu phút báo thức,
> và 0x02 lưu trạng thái bật hay tắt — giá trị 0x01 là bật, 0x00 là tắt."*

> *"Điểm quan trọng là chiến lược ghi thông minh — Smart Write Strategy.
> Hệ thống chỉ ghi EEPROM trong 2 trường hợp:
> Một là khi người dùng xác nhận xong báo thức bằng lần nhấn SW8 cuối cùng.
> Hai là mỗi khi SW12 toggle trạng thái bật tắt."*

> *"Tại sao quan trọng? Vì EEPROM chỉ chịu được khoảng 1 triệu chu kỳ ghi.
> Nếu ghi mỗi giây như đồng hồ thì EEPROM sẽ hỏng chỉ sau 11 ngày.
> Với chiến lược này, EEPROM có thể dùng hàng chục năm."*

**[Chuyển slide]**

---

## SLIDE 12 — Cảnh báo âm thanh: Hardware PWM Buzzer

> *"Buzzer được điều khiển hoàn toàn bằng phần cứng — CT16B0 phát PWM liên tục
> mà không tốn CPU cycle nào."*

> *"Tần số 600Hz được chọn vì nằm trong vùng nhạy cảm nhất của tai người.
> Duty cycle 50% cho âm lượng tối đa.
> Cấu hình: MR9 = 19999 cho chu kỳ, MR0 = 9999 cho duty."*

> *"Điều kiện kích hoạt báo thức: khi g_hour bằng alarm_hour
> và g_min bằng alarm_min — kiểm tra tại thời điểm phút mới bắt đầu."*

> *"Sau khi kích hoạt, buzzer kêu nhịp nhàng — ON 500ms, OFF 500ms —
> trong tối đa 60 giây rồi tự tắt.
> Người dùng có thể tắt ngay lập tức bằng bất kỳ phím nào."*

**[Chuyển slide]**

---

## SLIDE 13 — Lọc tín hiệu: Debounce phím bấm

> *"Đây là vấn đề thực tế mà bất kỳ dự án dùng nút bấm nào cũng phải giải quyết —
> hiện tượng rung cơ học của tiếp điểm."*

> *"Khi nhấn phím, tiếp điểm kim loại không đóng ngay một lần mà nảy qua nảy lại
> trong khoảng 5 đến 20ms, tạo ra nhiều xung giả liên tiếp."*

> *"Giải pháp phần mềm của nhóm: thuật toán quét 2 chiều Row/Column
> kết hợp bộ đếm ổn định 50ms.
> Cụ thể: phím phải được đọc ổn định liên tục trong 50 lần kiểm tra —
> tương đương 50ms vì quét mỗi 1ms — mới được ghi nhận là hợp lệ.
> Sau 200ms giữ phím, phím sẽ bị hủy để tránh nhấn lặp không mong muốn."*

**[Chuyển slide]**

---

## SLIDE 14 — Bảo vệ hệ thống: Watchdog Timer

> *"Watchdog Timer là lớp bảo vệ cuối cùng của hệ thống."*

> *"Nguyên lý hoạt động đơn giản: MCU phải 'feed' — nạp lại — giá trị 0x5AFA5AFA
> vào thanh ghi WDT trước khi timeout.
> Nếu không feed trong 250ms, WDT tự động reset toàn bộ MCU."*

> *"Điều này có nghĩa là: nếu firmware bị treo tại bất kỳ đâu —
> ví dụ I2C bị mất kết nối và loop vô tận, hoặc stack overflow —
> hệ thống sẽ tự khởi động lại thay vì treo mãi mãi."*

> *"Trong code, lệnh __WDT_FEED_VALUE được đặt tại đầu vòng while(1),
> đảm bảo mỗi chu kỳ 1ms đều feed dog.
> WDT được khởi tạo SAU tất cả các thao tác I2C khởi động
> để tránh reset sớm trong quá trình init."*

**[Chuyển slide]**

---

## SLIDE 15 — Kết quả thực nghiệm & kết luận

> *"Về kết quả: tất cả 5 tính năng cốt lõi đã hoạt động đúng trên phần cứng thực."*

> *"Hiển thị 7 đoạn multiplexing 250Hz không flicker.
> Cursor mode và timeout 30 giây hoạt động mượt mà.
> Ghi đọc EEPROM I2C thành công — giờ báo thức giữ nguyên sau khi mất điện.
> Buzzer PWM 600Hz và chuỗi âm báo thức chính xác.
> Hệ thống ổn định nhờ 1ms task scheduling và WDT."*

> *"Điểm em tâm đắc nhất là kiến trúc task-based 1ms —
> nó giải quyết hoàn toàn vấn đề timing mà không cần RTOS,
> phù hợp với vi điều khiển nhỏ như Cortex-M0."*

> *"Nếu phát triển tiếp, em muốn thêm hiển thị giây bằng LED rời,
> và giao tiếp UART để đồng bộ thời gian với máy tính."*

> *"Em xin cảm ơn thầy/cô và các bạn đã lắng nghe.
> Nhóm em xin mời thầy/cô có câu hỏi."*

---

## GHI CHÚ TRÌNH BÀY

| Slide | Thời gian | Điểm nhấn |
|-------|-----------|-----------|
| 1 | 30s | Giới thiệu ngắn, tự tin |
| 2 | 50s | Đọc rõ 5 mục, dừng nhẹ mỗi mục |
| 3 | 50s | Giải thích IHRC và PFPA nếu BTC hỏi |
| 4 | 50s | Chỉ tay vào từng khối trên sơ đồ |
| 5 | 40s | Demo thực tế bàn phím nếu có board |
| 6 | 40s | Nhấn mạnh phân tầng → dễ bảo trì |
| 7 | 60s | Vẽ tay lên bảng nếu BTC chưa rõ |
| 8 | 50s | Đây là kỹ thuật hay nhất — nói chậm |
| 9 | 40s | Giải thích ghosting nếu được hỏi |
| 10 | 50s | Demo cursor mode trực tiếp nếu có board |
| 11 | 50s | Nhấn mạnh wear leveling — điểm kỹ thuật hay |
| 12 | 40s | Giải thích tại sao 600Hz |
| 13 | 40s | Có thể demo bằng cách nhấn phím nhanh |
| 14 | 40s | Giải thích WDT init AFTER I2C |
| 15 | 60s | Tự tin, nhìn BTC, kết bằng mời hỏi |

**Tổng:** ~10–11 phút + Q&A
