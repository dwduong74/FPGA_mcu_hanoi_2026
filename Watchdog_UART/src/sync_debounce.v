// =============================================================================
// Đội thi    : Sunflower - PTIT
// Cuộc thi   : FPGA Extended Contest 2026 — Vòng sơ loại RTL Design
// Nền tảng   : Kiwi 1P5 (Gowin GW1N-UV1P5QN48XFC7/I6)
// Tệp        : sync_debounce.v
// Module     : sync_debounce
// Mô tả      : Bộ đồng bộ hóa 2 flip-flop kết hợp chống rung cơ học nút nhấn.
//              Gồm 2 tầng:
//                1. 2-FF synchronizer: loại bỏ nguy cơ metastability khi
//                   tín hiệu nút nhấn không đồng bộ với xung nhịp hệ thống.
//                2. Bộ đếm chống rung: chỉ cập nhật đầu ra khi tín hiệu
//                   giữ ổn định liên tục trong DEBOUNCE_CYCLES chu kỳ.
//
// Tham số:
//   DEBOUNCE_CYCLES : Số chu kỳ ổn định cần thiết.
//                     Mặc định 540_000 ≈ 20 ms @ 27 MHz
//
// Giao tiếp:
//   btn_n   : Đầu vào nút nhấn, tích cực thấp (active-low)
//   btn_out : Đầu ra đã chống rung, tích cực cao (active-high)
//             HIGH = đang nhấn nút, LOW = không nhấn
// =============================================================================

module sync_debounce #(
    parameter DEBOUNCE_CYCLES = 540_000  // 20 ms @ 27 MHz
)(
    input  clk,
    input  rst_n,
    input  btn_n,       // Đầu vào nút nhấn tích cực thấp
    output reg btn_out  // Đầu ra đã chống rung, tích cực cao
);

    // -------------------------------------------------------------------------
    // Bước 1: Đồng bộ hóa 2 flip-flop
    // Loại bỏ nguy cơ metastability khi tín hiệu không đồng bộ
    // -------------------------------------------------------------------------
    reg sync0, sync1;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            sync0 <= 1'b1;  // Kéo lên HIGH (nút không nhấn)
            sync1 <= 1'b1;
        end else begin
            sync0 <= btn_n;
            sync1 <= sync0;
        end
    end

    // -------------------------------------------------------------------------
    // Bước 2: Bộ đếm chống rung
    // Chỉ thay đổi đầu ra khi tín hiệu giữ nguyên đủ DEBOUNCE_CYCLES chu kỳ
    // -------------------------------------------------------------------------
    reg [19:0] cnt;     // Đủ chứa 540_000 (cần 20 bit)
    reg stable;         // Trạng thái ổn định hiện tại của tín hiệu

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            cnt     <= 20'd0;
            stable  <= 1'b1;    // Nút không nhấn
            btn_out <= 1'b0;    // Đầu ra tích cực cao → 0 = không nhấn
        end else begin
            if (sync1 != stable) begin
                // Tín hiệu đang thay đổi → đếm thời gian ổn định
                if (cnt >= DEBOUNCE_CYCLES - 1) begin
                    // Đã ổn định đủ lâu → cập nhật đầu ra
                    stable  <= sync1;
                    btn_out <= ~sync1;  // Đảo lại: active-low → active-high
                    cnt     <= 20'd0;
                end else begin
                    cnt <= cnt + 1'b1;
                end
            end else begin
                // Tín hiệu không thay đổi → reset bộ đếm
                cnt <= 20'd0;
            end
        end
    end

endmodule
