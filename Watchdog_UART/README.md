# Watchdog Monitor (TPS3431-compatible) – Kiwi 1P5 FPGA
**FPGA Extended Contest 2026 – Vòng sơ loại RTL Design**

---

## Mục lục
1. [Tổng quan](#1-tổng-quan)
2. [Kiến trúc module](#2-kiến-trúc-module)
3. [FSM watchdog_core](#3-fsm-watchdog_core)
4. [Flow hoạt động đầy đủ](#4-flow-hoạt-động-đầy-đủ)
5. [UART Protocol & Register Map](#5-uart-protocol--register-map)
6. [Mapping chân (Kiwi 1P5)](#6-mapping-chân-kiwi-1p5)
7. [Build & Flash](#7-build--flash)
8. [Testbench](#8-testbench)
9. [Checklist nộp bài](#9-checklist-nộp-bài)
10. [Kịch bản quay video test](#10-kịch-bản-quay-video-test)
11. [Ôn tập phỏng vấn](#11-ôn-tập-phỏng-vấn)

---

## 1. Tổng quan

Dự án mô phỏng nguyên lý hoạt động của IC watchdog **TPS3431 (Texas Instruments)** trên FPGA GW1N-UV1P5 (board Kiwi 1P5), với khả năng cấu hình tham số trong lúc chạy qua giao tiếp UART.

### Tính năng đã implement

| Tính năng | Trạng thái |
|-----------|-----------|
| Phát hiện cạnh xuống WDI (falling edge, 3-FF sync) | OK |
| FSM 4 trạng thái: IDLE / ARMING / MONITOR / FAULT | OK |
| arm_delay: bỏ qua WDI sau khi enable | OK |
| WDO active-low, tự phục hồi sau tRST | OK |
| ENOUT output | OK |
| Debounce 20ms cho button S1, S2 | OK |
| POR nội bộ (không cần chân reset ngoài) | OK |
| UART 9600 bps 8N1 | OK |
| WRITE_REG (0x01), READ_REG (0x02), KICK (0x03), GET_STATUS (0x04) | OK |
| NACK khi checksum sai (0xFE) hoặc lệnh lạ (0xFF) | OK |
| UART KICK tương đương WDI cạnh xuống | OK |
| last_kick_src sticky bit (button / UART) | OK |
| Testbench 5 test case | OK |
| GUI Python (test_watchdog.py) | OK |

---

## 2. Kiến trúc module

```
watchdog_top
├── sync_debounce x 2      (debounce S1 và S2)
├── uart_rx                (bộ nhận UART 8N1)
├── uart_tx                (bộ phát UART 8N1)
├── config_reg             (parser frame + regfile + TX sequencer)
│   ├── rd_mux (comb)      (mux đọc thanh ghi cho READ_REG)
│   └── TX sequencer       (gửi response byte-by-byte)
└── watchdog_core          (FSM watchdog chính)
    └── wdi_fall (comb)    (phát hiện cạnh xuống)
```

### Mô tả ngắn từng module

| Module | Vai trò |
|--------|---------|
| `watchdog_top` | Top module, kết nối tất cả, POR nội bộ, sticky last_kick_src |
| `sync_debounce` | 2-FF synchronizer + counter debounce 20ms, output active-HIGH |
| `uart_rx` | FSM nhận UART: IDLE→START→DATA×8→STOP, lấy mẫu tại BIT_COUNT-1 |
| `uart_tx` | FSM phát UART: IDLE→START→DATA×8→STOP |
| `config_reg` | Parser frame UART + thanh ghi cấu hình + TX sequencer |
| `watchdog_core` | FSM IDLE→ARMING→MONITOR→FAULT, phát hiện cạnh xuống WDI |

---

## 3. FSM watchdog_core

```
                    rst_n deassert
                         |
                         v
              +----------------------+
              |         IDLE         |<---- en=0 (tu bat ky state)
              |  wdo=1  enout=0      |
              |  fault_active=0      |
              +----------+-----------+
                         | en=1
                         v
              +----------------------+
              |       ARMING         |
              |  dem arm_delay       |----> en=0 --> IDLE
              |  enout=0             |
              |  WDI bi bo qua       |
              +----------+-----------+
                         | counter >= arm_delay_limit
                         | (enout <- 1)
                         v
              +----------------------+    counter >= twd_limit
              |       MONITOR        |------------------------------->+
              |  enout=1             |                                |
              |  dem tWD             |<-----------------------+       |
              |  wdi_fall -> reset   |  counter>=trst OR      |       |
              +----------------------+  clr_fault             |       |
                                                              |       v
                                              +----------------------+
                                              |        FAULT         |
                                              |  wdo=0  enout=1      |
                                              |  fault_active=1      |
                                              |  dem tRST            |
                                              +----------------------+
                                                       | en=0
                                                       v
                                                      IDLE
```

### Bảng chuyển trạng thái

| Tu | Den | Dieu kien |
|----|-----|-----------|
| IDLE | ARMING | `en=1` |
| ARMING | IDLE | `en=0` |
| ARMING | MONITOR | `counter >= arm_delay_limit` |
| MONITOR | IDLE | `en=0` |
| MONITOR | FAULT | `counter >= twd_limit` (timeout) |
| MONITOR | MONITOR | `wdi_fall=1` (reset counter, o lai) |
| FAULT | IDLE | `en=0` |
| FAULT | MONITOR | `counter >= trst_limit` hoac `clr_fault=1` |

---

## 4. Flow hoạt động đầy đủ

### Scenario 1: Kick binh thuong bang nut S1

```
t=0           t=arm_delay      t=kick1    t=kick2    t=kick3
IDLE -> ARMING -> MONITOR ----> MONITOR -> MONITOR -> MONITOR
                 enout=0  enout=1
                          wdo=1  wdo=1     wdo=1      wdo=1
                          LED D4 sang (ARMED)
                                 (counter reset moi lan kick)
```

### Scenario 2: Timeout va tu phuc hoi

```
t=0        t=arm_delay    t=tWD          t=tWD+tRST
MONITOR -> MONITOR ------> FAULT -------> MONITOR
           (khong kick)    wdo=0          wdo=1
                           LED D3 sang    (chu ky moi)
```

### Scenario 3: UART KICK flow

```
PC                  config_reg           watchdog_top      watchdog_core
|                       |                    |                   |
|--[55 03 00 00 03]---> |                    |                   |
|                       |--uart_kick=1 (1cy)>|                   |
|                       |                    |--u_wdi=0 (20cy)-->|
|                       |                    |                   |wdi_fall=1
|                       |                    |                   |counter<-0
|<--[AA 03 00 03]-------|                    |                   |
```

### Flow UART Frame parser trong config_reg

```
Byte den:   [0x55]  [CMD]   [ADDR]  [LEN]    [DATA x LEN]    [CHK]
State:       PS_HDR PS_CMD  PS_ADDR PS_LEN    PS_DATA (loop)  PS_CHK
                                      |                         |
                                 LEN=0 -> skip DATA       CHK match?
                                                         YES -> execute CMD
                                                         NO  -> NACK 0xFE
```

---

## 5. UART Protocol & Register Map

### Frame format

```
Gui di (PC -> FPGA):
[0x55] [CMD] [ADDR] [LEN] [DATA_0] ... [DATA_N-1] [CHK]
CHK = CMD ^ ADDR ^ LEN ^ DATA_0 ^ ... ^ DATA_N-1

Nhan ve (FPGA -> PC):
[0xAA] [CMD_ECHO] [STATUS_BYTE] [DATA...] [CHK]
CHK = XOR tat ca byte tu CMD_ECHO den DATA cuoi
```

### Bang lenh

| CMD | Ten | LEN gui | Response | Mo ta |
|-----|-----|---------|----------|-------|
| `0x01` | WRITE_REG | 2 hoac 4 byte | 4 byte `[AA 01 00 01]` | Ghi thanh ghi |
| `0x02` | READ_REG | 0 byte | 8 byte `[AA 02 00 D0 D1 D2 D3 CHK]` | Doc thanh ghi |
| `0x03` | KICK | 0 byte | 4 byte `[AA 03 00 03]` | Software kick |
| `0x04` | GET_STATUS | 0 byte | 7 byte `[AA 04 00 D0 D1 D2 CHK]` | Doc STATUS nhanh |
| `0xFE` | NACK checksum | — | 4 byte (FPGA gui) | CHK sai |
| `0xFF` | NACK unknown | — | 4 byte (FPGA gui) | CMD la |

### Register Map

| Dia chi | Ten | R/W | Default | Mo ta |
|---------|-----|-----|---------|-------|
| `0x00` | CTRL | R/W | `0x00` | bit0=EN_SW, bit2=CLR_FAULT (write-1-clear) |
| `0x04` | tWD_ms | R/W | `1600` | Timeout watchdog (ms) |
| `0x08` | tRST_ms | R/W | `200` | Thoi gian giu WDO khi loi (ms) |
| `0x0C` | arm_delay_us | R/W | `150` | Delay bo qua WDI sau enable (us) |
| `0x10` | STATUS | R only | realtime | Trang thai he thong |

### STATUS bits (0x10 / CMD 0x04 D0 byte)

```
Bit 4: last_kick_src   -- 0=button S1, 1=UART CMD
Bit 3: wdo             -- 1=OK, 0=FAULT (active-low)
Bit 2: enout           -- 1=da arm va dang chay
Bit 1: fault_active    -- 1=dang o trang thai loi
Bit 0: en_effective    -- 1=watchdog dang enable
```

### Vi du frame thuc te

```
# Ghi tWD = 3000ms (0x0BB8)
PC gui:   55 01 04 04 B8 0B 00 00 B3
FPGA tra: AA 01 00 01

# Doc lai tWD (addr=0x04)
PC gui:   55 02 04 00 06
FPGA tra: AA 02 00 B8 0B 00 00 B1

# Kich phan mem
PC gui:   55 03 00 00 03
FPGA tra: AA 03 00 03

# Doc STATUS
PC gui:   55 04 00 00 04
FPGA tra: AA 04 00 1D 00 00 19  (vi du: STATUS=0x1D = enout|wdo|fault|en)
```

---

## 6. Mapping chan (Kiwi 1P5)

| Signal | FPGA Pin | Phan cung | Ghi chu |
|--------|----------|-----------|---------|
| `clk` | 4 (IOL6A) | Oscillator 27 MHz | |
| `uart_rx_pin` | 33 (IOR11B) | USB-UART chip TX | FPGA nhan tu PC |
| `uart_tx_pin` | 34 (IOR11A) | USB-UART chip RX | FPGA gui ve PC |
| `s1_wdi_n` | 35 (IOR1B) | Button S1 | Active-low, debounce 20ms |
| `s2_en_n` | 36 (IOR1A) | Button S2 | Active-low, debounce 20ms |
| `wdo_led` | 27 (IOR17A) | LED D3 | Sang khi wdo=0 (FAULT) |
| `enout_led` | 28 (IOR15B) | LED D4 | Sang khi enout=1 (ARMED) |

**VCCIO:** 3.3V, IO_TYPE=LVCMOS33 (tat ca bank I/O)

**Open-drain:** Du an dung push-pull output voi logic active-low (Cach B theo de bai).
WDO = 0 khi loi (keo xuong), WDO = 1 khi binh thuong (day len). Khong can pull-up ngoai.

---

## 7. Build & Flash

### Build qua CLI

```bash
cd C:/Gowin
"Gowin_V1.9.11.03_Education_x64/IDE/bin/gw_sh.exe" Watchdog_UART/build.tcl
```

Bitstream output: `impl/pnr/watchdog_uart.fs`

### Flash bang Gowin Programmer

1. Mo Gowin Programmer
2. Chon `watchdog_uart.fs`
3. Device: GW1N-UV1P5QN48XFC7/I6
4. Mode: SRAM (test nhanh) hoac Flash (luu vao bo nho)

### Chay GUI test

```bash
pip install pyserial
python C:/Gowin/test_watchdog.py
```

Chon COM port dung, baudrate **9600**.

---

## 8. Testbench

### Chay simulation (Icarus Verilog)

```bash
iverilog -o watchdog_tb.vvp \
  src/watchdog_tb.v src/watchdog_top.v src/watchdog_core.v \
  src/config_reg.v src/sync_debounce.v src/uart_rx.v src/uart_tx.v
vvp watchdog_tb.vvp
gtkwave watchdog_tb.vcd
```

### 5 Test case

| TC | Ten | Kiem tra |
|----|-----|----------|
| TC5 | UART Config | Ghi tWD/tRST/arm_delay, doc lai tWD, bat WD qua UART |
| TC4 | arm_delay | ENOUT=0 trong arm_delay, ENOUT=1 sau khi ket thuc |
| TC1 | Normal kick | Kick 7ms < tWD=10ms, WDO luon HIGH |
| TC2 | Timeout | Khong kick 12ms > tWD=10ms, WDO xuong LOW roi tu phuc hoi |
| TC3 | Disable | EN=0 -> ENOUT=0, WDO giai phong |

---

## 9. Checklist nộp bài

### RTL (50 diem)

- [x] Phat hien **canh xuong** WDI (3-FF synchronizer + falling edge detect)
- [x] FSM du 4 state: IDLE / ARMING / MONITOR / FAULT
- [x] Reset mac dinh **an toan**: watchdog tat, wdo=1
- [x] **arm_delay** hoat dong: ENOUT=0 va WDI bi bo qua trong window
- [x] Timeout -> WDO=0 trong tRST -> **tu phuc hoi** (khong dung luon)
- [x] **WDO active-low**: 0=loi, 1=binh thuong
- [x] Push-pull output (don gian hoa open-drain) ghi ro trong README
- [x] Chuyen doi tham so ms/us -> **clock cycle** dung

### UART + Register (25 diem)

- [x] Frame: `[0x55][CMD][ADDR][LEN][DATA...][CHK]`
- [x] CMD 0x01 WRITE_REG voi ACK 4 byte
- [x] **CMD 0x02 READ_REG** voi response 8 byte
- [x] CMD 0x03 KICK (tuong duong WDI canh xuong)
- [x] CMD 0x04 GET_STATUS voi response 7 byte
- [x] NACK checksum sai (0xFE) va CMD la (0xFF)
- [x] Thanh ghi 0x00, 0x04, 0x08, 0x0C, 0x10 day du

### Testbench (15 diem)

- [x] TC1: Normal kick
- [x] TC2: Timeout + tu phuc hoi
- [x] TC3: Disable
- [x] TC4: arm_delay
- [x] TC5: Cau hinh + doc lai qua UART

### Code + Tai lieu (10 diem)

- [x] README day du (file nay)
- [x] FSM diagram
- [x] Pin mapping dung theo schematic board
- [x] Constraints file (.cst) + timing file (.sdc)
- [x] Code co comment tieng Viet ro rang

---

## 10. Kịch bản quay video test

> Muc tieu: **5-7 phut**

### Buoc 0 – Khoi dong (30 giay)
- Flash `watchdog_uart.fs` vao board
- Mo `test_watchdog.py`, chon COM port, chon baudrate 9600, bam KET NOI
- Noi: *"Board sau reset: watchdog tat (IDLE), WDO=1 (OK), ENOUT=0 (chua arm)"*
- Xac nhan tren GUI: tat ca den trang thai hien thi "---"

### Buoc 1 – Cau hinh qua UART (1 phut)
1. Ghi **tWD = 3000ms** → bam Ghi → GUI log hien thi `[ACK OK]`
2. Ghi **tRST = 500ms** → bam Ghi → log `[ACK OK]`
3. Mo serial terminal, gui `55 02 04 00 06` (READ tWD) → nhan `AA 02 00 B8 0B 00 00 B1`
4. Noi: *"UART hai chieu hoat dong – ghi va doc lai dung gia tri"*

### Buoc 2 – Enable va arm_delay (45 giay)
1. Bam **BAT WATCHDOG** → log `[ACK OK]`
2. LED D4 bat sang gần như ngay (arm_delay=150us qua nhanh)
3. GUI: ENOUT = "SANG (ARMED)", EN_EFFECTIVE = "DANG CHAY"
4. Noi: *"arm_delay 150us bao ve he thong khong bi timeout ngay sau enable"*

### Buoc 3 – Normal kick bang UART (45 giay)
1. Bam **KICK (Software)** moi 2 giay, trong 6 giay
2. GUI: WDO hien thi **OK (HIGH)**, LED D3 tat
3. Noi: *"Kick deu dan < tWD=3s → khong timeout"*
4. Kiem tra: `Nguon Kick cuoi: Lenh UART (Software)`

### Buoc 4 – Trigger Timeout (1 phut)
1. **DUNG kick** hoan toan
2. Dem nguoc 3 giay
3. **LED D3 BAT** (WDO=0) – GUI: `LOI (LOW)`, `PHAT HIEN LOI`
4. Doi 500ms
5. **LED D3 TAT** – GUI: `OK (HIGH)` tu dong
6. Noi: *"Sau tRST=500ms, watchdog tu reset va bat dau chu ky moi"*

### Buoc 5 – Kick bang nut S1 (30 giay)
1. Bam nut **S1** tren board moi 2 giay
2. WDO giu HIGH
3. GUI: `Nguon Kick cuoi: Nut nhan S1 (Hardware)`
4. Noi: *"Sticky bit phan biet nguon kick: hardware vs software"*

### Buoc 6 – CLR_FAULT va Disable (30 giay)
1. De timeout xay ra → LED D3 bat
2. Bam **XOA LOI (CLR)** → LED D3 tat ngay, khong can doi tRST
3. Bam **TAT WATCHDOG** → LED D4 tat, WD ve IDLE
4. Noi: *"CLR_FAULT = write-1-to-clear vao CTRL[2], phuc hoi ngay lap tuc"*

### Buoc 7 – Ket qua testbench (30 giay)
Hien thi terminal voi output:
```
TC5 PASS: Da gui va nhan khung UART thanh cong
TC4 PASS: ENOUT = 0 trong arm_delay, = 1 sau khi arm_delay ket thuc
TC1 PASS: WDO giu HIGH khi kick deu dan
TC2a PASS: WDO xuong LOW sau timeout
TC2b PASS: WDO tu phuc hoi sau tRST
TC3 PASS: ENOUT=0, WDO giai phong khi tat watchdog
KET QUA: TAT CA 5 TEST CASE DEU PASS
```

---

## 11. Ôn tập phỏng vấn

### A. Watchdog Core

**Q: Tai sao phai dung 3 flip-flop (wdi_s0, s1, s2) de phat hien canh xuong?**
> Dung 2 FF cho dong bo hoa (chong metastability). FF thu 3 de phat hien canh:
> `wdi_fall = wdi_s2 & ~wdi_s1`. Neu chi dung 2 FF, tin hieu con co the metastable o wdi_s1.

**Q: Tai sao can arm_delay?**
> Khi EN chuyen 0→1, he thong can thoi gian khoi dong. Neu khong co arm_delay, khong co kick ngay sau enable → timeout ngay → WDO xuong → sai. arm_delay cho phep he thong "on dinh" truoc khi bat dau dem tWD.

**Q: Tai sao sau FAULT khong ve ARMING ma ve MONITOR?**
> arm_delay chi can thiet lan dau enable (EN: 0→1). Sau fault, EN van =1, he thong da on dinh. Ve MONITOR truc tiep giam thoi gian phuc hoi va dung dung nguyen ly TPS3431.

**Q: Cach mo phong open-drain trong FPGA?**
> Cach don gian (dung trong project): push-pull output voi logic active-low.
> Cach chinh xac: dung cong `inout`, drive LOW khi fault, tha (hi-Z) khi OK, cong voi pull-up ngoai. De thi dung Cach B la du diem.

**Q: Neu tWD_ms = 10000, phep nhan `reg_twd_ms * (CLK_FREQ/1000)` co overflow 32-bit?**
> 10000 * 27000 = 270,000,000 — can 29 bit, OK voi 32-bit. Nguong tran: 2^32/27000 ≈ 158,000ms (~158s). Neu gia tri lon hon can dung logic 64-bit.

---

### B. UART

**Q: 9600 baud, 8N1 nghia la gi?**
> 9600 bit/giay. 8N1: 8 data bit, No parity, 1 stop bit. Moi byte can 10 bit (1 start + 8 data + 1 stop) → 10/9600 ≈ 1.04ms/byte.

**Q: Tai sao uart_rx lay mau o BIT_COUNT-1 thay vi BIT_COUNT?**
> uart_tx phat bit tu chu ky 0 den BIT_COUNT-1 (so sanh `timer == BIT_COUNT-1`). Uart_rx cung dung BIT_COUNT-1 → diem lay mau nam dung giua bit, dong bo voi TX, giam sai lech.

**Q: Tai sao can HALF_BIT_COUNT trong S_START cua uart_rx?**
> Khi phat hien canh xuong (bit start), RX dang o dau bit start. Doi HALF_BIT_COUNT (~0.52ms) de dat diem do vao GIUA bit start. Tu do moi bit tiep theo chi can doi BIT_COUNT.

**Q: XOR checksum co nhuoc diem gi?**
> Neu 2 bit loi o vi tri giong nhau trong cac byte khac nhau, XOR tu triet tieu → khong phat hien duoc. XOR chi bat so le bit loi. Ung dung quan trong can CRC-8 hoac CRC-16.

---

### C. FPGA / Verilog

**Q: Khac biet giua blocking (=) va non-blocking (<=)?**
> `=` (blocking): thuc thi tuan tu, dung trong `always @(*)` (combinatorial).
> `<=` (non-blocking): tat ca RHS duoc tinh cung luc, cap nhat sau, dung trong `always @(posedge clk)` (sequential). Dung sai gay race condition trong simulation.

**Q: Tai sao POR can 65536 clock cycles (2.4ms)?**
> `por_cnt` la register 16 bit, khoi tao = 0. `rst_n = &por_cnt` = HIGH khi tat ca 16 bit = 1. Can 65536 clock cycles tang dan tu 0 den 0xFFFF. Tai 27MHz: 65536/27M ≈ 2.43ms.

**Q: Tai sao dung 2-FF synchronizer truoc debounce?**
> Tin hieu nut nhan la async (khong dong bo voi clk). Neu dua thang vao logic clocked, co the gay metastability → output khong xac dinh. 2-FF synchronizer giam xac suat metastability xuong duoi nguong chap nhan (~10^-18 failure/cycle).

**Q: LVCMOS33 la gi? Sai voltage co anh huong gi?**
> LVCMOS33: I/O standard 3.3V. VIH ≥ 2.0V, VOH ≥ 2.4V. Neu dung 1.8V voi USB-UART 3.3V → UART TX chi reach 1.8V, khong du nguong VIH=2.0V cua USB bridge → PC khong nhan duoc gi.

**Q: Tai sao can file .sdc?**
> Khai bao clock de P&R tool biet timing requirement. Khong co .sdc → tool khong biet frequency muc tieu → dat logic khong toi uu → timing violation → mach hoat dong sai tren board du simulation dung.

**Q: wire vs reg trong Verilog?**
> `wire`: ket noi, khong co trang thai, driven boi `assign` hoac output module.
> `reg`: giu gia tri, dung trong `always` block. Trong synthesis: `reg` trong `always @(posedge clk)` → flip-flop; trong `always @(*)` → combinatorial logic (latch neu viet sai).

---

### D. Bảng timing tham khảo

| Tham so | Gia tri default | Y nghia |
|---------|----------------|---------|
| CLK | 27 MHz | 37ns/cycle |
| BIT_COUNT (9600 baud) | 2812 cycles | ~104us/bit |
| HALF_BIT_COUNT | 1406 cycles | ~52us |
| arm_delay | 150us = 4050 cycles | Bo qua WDI sau enable |
| tWD | 1600ms = 43,200,000 cycles | Timeout watchdog |
| tRST | 200ms = 5,400,000 cycles | Giu WDO thap |
| Debounce | 20ms = 540,000 cycles | Chong rung nut |
| POR | 65536 cycles = 2.43ms | Power-on reset noi bo |

---

*Du an: Watchdog Monitor FPGA – FPGA Extended Contest 2026*
*Board: Kiwi 1P5 (GW1N-UV1P5QN48XFC7/I6) | Tool: Gowin IDE V1.9.11.03 Education*
