// =============================================================================
// Đội thi    : Sunflower - PTIT
// Cuộc thi   : FPGA Extended Contest 2026 — Vòng sơ loại
// Nền tảng   : Kiwi 1P5 (Gowin GW1N-UV1P5QN48XFC7/I6)
// Tệp        : uart_tx.v
// Module     : uart_tx
// Mô tả      : Bộ phát UART, chuẩn 8N1 (8 bit dữ liệu, không parity, 1 stop bit).
//              FSM 4 trạng thái: IDLE → START → DATA → STOP
//
// Nguyên lý hoạt động:
//   - S_IDLE : Chờ tx_en = 1, đường TX giữ mức HIGH (idle)
//   - S_START: Kéo TX xuống LOW (bit Start) trong BIT_COUNT-1 chu kỳ
//   - S_DATA : Phát 8 bit dữ liệu, LSB trước, mỗi bit kéo dài BIT_COUNT-1 cy
//   - S_STOP : Kéo TX lên HIGH (bit Stop), phát tx_done = 1 khi xong
//
// Tín hiệu điều khiển:
//   tx_en   : Xung kích hoạt phát, chỉ cần HIGH 1 chu kỳ
//   tx_busy : HIGH trong toàn bộ quá trình phát (Start + Data + Stop)
//   tx_done : Xung HIGH 1 chu kỳ khi kết thúc bit Stop
//
// Tham số:
//   CLK_FREQ  : Tần số xung nhịp hệ thống (Hz), mặc định 27 MHz
//   BAUD_RATE : Tốc độ baud, mặc định 9600 bps
// =============================================================================

module uart_tx #(
    parameter CLK_FREQ  = 27_000_000,
    parameter BAUD_RATE = 9600
)(
    input  clk,
    input  rst_n,
    input  [7:0] tx_data,  // Byte dữ liệu cần phát
    input        tx_en,    // Xung kích hoạt phát (1 chu kỳ)
    output reg   tx,       // Đường phát dữ liệu nối tiếp
    output reg   tx_done,  // Xung HIGH 1 chu kỳ khi phát xong
    output reg   tx_busy   // HIGH trong suốt quá trình phát
);

    // Số chu kỳ cho mỗi bit
    localparam BIT_COUNT = CLK_FREQ / BAUD_RATE;

    // -------------------------------------------------------------------------
    // Máy trạng thái phát UART
    // -------------------------------------------------------------------------
    localparam S_IDLE  = 4'd0;  // Chờ lệnh phát
    localparam S_START = 4'd1;  // Phát bit Start (LOW)
    localparam S_DATA  = 4'd2;  // Phát 8 bit dữ liệu
    localparam S_STOP  = 4'd3;  // Phát bit Stop (HIGH)

    reg [3:0]  state;
    reg [31:0] timer;     // Bộ đếm thời gian trong từng trạng thái
    reg [2:0]  bit_idx;   // Chỉ số bit hiện tại (0..7)
    reg [7:0]  data_reg;  // Buffer lưu byte đang phát

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state    <= S_IDLE;
            tx       <= 1'b1;    // Trạng thái idle UART là HIGH
            tx_done  <= 1'b0;
            tx_busy  <= 1'b0;
            timer    <= 32'd0;
            bit_idx  <= 3'd0;
            data_reg <= 8'd0;
        end else begin
            tx_done <= 1'b0;  // Xóa cờ mặc định mỗi chu kỳ

            case (state)
                S_IDLE: begin
                    tx      <= 1'b1;   // Giữ đường TX ở mức HIGH
                    tx_busy <= 1'b0;
                    if (tx_en) begin
                        // Nhận lệnh phát → lưu dữ liệu và bắt đầu
                        data_reg <= tx_data;
                        tx_busy  <= 1'b1;
                        tx       <= 1'b0;  // Bit Start = LOW
                        timer    <= 32'd0;
                        state    <= S_START;
                    end
                end

                S_START: begin
                    // Duy trì bit Start trong BIT_COUNT-1 chu kỳ
                    if (timer == BIT_COUNT - 1) begin
                        state   <= S_DATA;
                        timer   <= 32'd0;
                        bit_idx <= 3'd0;
                        tx      <= data_reg[0];  // Phát bit LSB đầu tiên
                    end else begin
                        timer <= timer + 1'b1;
                    end
                end

                S_DATA: begin
                    // Phát từng bit, mỗi bit kéo dài BIT_COUNT chu kỳ
                    if (timer == BIT_COUNT - 1) begin
                        timer <= 32'd0;
                        if (bit_idx == 3'd7) begin
                            // Đã phát đủ 8 bit → chuyển sang Stop
                            state <= S_STOP;
                            tx    <= 1'b1;  // Bit Stop = HIGH
                        end else begin
                            bit_idx <= bit_idx + 1'b1;
                            tx      <= data_reg[bit_idx + 1];
                        end
                    end else begin
                        timer <= timer + 1'b1;
                    end
                end

                S_STOP: begin
                    // Duy trì bit Stop, sau đó về IDLE
                    if (timer == BIT_COUNT - 1) begin
                        tx_done <= 1'b1;   // Báo hiệu phát xong
                        tx_busy <= 1'b0;
                        state   <= S_IDLE;
                    end else begin
                        timer <= timer + 1'b1;
                    end
                end

                default: state <= S_IDLE;
            endcase
        end
    end

endmodule
