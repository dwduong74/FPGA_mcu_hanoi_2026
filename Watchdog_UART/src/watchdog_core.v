// =============================================================================
// Đội thi    : Sunflower - PTIT
// Cuộc thi   : FPGA Extended Contest 2026 — Vòng sơ loại
// Nền tảng   : Kiwi 1P5 (Gowin GW1N-UV1P5QN48XFC7/I6)
// Tệp        : watchdog_core.v
// Module     : watchdog_core
// Mô tả      : Lõi watchdog mô phỏng IC TPS3431 (Texas Instruments).
//              Theo dõi tín hiệu WDI và phát hiện timeout. Khi timeout xảy ra,
//              kéo WDO xuống thấp trong thời gian tRST rồi tự động phục hồi.
//
// FSM 4 trạng thái:
//   IDLE    → Watchdog tắt, chờ EN lên HIGH
//   ARMING  → Đếm arm_delay sau khi EN bật, bỏ qua mọi WDI
//   MONITOR → Đang theo dõi kick, đếm tWD
//   FAULT   → Timeout, giữ WDO=0 trong tRST rồi về MONITOR
//
// Tham số mặc định (theo tham khảo datasheet TPS3431):
//   tWD       = 1600 ms  (chế độ CWD pin để hở — NC)
//   tRST      = 200  ms
//   arm_delay = 150  µs
//
// Nguyên lý phát hiện cạnh xuống WDI:
//   Dùng 3 flip-flop: wdi_s0/s1 là 2-FF synchronizer chống metastability,
//   wdi_s2 trễ thêm 1 chu kỳ để phát hiện cạnh.
//   wdi_fall = wdi_s2 & ~wdi_s1  (trước=1, sau=0 → cạnh xuống)
// =============================================================================

