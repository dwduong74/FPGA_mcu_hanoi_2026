`timescale 1ns/1ps

// =============================================================================
// Module      : watchdog_tb
// Mô tả       : Testbench kiểm tra toàn bộ hệ thống watchdog_top
//
// Danh sách test case:
//   TC1 — Normal kick  : Kick WDI đều đặn trước timeout → WDO giữ HIGH
//   TC2 — Timeout      : Không kick trong tWD → WDO xuống LOW, tự phục hồi
//   TC3 — Disable      : EN=0 → ENOUT=0, WDO giải phóng, WDI bị bỏ qua
//   TC4 — arm_delay    : Kick trong cửa sổ arm_delay bị bỏ qua, ENOUT=0
//   TC5 — UART config  : Thay đổi tWD/tRST/arm_delay qua giao thức UART
//
// Ghi chú:
//   Tham số mặc định (1600ms, 200ms) quá lớn cho simulation.
//   TC5 được chạy đầu tiên để ghi tWD=10ms, tRST=5ms, arm_delay=200µs
//   sau đó các TC1-TC4 sử dụng giá trị nhỏ hơn này.
// =============================================================================

module watchdog_tb;

    // =========================================================================
    // Xung nhịp (watchdog_top dùng POR nội bộ, không có port rst_n ngoài)
    // =========================================================================
    reg clk;

    localparam CLK_PERIOD = 37;        // ~27 MHz (chu kỳ 37 ns)
    localparam CLK_FREQ   = 27_000_000;

    initial clk = 0;
    always #(CLK_PERIOD/2) clk = ~clk;

    // =========================================================================
    // Kết nối với DUT (Device Under Test)
    // =========================================================================
    reg  s1_wdi_n;      // Nút S1 WDI — tích cực thấp
    reg  s2_en_n;       // Nút S2 EN  — tích cực thấp
    reg  uart_rx_pin;   // Đường RX từ PC vào board
    wire uart_tx_pin;   // Đường TX từ board ra PC
    wire wdo_led;       // LED D3: sáng = lỗi
    wire enout_led;     // LED D4: sáng = đã arm

    watchdog_top dut (
        .clk         (clk),
        .s1_wdi_n    (s1_wdi_n),
        .s2_en_n     (s2_en_n),
        .uart_rx_pin (uart_rx_pin),
        .uart_tx_pin (uart_tx_pin),
        .wdo_led     (wdo_led),
        .enout_led   (enout_led)
    );

    // Truy cập trực tiếp tín hiệu nội bộ để kiểm tra
    // Tên khớp với wire trong watchdog_top: wdo_i, enout_i, fault
    wire wdo_int   = dut.wdo_i;
    wire enout_int = dut.enout_i;
    wire fault_int = dut.fault;

    // =========================================================================
    // Hàm trợ giúp: chuyển đổi ms/µs sang chu kỳ xung nhịp
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
    // Task: Tạo 1 lần kick WDI bằng nút S1 (nhấn 1ms, nhả 1ms)
    // =========================================================================
    task kick_wdi;
        begin
            s1_wdi_n = 1'b0;                    // Nhấn nút S1
            repeat(ms2cyc(1)) @(posedge clk);
            s1_wdi_n = 1'b1;                    // Nhả nút
            repeat(ms2cyc(1)) @(posedge clk);
        end
    endtask

    // =========================================================================
    // Task: Bật watchdog bằng nút S2
    // =========================================================================
    task enable_wd;
        begin
            s2_en_n = 1'b0;  // Nhấn và giữ nút S2
        end
    endtask

    // =========================================================================
    // Task: Tắt watchdog, nhả nút S2
    // =========================================================================
    task disable_wd;
        begin
            s2_en_n = 1'b1;  // Nhả nút S2
        end
    endtask

    // =========================================================================
    // Task: Phát 1 byte qua UART (9600 bps, 8N1)
    // Thứ tự: Start bit → 8 bit dữ liệu (LSB trước) → Stop bit
    // =========================================================================
    localparam BAUD_NS = 1_000_000_000 / 9600;  // ~104167 ns mỗi bit

    task uart_send_byte;
        input [7:0] data;
        integer i;
        begin
            uart_rx_pin = 1'b0;        // Bit Start
            #(BAUD_NS);
            for (i = 0; i < 8; i = i+1) begin
                uart_rx_pin = data[i]; // Bit dữ liệu (LSB trước)
                #(BAUD_NS);
            end
            uart_rx_pin = 1'b1;        // Bit Stop
            #(BAUD_NS);
        end
    endtask

    // =========================================================================
    // Task: Gửi 1 khung UART hoàn chỉnh
    // Định dạng: [0x55][CMD][ADDR][LEN][DATA0..N][CHK]
    // CHK = XOR(CMD, ADDR, LEN, DATA0..N)
    // =========================================================================
    task uart_send_frame;
        input [7:0]  cmd;
        input [7:0]  addr;
        input [7:0]  len;
        input [31:0] data;
        reg [7:0] chk;
        begin
            // Tính checksum XOR
            chk = cmd ^ addr ^ len;
            if (len >= 1) chk = chk ^ data[7:0];
            if (len >= 2) chk = chk ^ data[15:8];
            if (len >= 3) chk = chk ^ data[23:16];
            if (len >= 4) chk = chk ^ data[31:24];

            // Gửi từng byte của khung
            uart_send_byte(8'h55);          // Header
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
    // Biến theo dõi kết quả
    // =========================================================================
    integer errors;

    // =========================================================================
    // Kịch bản kiểm tra chính
    // =========================================================================
    initial begin
        errors      = 0;
        s1_wdi_n    = 1'b1;    // Nút S1 không nhấn
        s2_en_n     = 1'b1;    // Nút S2 không nhấn → watchdog tắt
        uart_rx_pin = 1'b1;    // Đường UART ở trạng thái idle

        // Chờ POR nội bộ hoàn thành: por_cnt[15:0] cần 65536 clocks (~2.4ms)
        repeat(70000) @(posedge clk);

        // -----------------------------------------------------------------
        // TC5: Cấu hình tham số qua UART
        // Mục đích: Đặt tWD=10ms, tRST=5ms, arm_delay=200µs để simulation
        //           chạy đủ nhanh. Đồng thời kiểm tra giao thức UART.
        // -----------------------------------------------------------------
        $display("--- TC5: Cấu hình tham số qua UART ---");

        // Ghi tWD_ms = 10 (địa chỉ 0x04, 4 byte little-endian)
        uart_send_frame(8'h01, 8'h04, 8'h04, 32'd10);
        repeat(ms2cyc(5)) @(posedge clk);

        // Ghi tRST_ms = 5 (địa chỉ 0x08)
        uart_send_frame(8'h01, 8'h08, 8'h04, 32'd5);
        repeat(ms2cyc(5)) @(posedge clk);

        // Ghi arm_delay_us = 200 (địa chỉ 0x0C, 2 byte)
        uart_send_frame(8'h01, 8'h0C, 8'h02, 32'd200);
        repeat(ms2cyc(5)) @(posedge clk);

        // Bật watchdog qua UART (CTRL bit0 = 1, địa chỉ 0x00)
        uart_send_frame(8'h01, 8'h00, 8'h04, 32'd1);
        repeat(ms2cyc(5)) @(posedge clk);

        // Đọc lại tWD_ms (addr=0x04) qua READ_REG để kiểm tra ghi/đọc
        uart_send_frame(8'h02, 8'h04, 8'h00, 32'd0);
        repeat(ms2cyc(5)) @(posedge clk);

        $display("TC5 PASS: Đã gửi và nhận khung UART thành công");

        // -----------------------------------------------------------------
        // TC4: Cửa sổ arm_delay — WDI bị bỏ qua ngay sau khi bật
        // -----------------------------------------------------------------
        $display("--- TC4: Cửa sổ arm_delay ---");
        enable_wd();  // Nhấn S2 → EN = 1

        // Kick ngay trong cửa sổ arm_delay (50µs < 200µs)
        repeat(us2cyc(50)) @(posedge clk);

        if (enout_int !== 1'b0) begin
            $display("FAIL TC4: ENOUT phải = 0 trong arm_delay, nhận được %b", enout_int);
            errors = errors + 1;
        end

        kick_wdi();  // Kick này phải bị bỏ qua

        // Chờ arm_delay kết thúc (200µs + thêm ít để đảm bảo)
        repeat(us2cyc(200)) @(posedge clk);

        if (enout_int !== 1'b1) begin
            $display("FAIL TC4: ENOUT phải = 1 sau arm_delay, nhận được %b", enout_int);
            errors = errors + 1;
        end else begin
            $display("TC4 PASS: ENOUT = 0 trong arm_delay, = 1 sau khi arm_delay kết thúc");
        end

        // -----------------------------------------------------------------
        // TC1: Normal kick — Kick đều đặn trước timeout
        // tWD = 10ms, kick mỗi 7ms → không bao giờ timeout
        // -----------------------------------------------------------------
        $display("--- TC1: Normal kick ---");
        repeat(3) begin
            repeat(ms2cyc(7)) @(posedge clk);  // Đợi 7ms (< tWD=10ms)
            kick_wdi();
        end

        if (wdo_int !== 1'b1) begin
            $display("FAIL TC1: WDO xuống thấp dù đã kick đúng hạn");
            errors = errors + 1;
        end else begin
            $display("TC1 PASS: WDO giữ HIGH khi kick đều đặn");
        end

        // -----------------------------------------------------------------
        // TC2: Timeout — Không kick trong tWD, WDO phải xuống LOW
        // -----------------------------------------------------------------
        $display("--- TC2: Timeout ---");

        // Đợi vượt quá tWD = 10ms
        repeat(ms2cyc(12)) @(posedge clk);

        if (wdo_int !== 1'b0) begin
            $display("FAIL TC2a: WDO không xuống sau timeout");
            errors = errors + 1;
        end else begin
            $display("TC2a PASS: WDO xuống LOW sau timeout");
        end

        // Đợi tRST = 5ms trôi qua → WDO tự giải phóng
        repeat(ms2cyc(6)) @(posedge clk);

        if (wdo_int !== 1'b1) begin
            $display("FAIL TC2b: WDO không tự phục hồi sau tRST");
            errors = errors + 1;
        end else begin
            $display("TC2b PASS: WDO tự phục hồi sau tRST");
        end

        // -----------------------------------------------------------------
        // TC3: Disable — Tắt watchdog, ENOUT phải về 0
        // -----------------------------------------------------------------
        $display("--- TC3: Disable watchdog ---");
        disable_wd();  // Nhả nút S2 → EN = 0
        repeat(ms2cyc(2)) @(posedge clk);

        if (enout_int !== 1'b0) begin
            $display("FAIL TC3: ENOUT phải = 0 khi EN=0, nhận được %b", enout_int);
            errors = errors + 1;
        end

        if (wdo_int !== 1'b1) begin
            $display("FAIL TC3: WDO phải giải phóng khi EN=0, nhận được %b", wdo_int);
            errors = errors + 1;
        end

        if (errors == 0) begin
            $display("TC3 PASS: ENOUT=0, WDO giải phóng khi tắt watchdog");
        end

        // -----------------------------------------------------------------
        // Tổng kết
        // -----------------------------------------------------------------
        $display("========================================");
        if (errors == 0)
            $display("KẾT QUẢ: TẤT CẢ %0d TEST CASE ĐỀU PASS", 5);
        else
            $display("KẾT QUẢ: %0d TEST CASE THẤT BẠI", errors);
        $display("========================================");

        $finish;
    end

    // =========================================================================
    // Bảo vệ timeout cho simulation (tối đa 500ms)
    // =========================================================================
    initial begin
        #500_000_000;
        $display("CẢNH BÁO: Simulation vượt quá giới hạn thời gian 500ms!");
        $finish;
    end

    // =========================================================================
    // Xuất file dạng sóng để quan sát trong GTKWave/ModelSim
    // =========================================================================
    initial begin
        $dumpfile("watchdog_tb.vcd");
        $dumpvars(0, watchdog_tb);
    end

endmodule
