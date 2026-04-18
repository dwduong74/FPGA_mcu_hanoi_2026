// =============================================================================
// Đội thi    : Sunflower - PTIT
// Cuộc thi   : FPGA Extended Contest 2026 — Vòng sơ loại
// Nền tảng   : Kiwi 1P5 (Gowin GW1N-UV1P5QN48XFC7/I6)
// Tệp        : watchdog_top.v
// Module     : watchdog_top
// Mô tả      : Module đỉnh (top-level) kết nối toàn bộ hệ thống.
//              Bao gồm POR nội bộ, debounce nút nhấn, UART, thanh ghi cấu
//              hình và lõi watchdog. Không có chân reset ngoài — dùng POR.
//
// Sơ đồ kết nối:
//   Button S1 (s1_wdi_n) ──► sync_debounce ──► wdi_db ──► watchdog_core.wdi
//   Button S2 (s2_en_n)  ──► sync_debounce ──► en_btn ──► watchdog_core.en
//   uart_rx_pin ──► uart_rx ──► rx_data/rx_done ──► config_reg
//   config_reg  ──► tx_data/tx_en ──► uart_tx ──► uart_tx_pin
//   config_reg  ──► uart_kick ──► u_wdi pulse ──► watchdog_core.wdi
//   watchdog_core ──► wdo_i / enout_i ──► LED D3 / LED D4
//
// Mapping chân (Kiwi 1P5 — theo schematic trang 3):
//   Pin  4 (IOL6A)  = clk         27 MHz oscillator
//   Pin 33 (IOR11B) = uart_rx_pin USB-UART chip TX → FPGA nhận
//   Pin 34 (IOR11A) = uart_tx_pin FPGA gửi → USB-UART chip RX
//   Pin 35 (IOR1B)  = s1_wdi_n   Nút S1, active-low
//   Pin 36 (IOR1A)  = s2_en_n    Nút S2, active-low
//   Pin 27 (IOR17A) = wdo_led    LED D3, sáng khi wdo=0 (lỗi)
//   Pin 28 (IOR15B) = enout_led  LED D4, sáng khi enout=1 (đã arm)
// =============================================================================

