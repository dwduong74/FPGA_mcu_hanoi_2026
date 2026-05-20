# KỊCH BẢN QUAY VIDEO DEMO
# Đồng hồ kỹ thuật số SN32F407

**Thời lượng dự kiến:** 3 – 4 phút  
**Cần chuẩn bị trước:**
- Board đã flash firmware, kết nối buzzer + 7-SEG
- Cài sẵn báo thức ở một giờ cụ thể (VD: 10:15)
- Đặt camera góc chụp thấy rõ màn hình 7-SEG và tay nhấn phím
- Đặt micro gần board để thu tiếng buzzer

---

## CẢNH 1 — GIỚI THIỆU (0:00 – 0:20)

**[Camera: quay toàn cảnh board + màn hình]**

> *"Đây là đồng hồ kỹ thuật số được lập trình trên vi điều khiển SN32F407,
> ARM Cortex-M0 chạy ở 12 MHz. Board sử dụng 4 LED 7-đoạn hiển thị giờ và phút,
> bàn phím ma trận 4×4 để cài đặt, EEPROM lưu báo thức và buzzer phát âm thanh."*

---

## CẢNH 2 — KHỞI ĐỘNG VÀ STARTUP ANIMATION (0:20 – 0:40)

**[Camera: zoom vào màn hình 7-SEG, tay cầm dây nguồn]**

> *"Bây giờ tôi sẽ cắm nguồn vào board."*

**[Cắm nguồn — màn hình nhấp nháy giờ báo thức 2 lần rồi về 00:00]**

> *"Khi bật nguồn, board tự động đọc EEPROM và hiển thị giờ báo thức đã lưu
> nhấp nháy 2 lần — ở đây là 10:15 — để người dùng xác nhận cài đặt trước đó.
> Sau đó đồng hồ bắt đầu chạy từ 00:00."*

**[Camera: zoom vào LED D1 đang nhấp nháy]**

> *"LED D1 nhấp nháy đều đặn 1 lần mỗi giây — đây là chỉ thị giây đang chạy
> vì màn hình 7-SEG chỉ hiển thị giờ và phút."*

---

## CẢNH 3 — CÀI ĐẶT GIỜ (0:40 – 1:30)

**[Camera: quay màn hình + tay người dùng, góc đủ thấy cả phím]**

> *"Bây giờ tôi sẽ cài giờ. Nhấn phím SW4 — nút Cài Giờ ở góc trên bên phải bàn phím."*

**[Nhấn SW4 — nghe 1 tiếng bíp — chữ số đầu tiên bắt đầu nhấp nháy]**

> *"Hệ thống vào chế độ cài giờ. Con trỏ đang ở chữ số hàng chục của giờ —
> chữ số này đang nhấp nháy. Tôi nhấn số 1."*

**[Nhấn phím SW1=7... xong nhấn SW9=1]**

> *"Nhấn 1 — màn hình hiển thị 1 ngay lập tức. Tiếp tục nhấn SW4 để sang
> chữ số tiếp theo — đơn vị giờ."*

**[Nhấn SW4, tiếp tục nhập các chữ số: 1, 0, 3, 0 → cài thành 11:30... hoặc 14:25 tùy demo]**

> *"Tôi nhập lần lượt từng chữ số cho giờ và phút. Sau chữ số thứ 4,
> hệ thống tự lưu và về chế độ bình thường. Giây cũng được reset về 0."*

**[Màn hình hiển thị giờ vừa cài, đồng hồ bắt đầu chạy]**

**[Thử nghiệm phím + và −]**

> *"Tôi cũng có thể dùng phím cộng và trừ để tăng giảm từng chữ số tại vị trí con trỏ,
> có wrap-around: qua 23 giờ sẽ về lại 00."*

---

## CẢNH 4 — CÀI BÁO THỨC VÀ LƯU EEPROM (1:30 – 2:10)

**[Camera: tay nhấn SW8]**

> *"Tiếp theo tôi cài báo thức. Nhấn SW8 — nút Cài Báo Thức."*

**[Nhấn SW8 — LED D0 bắt đầu nhấp nháy cùng cursor]**

> *"Chú ý LED D0 cũng nhấp nháy — chỉ thị đang ở chế độ cài báo thức.
> Tôi nhập giờ báo thức."*

**[Nhập 4 chữ số, nhấn SW8 lần cuối]**

> *"Sau khi nhập xong và nhấn SW8 lần nữa, hệ thống lưu giờ báo thức vào EEPROM.
> Tôi sẽ rút điện và cắm lại để kiểm tra."*