module watchdog_core (
    input  clk,               // Xung nhịp hệ thống 27 MHz
    input  rst_n,             // Reset bất đồng bộ, tích cực thấp (từ POR)

    input  en,                // Enable: 1 = bật watchdog (CTRL[0] hoặc nút S2)
    input  wdi,               // Tín hiệu kick tổng hợp (nút S1 hoặc UART kick)
    input  clr_fault,         // Xóa lỗi ngay lập tức (write-1-to-clear CTRL[2])

    output reg wdo,           // WDO output: 0 = lỗi (active-low, mô phỏng open-drain)
    output reg enout,         // ENOUT: HIGH sau khi arm_delay hoàn thành

    // Giới hạn thời gian từ config_reg (đã quy đổi sang số chu kỳ clock)
    input [31:0] twd_limit,         // Ngưỡng timeout tWD
    input [31:0] trst_limit,        // Thời gian giữ WDO thấp khi lỗi
    input [31:0] arm_delay_limit,   // Cửa sổ bỏ qua WDI sau khi bật EN

    output reg fault_active         // 1 khi đang ở trạng thái FAULT
);

    // =========================================================================
    // Định nghĩa trạng thái FSM
    // =========================================================================
    localparam IDLE    = 2'd0;  // Watchdog tắt
    localparam ARMING  = 2'd1;  // Đang đếm arm_delay, bỏ qua WDI
    localparam MONITOR = 2'd2;  // Đang giám sát kick WDI
    localparam FAULT   = 2'd3;  // Timeout, đang giữ WDO thấp

    reg [1:0]  state;
    reg [31:0] counter;   // Bộ đếm dùng chung (arm_delay / tWD / tRST)

    // =========================================================================
    // Phát hiện cạnh xuống của WDI (3-flip-flop)
    // =========================================================================
    reg wdi_s0, wdi_s1, wdi_s2;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            // Khởi tạo = 1 (UART idle / nút không nhấn)
            wdi_s0 <= 1'b1;
            wdi_s1 <= 1'b1;
            wdi_s2 <= 1'b1;
        end else begin
            wdi_s0 <= wdi;       // FF1: đồng bộ hóa lần 1
            wdi_s1 <= wdi_s0;    // FF2: đồng bộ hóa lần 2 (chống metastability)
            wdi_s2 <= wdi_s1;    // FF3: trễ để phát hiện cạnh
        end
    end

    // Cạnh xuống hợp lệ: chu kỳ trước HIGH (wdi_s2=1), bây giờ LOW (wdi_s1=0)
    wire wdi_fall = wdi_s2 & ~wdi_s1;

    // =========================================================================
    // FSM chính + bộ đếm
    // =========================================================================
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            // Trạng thái an toàn sau reset: watchdog tắt, không có lỗi
            state        <= IDLE;
            counter      <= 32'd0;
            wdo          <= 1'b1;    // WDO = 1: bình thường (không lỗi)
            enout        <= 1'b0;    // ENOUT = 0: chưa arm
            fault_active <= 1'b0;
        end else begin
            case (state)

                // =============================================================
                // IDLE: Watchdog đang tắt, chờ EN lên HIGH
                // Tất cả tín hiệu đầu ra ở trạng thái an toàn (safe state)
                // =============================================================
                IDLE: begin
                    wdo          <= 1'b1;   // Giải phóng WDO (không lỗi)
                    enout        <= 1'b0;   // Chưa arm
                    fault_active <= 1'b0;
                    counter      <= 32'd0;
                    if (en)
                        state <= ARMING;    // EN lên → bắt đầu đếm arm_delay
                end

                // =============================================================
                // ARMING: Đang đếm arm_delay sau khi EN bật
                // Toàn bộ kick WDI trong giai đoạn này bị bỏ qua (không check
                // wdi_fall), giúp hệ thống có thời gian ổn định sau khi bật.
                // =============================================================
                ARMING: begin
                    enout <= 1'b0;   // ENOUT vẫn = 0 trong suốt arm_delay
                    if (!en) begin
                        // EN bị tắt giữa chừng → về IDLE ngay
                        state   <= IDLE;
                        counter <= 32'd0;
                    end else if (counter >= arm_delay_limit) begin
                        // Đã qua arm_delay → bắt đầu giám sát kick
                        state   <= MONITOR;
                        counter <= 32'd0;
                        enout   <= 1'b1;    // ENOUT lên 1: watchdog đã hoạt động
                    end else begin
                        counter <= counter + 1'b1;
                    end
                end

                // =============================================================
                // MONITOR: Đang giám sát tín hiệu kick WDI
                // Mỗi cạnh xuống hợp lệ → reset bộ đếm (gia hạn timeout)
                // Không nhận kick trong tWD chu kỳ → chuyển sang FAULT
                // =============================================================
                MONITOR: begin
                    enout <= 1'b1;   // ENOUT giữ = 1 khi đang giám sát
                    if (!en) begin
                        // Watchdog bị tắt → về IDLE, giải phóng output
                        state   <= IDLE;
                        counter <= 32'd0;
                    end else if (wdi_fall) begin
                        // Kick hợp lệ (cạnh xuống) → reset bộ đếm, ở lại MONITOR
                        counter <= 32'd0;
                    end else if (counter >= twd_limit) begin
                        // Timeout: không nhận kick trong tWD → chuyển FAULT
                        state        <= FAULT;
                        counter      <= 32'd0;
                        wdo          <= 1'b0;   // Kéo WDO xuống 0 (báo lỗi)
                        fault_active <= 1'b1;
                    end else begin
                        counter <= counter + 1'b1;
                    end
                end

                // =============================================================
                // FAULT: Đang giữ WDO thấp trong thời gian tRST
                // Sau tRST → tự động về MONITOR (chu kỳ mới, không cần arm lại)
                // clr_fault = 1 → phục hồi ngay lập tức không cần chờ tRST
                // =============================================================
                FAULT: begin
                    enout <= 1'b1;   // ENOUT vẫn = 1 trong FAULT
                    if (!en) begin
                        // Tắt watchdog → giải phóng WDO, về IDLE
                        state        <= IDLE;
                        counter      <= 32'd0;
                        wdo          <= 1'b1;
                        fault_active <= 1'b0;
                    end else if (clr_fault) begin
                        // Lệnh xóa lỗi từ UART (write 1 vào CTRL[2]) → phục hồi ngay
                        state        <= MONITOR;
                        counter      <= 32'd0;
                        wdo          <= 1'b1;
                        fault_active <= 1'b0;
                    end else if (counter >= trst_limit) begin
                        // Hết tRST → tự động phục hồi, bắt đầu chu kỳ giám sát mới
                        state        <= MONITOR;
                        counter      <= 32'd0;
                        wdo          <= 1'b1;
                        fault_active <= 1'b0;
                    end else begin
                        counter <= counter + 1'b1;
                        wdo     <= 1'b0;    // Tiếp tục giữ WDO thấp trong tRST
                    end
                end

                default: state <= IDLE;  // An toàn: về IDLE nếu state không hợp lệ

            endcase
        end
    end

endmodule