module watchdog_top (
    input  wire clk,          // Xung nhịp hệ thống 27 MHz (Pin 4)
    input  wire s1_wdi_n,     // Nút S1 — WDI kick, active-low (Pin 35)
    input  wire s2_en_n,      // Nút S2 — Enable watchdog, active-low (Pin 36)
    input  wire uart_rx_pin,  // UART RX — nhận dữ liệu từ PC (Pin 33)
    output wire uart_tx_pin,  // UART TX — gửi dữ liệu về PC (Pin 34)
    output wire wdo_led,      // LED D3 — báo lỗi WDO, active-low (Pin 27)
    output wire enout_led     // LED D4 — báo đã arm, active-low (Pin 28)
);

    // =========================================================================
    // Power-On Reset (POR) nội bộ
    // -------------------------------------------------------------------------
    // por_cnt đếm từ 0 đến 0xFFFF (65536 chu kỳ ≈ 2.43ms @ 27MHz).
    // rst_n = AND của tất cả 16 bit → chỉ lên HIGH khi por_cnt = 0xFFFF.
    // Dùng literal 16'd1 để tránh cảnh báo truncation size-17.
    // =========================================================================
    reg [15:0] por_cnt = 16'd0;
    wire rst_n = &por_cnt;
    always @(posedge clk)
        if (!rst_n) por_cnt <= por_cnt + 16'd1;

    // =========================================================================
    // Debounce nút nhấn (sync_debounce × 2)
    // -------------------------------------------------------------------------
    // Mỗi nút được xử lý bởi 1 instance sync_debounce:
    //   - 2-FF synchronizer: chống metastability
    //   - Counter 540_000 cy ≈ 20ms: lọc rung cơ học
    // Đầu ra btn_out là active-HIGH:
    //   wdi_db = 1 khi S1 đang nhấn (dùng để cập nhật last_kick_src)
    //   en_btn = 1 khi S2 đang nhấn (dùng để enable watchdog)
    // =========================================================================
    wire wdi_db, en_btn;
    sync_debounce #(.DEBOUNCE_CYCLES(540_000)) u_db1 (
        .clk    (clk),
        .rst_n  (rst_n),
        .btn_n  (s1_wdi_n),  // Đầu vào active-low từ S1
        .btn_out(wdi_db)     // Đầu ra active-high: 1 = đang nhấn S1
    );
    sync_debounce #(.DEBOUNCE_CYCLES(540_000)) u_db2 (
        .clk    (clk),
        .rst_n  (rst_n),
        .btn_n  (s2_en_n),   // Đầu vào active-low từ S2
        .btn_out(en_btn)     // Đầu ra active-high: 1 = đang nhấn S2
    );

    // =========================================================================
    // Bộ thu phát UART 9600 bps 8N1
    // -------------------------------------------------------------------------
    // uart_rx: phát hiện cạnh xuống bit Start → lấy mẫu 8 bit → rx_done=1
    // uart_tx: nhận tx_en=1 → phát Start + 8 bit + Stop → tx_busy trong quá trình
    // tx_done_unused: port tx_done không được dùng, kết nối để tránh warning
    // =========================================================================
    wire [7:0] rx_data;   // Byte dữ liệu vừa nhận được từ PC
    wire       rx_done;   // Xung 1 chu kỳ: nhận xong 1 byte
    wire [7:0] tx_data;   // Byte dữ liệu cần phát về PC
    wire       tx_en;     // Xung 1 chu kỳ: kích hoạt phát 1 byte
    wire       tx_busy;   // HIGH trong suốt quá trình phát
    wire       tx_done_unused;  // Không dùng, chỉ kết nối để không float

    uart_rx #(.BAUD_RATE(9600)) u_rx (
        .clk    (clk),
        .rst_n  (rst_n),
        .rx     (uart_rx_pin),
        .rx_data(rx_data),
        .rx_done(rx_done)
    );
    uart_tx #(.BAUD_RATE(9600)) u_tx (
        .clk    (clk),
        .rst_n  (rst_n),
        .tx_data(tx_data),
        .tx_en  (tx_en),
        .tx     (uart_tx_pin),
        .tx_busy(tx_busy),
        .tx_done(tx_done_unused)
    );

    // =========================================================================
    // Thanh ghi cấu hình + Parser giao thức UART (config_reg)
    // -------------------------------------------------------------------------
    // Nhận frame [0x55][CMD][ADDR][LEN][DATA...][CHK] từ uart_rx,
    // giải mã lệnh, cập nhật thanh ghi, và gửi response qua uart_tx.
    // Cũng tính toán các giới hạn thời gian (ms/µs → số chu kỳ clock).
    // =========================================================================
    wire [31:0] twd;      // Giới hạn timeout tWD (đơn vị: chu kỳ clock)
    wire [31:0] trst;     // Giới hạn tRST (đơn vị: chu kỳ clock)
    wire [31:0] arm;      // Giới hạn arm_delay (đơn vị: chu kỳ clock)
    wire        en_sw;    // Bit enable từ thanh ghi CTRL[0] (qua UART)
    wire        clr_f;    // Xung xóa lỗi (write-1-to-clear CTRL[2])
    wire        u_kick;   // Xung UART kick (CMD 0x03)
    wire        fault;    // Trạng thái lỗi từ watchdog_core
    wire        enout_i;  // Trạng thái ENOUT từ watchdog_core
    wire        wdo_i;    // Trạng thái WDO từ watchdog_core

    // =========================================================================
    // Sticky bit: lưu nguồn kick gần nhất
    // -------------------------------------------------------------------------
    // 0 = nút S1 (hardware), 1 = lệnh UART (software)
    // Cập nhật ngay khi có kick mới, giữ nguyên cho đến kick tiếp theo.
    // =========================================================================
    reg last_kick_src_r;
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n)
            last_kick_src_r <= 1'b0;        // Mặc định: nguồn là button
        else if (u_kick)
            last_kick_src_r <= 1'b1;        // Vừa có kick từ UART
        else if (wdi_db)
            last_kick_src_r <= 1'b0;        // Vừa có kick từ nút S1
    end

    config_reg u_cfg (
        .clk            (clk),
        .rst_n          (rst_n),
        // UART data path
        .rx_data        (rx_data),
        .rx_done        (rx_done),
        .tx_data        (tx_data),
        .tx_en          (tx_en),
        .tx_busy        (tx_busy),
        // Giới hạn thời gian tính theo số chu kỳ
        .twd_limit      (twd),
        .trst_limit     (trst),
        .arm_delay_limit(arm),
        // Điều khiển và trạng thái
        .en_sw          (en_sw),
        .clr_fault      (clr_f),
        .uart_kick      (u_kick),
        .fault_active   (fault),
        .enout          (enout_i),
        .wdo            (wdo_i),
        .en_effective   (en_sw | en_btn),   // Enable tổng: UART hoặc nút S2
        .last_kick_src  (last_kick_src_r)
    );

    // =========================================================================
    // Tạo xung WDI giả cho UART KICK
    // -------------------------------------------------------------------------
    // Khi uart_kick=1: u_wdi kéo xuống 0 và giữ trong 20 chu kỳ, sau đó
    // nhả lên 1. watchdog_core nhìn thấy cạnh xuống (1→0) → đây là kick
    // hợp lệ. Cần 20 chu kỳ để đảm bảo qua được bộ đồng bộ 3-FF bên trong
    // watchdog_core. Dùng literal 5'd để tránh cảnh báo truncation size-6.
    // =========================================================================
    reg       u_wdi;          // Tín hiệu WDI phần mềm (từ UART kick)
    reg [4:0] k_cnt;          // Đếm 20 chu kỳ giữ mức thấp
    always @(posedge clk) begin
        if (u_kick) begin
            u_wdi <= 1'b0;    // Bắt đầu xung kick: kéo xuống 0
            k_cnt <= 5'd1;
        end else if (k_cnt != 5'd0) begin
            if (k_cnt == 5'd20) begin
                u_wdi <= 1'b1;  // Đã đủ 20 chu kỳ → nhả lên 1
                k_cnt <= 5'd0;
            end else begin
                k_cnt <= k_cnt + 5'd1;
            end
        end else begin
            u_wdi <= 1'b1;    // Idle: giữ mức 1 (không kick)
        end
    end

    // =========================================================================
    // Lõi watchdog (watchdog_core)
    // -------------------------------------------------------------------------
    // Tín hiệu WDI tổng hợp = (~wdi_db) & u_wdi:
    //   ~wdi_db: đảo debounce output → 1 khi S1 KHÔNG nhấn, 0 khi đang nhấn
    //   u_wdi:   0 trong 20 cy khi UART kick, 1 còn lại
    // Kết hợp AND: wdi = 0 khi có kick (từ nút HOẶC UART) → tạo cạnh xuống
    // =========================================================================
    watchdog_core u_core (
        .clk            (clk),
        .rst_n          (rst_n),
        .en             (en_sw | en_btn),       // Enable: UART hoặc nút S2
        .wdi            ((~wdi_db) & u_wdi),    // WDI tổng hợp từ 2 nguồn
        .clr_fault      (clr_f),
        .wdo            (wdo_i),
        .enout          (enout_i),
        .twd_limit      (twd),
        .trst_limit     (trst),
        .arm_delay_limit(arm),
        .fault_active   (fault)
    );

    // =========================================================================
    // Gán LED đầu ra
    // -------------------------------------------------------------------------
    // LED trên Kiwi 1P5 là active-LOW (LED sáng khi pin = 0):
    //   wdo_led   = wdo_i    → wdo=0 (FAULT) → pin=0 → LED D3 SÁNG
    //                        → wdo=1 (OK)    → pin=1 → LED D3 TẮT
    //   enout_led = ~enout_i → enout=1 (ARM) → pin=0 → LED D4 SÁNG
    //                        → enout=0 (OFF) → pin=1 → LED D4 TẮT
    // =========================================================================
    assign wdo_led   = wdo_i;    // LED D3: báo trạng thái WDO
    assign enout_led = ~enout_i; // LED D4: báo đã arm (đảo vì active-low)

endmodule