**[Rút điện — cắm lại — màn hình nhấp nháy đúng giờ báo thức vừa cài]**

> *"Sau khi mất điện, giờ báo thức vẫn được giữ nguyên trong EEPROM.
> Board hiển thị lại đúng giá trị đó khi khởi động."*

---

## CẢNH 5 — BẬT/TẮT BÁO THỨC (2:10 – 2:35)

**[Camera: tay gần phím SW12]**

> *"SW12 là nút bật tắt báo thức. Nhấn khi báo thức đang bật sẽ nghe 1 tiếng bíp
> và báo thức tắt. Nhấn lại khi báo thức đang tắt sẽ nghe 2 tiếng bíp và báo thức bật."*

**[Nhấn SW12 — nghe 1 bíp]**

> *"Một tiếng bíp — báo thức đã tắt."*

**[Nhấn SW12 lần nữa — nghe 2 bíp]**

> *"Hai tiếng bíp — báo thức đã bật lại. Trạng thái này được lưu vào EEPROM ngay lập tức."*

---

## CẢNH 6 — BÁO THỨC KÊU (2:35 – 3:10)

> *"Tôi sẽ cài báo thức vào giờ gần nhất để demo tiếng báo thức."*

**[Cài báo thức = giờ hiện tại + 1 phút]**

> *"Khi đến giờ đã cài..."*

**[Chờ đến giờ — buzzer bắt đầu kêu ON/OFF]**

**[Camera: zoom vào board nghe tiếng buzzer]**

> *"Buzzer kêu nhịp nhàng ON 500ms OFF 500ms. Báo thức sẽ tự tắt sau 60 giây.
> Hoặc người dùng có thể nhấn bất kỳ phím nào để tắt ngay lập tức."*

**[Nhấn bất kỳ phím — buzzer dừng ngay]**

> *"Nhấn phím — buzzer dừng ngay lập tức."*

---

## CẢNH 7 — TẮT MÀN HÌNH TIẾT KIỆM ĐIỆN (3:10 – 3:30)

**[Camera: tay nhấn SW16]**

> *"Cuối cùng là chức năng tắt màn hình tiết kiệm điện. Nhấn SW16."*

**[Nhấn SW16 — toàn bộ LED tắt]**

> *"Toàn bộ LED 7-SEG, LED D0 và D1 đều tắt. MCU vẫn đang chạy và đếm giờ bên trong.
> Nhấn bất kỳ phím nào để bật lại."*

**[Nhấn bất kỳ phím — màn hình bật lại hiển thị đúng giờ]**

> *"Màn hình bật lại, giờ vẫn chính xác."*

---

## CẢNH 8 — KẾT (3:30 – 3:50)

**[Camera: quay toàn cảnh board đang chạy]**

> *"Tóm lại, đồ án đã thực hiện thành công đồng hồ kỹ thuật số trên SN32F407
> với đầy đủ các tính năng: hiển thị 7-SEG multiplexing, quét phím ma trận debounce,
> giao tiếp I2C EEPROM, điều khiển buzzer PWM và state machine 4 trạng thái.
> Toàn bộ lập trình bằng ngôn ngữ C trên Keil MDK."*

---

## GHI CHÚ QUAY PHIM

| Cảnh | Thời gian | Góc camera | Âm thanh cần nghe |
|------|-----------|------------|-------------------|
| 1 | 20s | Toàn cảnh board | Giọng thuyết minh |
| 2 | 20s | Zoom màn hình | Tiếng buzzer 2 bíp startup |
| 3 | 50s | Màn hình + tay | 1 bíp mỗi phím |
| 4 | 40s | Màn hình + tay | Không đặc biệt |
| 5 | 25s | Tay + loa | 1 bíp / 2 bíp |
| 6 | 35s | Board + loa gần | Buzzer kêu rõ |
| 7 | 20s | Toàn cảnh | 1 bíp wake-up |
| 8 | 20s | Toàn cảnh | Giọng thuyết minh |

**Tips quay:**
- Để board trên nền tối (vải đen) cho LED 7-SEG nổi rõ
- Quay thêm slow-motion cảnh nhấp nháy cursor nếu có thể
- Micro đặt cách buzzer 10~15 cm, không quá gần (vỡ âm)
- Cảnh 6 nên quay 2 lần: 1 lần để tự tắt 60s, 1 lần tắt bằng phím
