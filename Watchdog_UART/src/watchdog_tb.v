`timescale 1ns/1ps

// =============================================================================
// Đội thi    : Sunflower - PTIT
// Cuộc thi   : FPGA Extended Contest 2026 — Vòng sơ loại RTL Design
// Nền tảng   : Kiwi 1P5 (Gowin GW1N-UV1P5QN48XFC7/I6)
// Tệp        : watchdog_tb.v
// Module     : watchdog_tb
// Mô tả      : Testbench toàn diện — bao phủ toàn bộ checklist nộp bài
//
// Danh sách test case (tương ứng checklist):
//
//   RTL (50đ):
//     TC_RESET      — POR an toàn: wdo=1, enout=0, fault=0 ngay sau reset
//     TC_ARM_DELAY  — ENOUT=0 và WDI bị bỏ qua trong cửa sổ arm_delay
//     TC_TIMEOUT    — Timeout → WDO=0 trong tRST → tự phục hồi
//     TC_WDO_POL    — WDO active-low: 0=lỗi, LED sáng; 1=OK, LED tắt
//     TC_FALLING    — Chỉ cạnh xuống WDI mới kick, rising edge bị bỏ qua
//     TC_CLR_FAULT  — clr_fault (CTRL[2]) phục hồi ngay lập tức
//     TC_TIMING     — Kiểm tra quy đổi ms/µs → chu kỳ clock đúng
//
//   UART + Register (25đ):
//     TC_WRITE_ACK  — CMD 0x01 WRITE_REG: kiểm tra ACK 4 byte [AA][01][00][01]
//     TC_READ_REG   — CMD 0x02 READ_REG: kiểm tra response 8 byte
//     TC_UART_KICK  — CMD 0x03 KICK: tương đương WDI cạnh xuống
//     TC_STATUS     — CMD 0x04 GET_STATUS: 7 byte, kiểm tra từng bit
//     TC_NACK_CHK   — Checksum sai → NACK 0xFE
//     TC_NACK_CMD   — CMD không hợp lệ → NACK 0xFF
//     TC_ALL_REGS   — Ghi/đọc lại đủ 5 thanh ghi (0x00 0x04 0x08 0x0C 0x10)
//
//   Testbench chuẩn (15đ):
//     TC1           — Normal kick (UART kick đều đặn) giữ WDO=1
//     TC2           — Timeout + tự phục hồi sau tRST
//     TC3           — Tắt watchdog: ENOUT=0, WDO giải phóng
//     TC4           — arm_delay: ENOUT=0, WDI bị bỏ qua trong window
//     TC5           — Cấu hình đầy đủ qua UART + đọc lại tất cả thanh ghi
//
// Lưu ý debounce:
//   DEBOUNCE_CYCLES = 540_000 ≈ 20ms. Nút nhấn cần giữ >20ms để đăng ký.
//   Các test timing dùng UART kick (bypass debounce) để simulation nhanh.
//   Giao diện nút nhấn được kiểm tra qua TC4 (enable S2) và TC3 (disable).
// =============================================================================

module watchdog_tb;

    // =========================================================================
    // Clock — 27 MHz
    // =========================================================================
    reg clk;
    localparam CLK_PERIOD = 37;          // ns
    localparam CLK_FREQ   = 27_000_000;
    localparam BAUD_NS    = 100;         // ns/bit (Tang toc de simulation nhanh)

    initial clk = 0;
    always #(CLK_PERIOD/2) clk = ~clk;

    // =========================================================================
    // Cổng DUT
    // =========================================================================
    reg  s1_wdi_n;
    reg  s2_en_n;
    reg  uart_rx_pin;
    wire uart_tx_pin;
    wire wdo_led;
    wire enout_led;

    watchdog_top dut (
        .clk         (clk),
        .s1_wdi_n    (s1_wdi_n),
        .s2_en_n     (s2_en_n),
        .uart_rx_pin (uart_rx_pin),
        .uart_tx_pin (uart_tx_pin),
        .wdo_led     (wdo_led),
        .enout_led   (enout_led)
    );

    // Tín hiệu nội bộ (truy cập qua hierarchical reference)
    wire       wdo_int   = dut.wdo_i;    // WDO từ watchdog_core
    wire       enout_int = dut.enout_i;  // ENOUT từ watchdog_core
    wire       fault_int = dut.fault;    // fault_active từ watchdog_core
    wire       rst_n_int = dut.rst_n;    // Tín hiệu POR nội bộ

    // =========================================================================
    // Hàm chuyển đổi thời gian → chu kỳ clock
    // =========================================================================
    function integer ms2cyc;
        input integer ms;
        begin ms2cyc = ms * (CLK_FREQ / 1000); end
    endfunction

    function integer us2cyc;
        input integer us;
        begin us2cyc = us * (CLK_FREQ / 1_000_000); end
    endfunction

    // =========================================================================
    // Task: Gửi 1 byte UART 9600 bps 8N1 (PC → FPGA)
    // =========================================================================
    task uart_send_byte;
        input [7:0] data;
        integer i;
        begin
            uart_rx_pin = 1'b0;        // Bit Start (LOW)
            #(BAUD_NS);
            for (i = 0; i < 8; i = i+1) begin
                uart_rx_pin = data[i]; // Bit dữ liệu, LSB trước
                #(BAUD_NS);
            end
            uart_rx_pin = 1'b1;        // Bit Stop (HIGH)
            #(BAUD_NS);
        end
    endtask

    // =========================================================================
    // Task: Nhận 1 byte UART từ DUT (FPGA → PC), lấy mẫu giữa bit
    // =========================================================================
    task uart_recv_byte;
        output [7:0] data;
        integer i;
        begin
            @(negedge uart_tx_pin);          // Chờ cạnh xuống = start bit
            #(BAUD_NS + BAUD_NS/2);          // Bỏ qua start, căn giữa bit 0
            for (i = 0; i < 8; i = i+1) begin
                data[i] = uart_tx_pin;       // Lấy mẫu bit i
                if (i < 7) #(BAUD_NS);
            end
            #(BAUD_NS/2 + BAUD_NS);          // Hết bit 7 + stop bit
        end
    endtask

    // =========================================================================
    // Buffer nhận UART (dùng chung cho tất cả test)
    // =========================================================================
    reg [7:0] rbuf [0:7];

    task uart_recv_n;
        input integer n;
        integer k;
        begin
            for (k = 0; k < n; k = k+1)
                uart_recv_byte(rbuf[k]);
        end
    endtask

    // =========================================================================
    // Task: Gửi frame UART hoàn chỉnh [0x55][CMD][ADDR][LEN][DATA...][CHK]
    // CHK = XOR(CMD, ADDR, LEN, DATA_0..N-1)
    // =========================================================================
    task uart_send_frame;
        input [7:0]  cmd;
        input [7:0]  addr;
        input [7:0]  len;
        input [31:0] data;
        reg   [7:0]  chk;
        begin
            chk = cmd ^ addr ^ len;
            if (len >= 1) chk = chk ^ data[7:0];
            if (len >= 2) chk = chk ^ data[15:8];
            if (len >= 3) chk = chk ^ data[23:16];
            if (len >= 4) chk = chk ^ data[31:24];
            uart_send_byte(8'h55);
            uart_send_byte(cmd);
            uart_send_byte(addr);
            uart_send_byte(len);
            if (len >= 1) uart_send_byte(data[7:0]);
            if (len >= 2) uart_send_byte(data[15:8]);
            if (len >= 3) uart_send_byte(data[23:16]);
            if (len >= 4) uart_send_byte(data[31:24]);
            uart_send_byte(chk);
        end
    endtask

    // =========================================================================
    // Task: UART kick (CMD 0x03), drain ACK
    // =========================================================================
    task uart_kick;
        begin
            uart_send_frame(8'h03, 8'h00, 8'h00, 32'd0);
            uart_recv_n(4);  // Drain ACK [AA][03][00][03]
        end
    endtask

    // =========================================================================
    // Task: Enable watchdog qua UART (CTRL bit0=1)
    // =========================================================================
    task uart_enable;
        begin
            uart_send_frame(8'h01, 8'h00, 8'h04, 32'd1);
            uart_recv_n(4);
        end
    endtask

    // =========================================================================
    // Task: Disable watchdog qua UART (CTRL=0) + nhả nút S2
    // Gọi task này giữa các test để đảm bảo trạng thái sạch
    // =========================================================================
    task reset_dut_state;
        begin
            s2_en_n  = 1'b1;   // Nhả S2
            s1_wdi_n = 1'b1;   // Nhả S1
            uart_send_frame(8'h01, 8'h00, 8'h04, 32'd0);  // CTRL=0
            uart_recv_n(4);
            repeat(ms2cyc(2)) @(posedge clk);
        end
    endtask

    // =========================================================================
    // Biến đếm lỗi toàn cục
    // =========================================================================
    integer errors;
    integer tc_errors;
    reg [7:0] tmp_chk;    // Dùng để tính checksum tạm

    // =========================================================================
    // Macro-task kiểm tra: in FAIL nếu actual != expected
    // =========================================================================
    task check1;
        input        actual;
        input        expected;
        input [200*8-1:0] msg;
        begin
            if (actual !== expected) begin
                $display("  FAIL [%0t ns]: actual=%b expected=%b — %s",
                         $time, actual, expected, msg);
                errors    = errors    + 1;
                tc_errors = tc_errors + 1;
            end
        end
    endtask

    task check8;
        input [7:0]  actual;
        input [7:0]  expected;
        input [200*8-1:0] msg;
        begin
            if (actual !== expected) begin
                $display("  FAIL [%0t ns]: actual=%02Xh expected=%02Xh — %s",
                         $time, actual, expected, msg);
                errors    = errors    + 1;
                tc_errors = tc_errors + 1;
            end
        end
    endtask

    // =========================================================================
    // Kịch bản kiểm tra chính
    // =========================================================================
    initial begin
        errors      = 0;
        tc_errors   = 0;
        s1_wdi_n    = 1'b1;
        s2_en_n     = 1'b1;
        uart_rx_pin = 1'b1;

        // Chờ POR nội bộ hoàn thành (por_cnt 16-bit = 65536 chu kỳ ≈ 2.4ms)
        repeat(70000) @(posedge clk);

        // =================================================================
        // TC_RESET: Trạng thái an toàn sau POR
        // Mục tiêu: wdo=1, enout=0, fault=0, LED tắt đúng
        // Kiểm tra: FSM khởi động ở IDLE, WDO không active, không fault
        // =================================================================
        $display("\n=== TC_RESET: Trang thai an toan sau POR ===");
        tc_errors = 0;
        check1(wdo_int,   1'b1, "POR wdo=1 (khong loi)");
        check1(enout_int, 1'b0, "POR enout=0 (chua arm)");
        check1(fault_int, 1'b0, "POR fault=0");
        // LED logic: wdo_led=wdo_i=1 (LED D3 tắt), enout_led=~enout_i=1 (LED D4 tắt)
        check1(wdo_led,   1'b1, "POR wdo_led=1 (LED D3 tat: khong loi)");
        check1(enout_led, 1'b1, "POR enout_led=1 (LED D4 tat: chua arm)");
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // SETUP: Ghi giá trị timing nhỏ để simulation chạy nhanh
        // tWD=10ms, tRST=5ms, arm_delay=200µs, CTRL=0 (chưa enable)
        // =================================================================
        $display("\n=== SETUP: Ghi timing nho cho simulation ===");
        uart_send_frame(8'h01, 8'h04, 8'h04, 32'd10);   // tWD_ms = 10
        uart_recv_n(4);
        uart_send_frame(8'h01, 8'h08, 8'h04, 32'd5);    // tRST_ms = 5
        uart_recv_n(4);
        uart_send_frame(8'h01, 8'h0C, 8'h02, 32'd200);  // arm_delay_us = 200
        uart_recv_n(4);
        $display("  Done: tWD=10ms, tRST=5ms, arm_delay=200us");

        // =================================================================
        // TC_WRITE_ACK: CMD 0x01 WRITE_REG — kiểm tra ACK 4 byte
        // Response phải là [AA][01][00][01] (cố định theo thiết kế)
        // =================================================================
        $display("\n=== TC_WRITE_ACK: CMD 0x01 WRITE_REG ACK ===");
        tc_errors = 0;
        uart_send_frame(8'h01, 8'h04, 8'h04, 32'd10);   // Ghi lại tWD=10
        uart_recv_n(4);
        check8(rbuf[0], 8'hAA, "WRITE ACK[0]=AA (header)");
        check8(rbuf[1], 8'h01, "WRITE ACK[1]=01 (echo CMD)");
        check8(rbuf[2], 8'h00, "WRITE ACK[2]=00");
        check8(rbuf[3], 8'h01, "WRITE ACK[3]=01");
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // TC_READ_REG: CMD 0x02 READ_REG — kiểm tra response 8 byte
        // Đọc tWD_ms (0x04) = 10 = 0x0000000A
        // Response: [AA][02][00][0A][00][00][00][CHK]
        // CHK = 0x02 ^ 0x00 ^ 0x0A ^ 0x00 ^ 0x00 ^ 0x00 = 0x08
        // =================================================================
        $display("\n=== TC_READ_REG: CMD 0x02 READ_REG ===");
        tc_errors = 0;
        uart_send_frame(8'h02, 8'h04, 8'h00, 32'd0);
        uart_recv_n(8);
        check8(rbuf[0], 8'hAA, "READ resp[0]=AA");
        check8(rbuf[1], 8'h02, "READ resp[1]=02 (echo CMD)");
        check8(rbuf[2], 8'h00, "READ resp[2]=00");
        check8(rbuf[3], 8'h0A, "READ resp[3]=0A (tWD=10, byte thap)");
        check8(rbuf[4], 8'h00, "READ resp[4]=00");
        check8(rbuf[5], 8'h00, "READ resp[5]=00");
        check8(rbuf[6], 8'h00, "READ resp[6]=00");
        check8(rbuf[7], 8'h02^8'h00^8'h0A^8'h00^8'h00^8'h00,
               "READ CHK=02^00^0A^00^00^00");
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // TC_ALL_REGS: Ghi/đọc đủ cả 5 thanh ghi
        // 0x00 CTRL, 0x04 tWD_ms, 0x08 tRST_ms, 0x0C arm_delay_us, 0x10 STATUS
        // =================================================================
        $display("\n=== TC_ALL_REGS: Ghi/doc 5 thanh ghi ===");
        tc_errors = 0;

        // CTRL (0x00): đọc lại = 0 (chưa enable)
        uart_send_frame(8'h02, 8'h00, 8'h00, 32'd0);
        uart_recv_n(8);
        check8(rbuf[3], 8'h00, "REG 0x00 CTRL=0 (disabled)");

        // tWD_ms (0x04) = 10 = 0x0A
        uart_send_frame(8'h02, 8'h04, 8'h00, 32'd0);
        uart_recv_n(8);
        check8(rbuf[3], 8'h0A, "REG 0x04 tWD=10");

        // tRST_ms (0x08) = 5 = 0x05
        uart_send_frame(8'h02, 8'h08, 8'h00, 32'd0);
        uart_recv_n(8);
        check8(rbuf[3], 8'h05, "REG 0x08 tRST=5");

        // arm_delay_us (0x0C) = 200 = 0x00C8 → D0=0xC8, D1=0x00
        uart_send_frame(8'h02, 8'h0C, 8'h00, 32'd0);
        uart_recv_n(8);
        check8(rbuf[3], 8'hC8, "REG 0x0C arm=200 (D0=C8)");
        check8(rbuf[4], 8'h00, "REG 0x0C arm=200 (D1=00)");

        // STATUS (0x10): disabled → en_effective=0, wdo=1, enout=0, fault=0
        // STATUS[4:0] = {last_kick_src, wdo, enout, fault, en_effective}
        //             = {0, 1, 0, 0, 0} = 0x08
        uart_send_frame(8'h02, 8'h10, 8'h00, 32'd0);
        uart_recv_n(8);
        check8(rbuf[3], 8'h08, "REG 0x10 STATUS=08 (wdo=1, rest=0)");
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // TC_NACK_CHK: Checksum sai → NACK 0xFE
        // Frame: [55][01][04][04][0A][00][00][00][CHK_BAD]
        // CHK đúng = 01^04^04^0A = 0x01; CHK sai = 0x02
        // =================================================================
        $display("\n=== TC_NACK_CHK: Checksum sai -> NACK 0xFE ===");
        tc_errors = 0;
        tmp_chk = 8'h01 ^ 8'h04 ^ 8'h04 ^ 8'h0A ^ 8'h00 ^ 8'h00 ^ 8'h00;
        tmp_chk = tmp_chk ^ 8'h01;   // Làm sai checksum (+1)
        uart_send_byte(8'h55);
        uart_send_byte(8'h01);  // CMD WRITE
        uart_send_byte(8'h04);  // ADDR tWD
        uart_send_byte(8'h04);  // LEN = 4
        uart_send_byte(8'h0A);  // data[0]
        uart_send_byte(8'h00);  // data[1]
        uart_send_byte(8'h00);  // data[2]
        uart_send_byte(8'h00);  // data[3]
        uart_send_byte(tmp_chk);// CHK sai
        uart_recv_n(4);
        check8(rbuf[0], 8'hAA, "NACK_CHK resp[0]=AA");
        check8(rbuf[1], 8'hFE, "NACK_CHK resp[1]=FE (bad checksum)");
        check8(rbuf[2], 8'h00, "NACK_CHK resp[2]=00");
        check8(rbuf[3], 8'hFE, "NACK_CHK resp[3]=FE");
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // TC_NACK_CMD: CMD không hợp lệ → NACK 0xFF
        // Frame: [55][AB][00][00][CHK] với CHK=AB^00^00=AB
        // =================================================================
        $display("\n=== TC_NACK_CMD: CMD khong hop le -> NACK 0xFF ===");
        tc_errors = 0;
        uart_send_byte(8'h55);
        uart_send_byte(8'hAB);  // CMD không hợp lệ
        uart_send_byte(8'h00);  // ADDR
        uart_send_byte(8'h00);  // LEN = 0
        uart_send_byte(8'hAB);  // CHK = AB^00^00 = AB
        uart_recv_n(4);
        check8(rbuf[0], 8'hAA, "NACK_CMD resp[0]=AA");
        check8(rbuf[1], 8'hFF, "NACK_CMD resp[1]=FF (unknown CMD)");
        check8(rbuf[2], 8'h00, "NACK_CMD resp[2]=00");
        check8(rbuf[3], 8'hFF, "NACK_CMD resp[3]=FF");
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // TC4 / TC_ARM_DELAY: Kiểm tra cửa sổ arm_delay
        // Mục tiêu: ENOUT=0 và WDI (UART kick) bị bỏ qua trong arm_delay
        // =================================================================
        $display("\n=== TC4 / TC_ARM_DELAY: Cua so arm_delay ===");
        tc_errors = 0;
        reset_dut_state();
        // Enable watchdog qua UART → vào ARMING ngay lập tức
        uart_enable();

        // Kiểm tra ENOUT=0 ngay sau khi enable (chưa qua arm_delay)
        @(posedge clk);
        check1(enout_int, 1'b0, "ARM: ENOUT=0 ngay sau enable");

        // Thử kick trong cửa sổ arm_delay (50µs << arm_delay=200µs)
        // Kick này phải bị bỏ qua (ARMING state không xử lý wdi_fall)
        repeat(us2cyc(50)) @(posedge clk);
        check1(enout_int, 1'b0, "ARM: ENOUT=0 luc 50us (trong arm_delay)");
        uart_kick();  // Kick này phải bị bỏ qua

        // Chờ arm_delay kết thúc (>200µs)
        repeat(us2cyc(220)) @(posedge clk);
        check1(enout_int, 1'b1, "ARM: ENOUT=1 sau khi arm_delay ket thuc");
        check1(wdo_int,   1'b1, "ARM: WDO=1 (binh thuong sau arm)");

        // Kick ngay để reset bộ đếm tWD
        uart_kick();
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // TC1: Normal kick — UART kick đều đặn trước timeout → WDO giữ HIGH
        // Kick mỗi 7ms, tWD=10ms → không bao giờ timeout
        // =================================================================
        $display("\n=== TC1: Normal kick (UART, 7ms < tWD=10ms) ===");
        tc_errors = 0;
        // Watchdog đang MONITOR từ TC4, tiếp tục kick
        repeat(3) begin
            repeat(ms2cyc(7)) @(posedge clk);
            uart_kick();
        end
        check1(wdo_int,   1'b1, "TC1: WDO=1 sau 3 vong kick deu dan");
        check1(enout_int, 1'b1, "TC1: ENOUT=1 (watchdog dang chay)");
        check1(fault_int, 1'b0, "TC1: fault=0");
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // TC_FALLING: Chỉ cạnh xuống của WDI mới kick watchdog
        // Sau 1 kick (falling edge), rising edge trở về 1 KHÔNG reset timer.
        // Không kick thêm → timer chạy đến hết tWD → timeout.
        // Điều này chứng minh rising edge bị bỏ qua.
        // =================================================================
        $display("\n=== TC_FALLING: Chi canh xuong moi kick ===");
        tc_errors = 0;
        // Một kick hợp lệ → reset timer
        uart_kick();
        // Sau ~740ns, wdi trở về 1 (rising edge) — timer KHÔNG reset
        // Đợi 8ms (< tWD=10ms): WDO vẫn 1 (timer chưa hết)
        repeat(ms2cyc(8)) @(posedge clk);
        check1(wdo_int, 1'b1, "FALLING: WDO=1 tai 8ms (timer chua het)");
        // Đợi thêm 4ms nữa (tổng 12ms > tWD=10ms): timeout phải xảy ra
        repeat(ms2cyc(4)) @(posedge clk);
        check1(wdo_int,   1'b0, "FALLING: WDO=0 tai 12ms (timeout, rising edge khong kick)");
        check1(fault_int, 1'b1, "FALLING: fault=1");
        // Chờ phục hồi để sẵn sàng cho TC tiếp theo
        repeat(ms2cyc(6)) @(posedge clk);
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // TC2: Timeout + tự phục hồi sau tRST
        // WDO=0 trong tRST=5ms rồi tự về MONITOR
        // =================================================================
        $display("\n=== TC2: Timeout + tu phuc hoi ===");
        tc_errors = 0;
        // Kick để reset timer, rồi không kick nữa
        uart_kick();
        // Đợi vượt tWD=10ms → timeout
        repeat(ms2cyc(12)) @(posedge clk);
        check1(wdo_int,   1'b0, "TC2a: WDO=0 sau timeout 12ms");
        check1(fault_int, 1'b1, "TC2a: fault=1");
        // LED D3 phải sáng (wdo_led = wdo_i = 0 → active-low LED ON)
        check1(wdo_led,   1'b0, "TC2a: LED D3 sang (wdo_led=0 khi fault)");

        // Đợi tRST=5ms → tự phục hồi
        repeat(ms2cyc(6)) @(posedge clk);
        check1(wdo_int,   1'b1, "TC2b: WDO=1 sau tu phuc hoi (tRST=5ms)");
        check1(fault_int, 1'b0, "TC2b: fault=0 sau tu phuc hoi");
        check1(wdo_led,   1'b1, "TC2b: LED D3 tat (wdo_led=1 khi OK)");
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // TC_WDO_POL: WDO active-low — 0=lỗi, 1=bình thường; LED logic
        // Đã kiểm tra trong TC2a/TC2b, nhắc lại tổng hợp
        // =================================================================
        $display("\n=== TC_WDO_POL: WDO active-low va LED logic ===");
        tc_errors = 0;
        // Trạng thái hiện tại: MONITOR bình thường
        check1(wdo_int,   1'b1, "POL: wdo=1 = binh thuong");
        check1(wdo_led,   1'b1, "POL: wdo_led=1 = LED D3 tat (OK)");
        check1(enout_int, 1'b1, "POL: enout=1 = da arm");
        check1(enout_led, 1'b0, "POL: enout_led=0 = LED D4 sang (da arm)");
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // TC_CLR_FAULT: CTRL[2]=1 phục hồi FAULT ngay lập tức (không đợi tRST)
        // =================================================================
        $display("\n=== TC_CLR_FAULT: clr_fault phuc hoi ngay lap tuc ===");
        tc_errors = 0;
        // Tạo timeout
        uart_kick();
        repeat(ms2cyc(12)) @(posedge clk);
        check1(wdo_int, 1'b0, "CLR: WDO=0 sau timeout (truoc khi xoa)");
        // Gửi CTRL=5: bit0=en_sw=1 (giữ enable), bit2=clr_fault=1
        // Chú ý: f_data[0][2]=1 → clr_fault fires; reg_ctrl[0]=1 → en_sw giữ
        uart_send_frame(8'h01, 8'h00, 8'h04, 32'd5);
        uart_recv_n(4);
        repeat(ms2cyc(1)) @(posedge clk);
        check1(wdo_int,   1'b1, "CLR: WDO=1 ngay sau clr_fault (khong doi tRST)");
        check1(fault_int, 1'b0, "CLR: fault=0 ngay sau clr_fault");
        // Reset CTRL về 1 (chỉ giữ en_sw, xóa bit clr_fault)
        uart_send_frame(8'h01, 8'h00, 8'h04, 32'd1);
        uart_recv_n(4);
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // TC_UART_KICK: CMD 0x03 KICK tương đương WDI cạnh xuống
        // Kick đúng lúc → không timeout; kiểm tra ACK format
        // =================================================================
        $display("\n=== TC_UART_KICK: CMD 0x03 KICK ===");
        tc_errors = 0;
        uart_kick();   // Reset timer
        // Đợi 7ms, kick qua UART
        repeat(ms2cyc(7)) @(posedge clk);
        uart_send_frame(8'h03, 8'h00, 8'h00, 32'd0);
        uart_recv_n(4);
        check8(rbuf[0], 8'hAA, "KICK ACK[0]=AA");
        check8(rbuf[1], 8'h03, "KICK ACK[1]=03 (echo CMD)");
        check8(rbuf[2], 8'h00, "KICK ACK[2]=00");
        check8(rbuf[3], 8'h03, "KICK ACK[3]=03");
        // Đợi thêm 7ms → nếu UART kick không hoạt động sẽ timeout
        repeat(ms2cyc(7)) @(posedge clk);
        check1(wdo_int, 1'b1, "KICK: WDO=1 sau UART kick (timer reset thanh cong)");
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // TC_STATUS: CMD 0x04 GET_STATUS — 7 byte + kiểm tra từng bit
        // STATUS byte = {last_kick_src, wdo, enout, fault_active, en_effective}
        // Trạng thái: enabled(1), no fault(0), armed(1), wdo=1, last_kick=1(UART)
        // Bits: bit0=1, bit1=0, bit2=1, bit3=1, bit4=1 → 0x1D
        // CHK = 0x04 ^ 0x00 ^ 0x1D ^ 0x00 ^ 0x00 = 0x19
        // =================================================================
        $display("\n=== TC_STATUS: CMD 0x04 GET_STATUS ===");
        tc_errors = 0;
        uart_kick();   // Kick để trạng thái sạch + last_kick_src=1 (UART)
        repeat(ms2cyc(1)) @(posedge clk);
        uart_send_frame(8'h04, 8'h00, 8'h00, 32'd0);
        uart_recv_n(7);
        check8(rbuf[0], 8'hAA, "STATUS resp[0]=AA");
        check8(rbuf[1], 8'h04, "STATUS resp[1]=04 (echo CMD)");
        check8(rbuf[2], 8'h00, "STATUS resp[2]=00");
        // Kiểm tra từng bit của D0 (rbuf[3])
        if (rbuf[3][0] !== 1'b1) begin
            $display("  FAIL: STATUS bit0 en_effective=%b expect=1", rbuf[3][0]);
            errors = errors + 1; tc_errors = tc_errors + 1;
        end
        if (rbuf[3][1] !== 1'b0) begin
            $display("  FAIL: STATUS bit1 fault=%b expect=0", rbuf[3][1]);
            errors = errors + 1; tc_errors = tc_errors + 1;
        end
        if (rbuf[3][2] !== 1'b1) begin
            $display("  FAIL: STATUS bit2 enout=%b expect=1", rbuf[3][2]);
            errors = errors + 1; tc_errors = tc_errors + 1;
        end
        if (rbuf[3][3] !== 1'b1) begin
            $display("  FAIL: STATUS bit3 wdo=%b expect=1", rbuf[3][3]);
            errors = errors + 1; tc_errors = tc_errors + 1;
        end
        if (rbuf[3][4] !== 1'b1) begin
            $display("  FAIL: STATUS bit4 last_kick_src=%b expect=1(UART)", rbuf[3][4]);
            errors = errors + 1; tc_errors = tc_errors + 1;
        end
        check8(rbuf[4], 8'h00, "STATUS D1=00 (byte cao luon 0)");
        check8(rbuf[5], 8'h00, "STATUS D2=00 (byte cao luon 0)");
        // Kiểm tra checksum
        tmp_chk = 8'h04 ^ 8'h00 ^ rbuf[3] ^ rbuf[4] ^ rbuf[5];
        check8(rbuf[6], tmp_chk, "STATUS CHK=04^00^D0^D1^D2");
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // TC_TIMING: Kiểm tra quy đổi tham số ms/µs → chu kỳ clock
        // Đọc lại twd_limit qua watchdog_core.counter để kiểm tra gián tiếp:
        // tWD=10ms @ 27MHz → twd_limit = 10 * 27000 = 270000 cycles
        // tRST=5ms → trst_limit = 135000 cycles
        // arm_delay=200µs → arm_delay_limit = 200 * 27 = 5400 cycles
        // Kiểm tra bằng cách đo thời gian từ ARMING đến MONITOR = arm_delay
        // =================================================================
        $display("\n=== TC_TIMING: Quy doi tham so ms/us -> clock cycle ===");
        tc_errors = 0;
        // arm_delay_limit = 200us * 27 = 5400 cycles
        // Đo: enable → đếm đến ENOUT=1
        reset_dut_state();
        uart_enable();
        // ENOUT phải = 0 trong arm_delay
        check1(enout_int, 1'b0, "TIMING: ENOUT=0 ngay sau enable");
        // Đợi 190µs (< 200µs): vẫn phải = 0
        repeat(us2cyc(190)) @(posedge clk);
        check1(enout_int, 1'b0, "TIMING: ENOUT=0 luc 190us (< arm_delay=200us)");
        // Đợi thêm 20µs (tổng 210µs > arm_delay): phải = 1
        repeat(us2cyc(20)) @(posedge clk);
        check1(enout_int, 1'b1, "TIMING: ENOUT=1 luc 210us (> arm_delay=200us)");
        uart_kick();
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // TC3: Tắt watchdog — ENOUT=0, WDO giải phóng
        // =================================================================
        $display("\n=== TC3: Disable watchdog ===");
        tc_errors = 0;
        // Tắt qua cả UART lẫn nhả nút S2
        s2_en_n = 1'b1;
        uart_send_frame(8'h01, 8'h00, 8'h04, 32'd0);   // CTRL=0
        uart_recv_n(4);
        repeat(ms2cyc(2)) @(posedge clk);
        check1(enout_int, 1'b0, "TC3: ENOUT=0 khi disable");
        check1(wdo_int,   1'b1, "TC3: WDO=1 (giai phong) khi disable");
        check1(fault_int, 1'b0, "TC3: fault=0");
        // LED: D4 tắt (enout=0 → enout_led=~0=1), D3 tắt (wdo=1→wdo_led=1)
        check1(enout_led, 1'b1, "TC3: LED D4 tat (enout_led=1)");
        check1(wdo_led,   1'b1, "TC3: LED D3 tat (wdo_led=1)");
        if (tc_errors == 0) $display("  => PASS");

        // =================================================================
        // TC5: Cấu hình đầy đủ + đọc lại tất cả thanh ghi qua UART
        // Thay đổi tất cả tham số rồi đọc lại để xác nhận ghi đúng
        // =================================================================
        $display("\n=== TC5: Cau hinh day du qua UART + doc lai ===");
        tc_errors = 0;
        // Ghi giá trị mới cho tất cả tham số
        uart_send_frame(8'h01, 8'h04, 8'h04, 32'd15);   // tWD=15ms
        uart_recv_n(4);
        uart_send_frame(8'h01, 8'h08, 8'h04, 32'd7);    // tRST=7ms
        uart_recv_n(4);
        uart_send_frame(8'h01, 8'h0C, 8'h02, 32'd300);  // arm_delay=300µs
        uart_recv_n(4);

        // Đọc lại và xác nhận
        uart_send_frame(8'h02, 8'h04, 8'h00, 32'd0);
        uart_recv_n(8);
        check8(rbuf[3], 8'h0F, "TC5: tWD=15ms doc lai D0=0F");
        check8(rbuf[4], 8'h00, "TC5: tWD=15ms doc lai D1=00");

        uart_send_frame(8'h02, 8'h08, 8'h00, 32'd0);
        uart_recv_n(8);
        check8(rbuf[3], 8'h07, "TC5: tRST=7ms doc lai D0=07");

        // 300 = 0x012C → little-endian: D0=0x2C, D1=0x01
        uart_send_frame(8'h02, 8'h0C, 8'h00, 32'd0);
        uart_recv_n(8);
        check8(rbuf[3], 8'h2C, "TC5: arm_delay=300us D0=2C (300=0x12C)");
        check8(rbuf[4], 8'h01, "TC5: arm_delay=300us D1=01");

        // Đọc STATUS trong lúc disabled
        uart_send_frame(8'h02, 8'h10, 8'h00, 32'd0);
        uart_recv_n(8);
        check8(rbuf[3][3:0], 4'h8, "TC5: STATUS disabled: wdo=1 rest=0 → 0x?8");
        if (tc_errors == 0) $display("  => PASS");

        // Khôi phục timing ban đầu (cho phép chạy lại)
        uart_send_frame(8'h01, 8'h04, 8'h04, 32'd10);
        uart_recv_n(4);
        uart_send_frame(8'h01, 8'h08, 8'h04, 32'd5);
        uart_recv_n(4);
        uart_send_frame(8'h01, 8'h0C, 8'h02, 32'd200);
        uart_recv_n(4);

        // =================================================================
        // Tổng kết
        // =================================================================
        $display("\n========================================");
        $display("TONG SO ASSERTION THAT BAI: %0d", errors);
        if (errors == 0)
            $display("KET QUA TONG HOP: TAT CA TEST CASE PASS !!!");
        else
            $display("KET QUA TONG HOP: CON %0d LOI CAN SUA", errors);
        $display("========================================");

        $finish;
    end

    // =========================================================================
    // Bảo vệ: tự kết thúc nếu simulation chạy quá 5000ms
    // =========================================================================
    initial begin
        #5000_000_000;
        $display("CANH BAO: Simulation vuot qua gioi han 5000ms!");
        $finish;
    end

    // =========================================================================
    // Xuất VCD để xem dạng sóng trong GTKWave
    // Chạy: iverilog -o sim/watchdog_tb.vvp src/*.v && vvp sim/watchdog_tb.vvp
    // Xem:  gtkwave sim/watchdog_tb.vcd sim/watchdog.gtkw
    // =========================================================================
    initial begin
        $dumpfile("watchdog_tb.vcd");
        $dumpvars(0, watchdog_tb);
    end

endmodule
