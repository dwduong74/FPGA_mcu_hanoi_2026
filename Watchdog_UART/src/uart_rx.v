// =============================================================================
// Đội thi    : Sunflower - PTIT
// Cuộc thi   : FPGA Extended Contest 2026 — Vòng sơ loại RTL Design
// Nền tảng   : Kiwi 1P5 (Gowin GW1N-UV1P5QN48XFC7/I6)
// Tệp        : uart_rx.v
// Module     : uart_rx
// Mô tả      : Bộ nhận UART, chuẩn 8N1 (8 bit dữ liệu, không parity, 1 stop bit).
//              FSM 4 trạng thái: IDLE → START → DATA → STOP
//
// Nguyên lý hoạt động:
//   - S_IDLE : Chờ cạnh xuống trên RX (bit Start), tín hiệu idle = HIGH
//   - S_START: Đợi HALF_BIT_COUNT chu kỳ để căn điểm lấy mẫu vào giữa bit
//   - S_DATA : Lấy mẫu 8 bit dữ liệu (LSB trước) tại cuối mỗi bit (BIT_COUNT-1)
//   - S_STOP : Đợi bit Stop, sau đó phát rx_done = HIGH trong 1 chu kỳ
//
// Tham số:
//   CLK_FREQ  : Tần số xung nhịp hệ thống (Hz), mặc định 27 MHz
//   BAUD_RATE : Tốc độ baud, mặc định 9600 bps
//
// Lưu ý đồng bộ: lấy mẫu tại BIT_COUNT-1 (không phải BIT_COUNT) để khớp
//   với thời điểm uart_tx chuyển bit — tránh đọc trong vùng chuyển tiếp.
// =============================================================================

module uart_rx #(
    parameter CLK_FREQ  = 27_000_000,
    parameter BAUD_RATE = 9600
)(
    input  clk,
    input  rst_n,
    input  rx,                  // Đường nhận dữ liệu nối tiếp
    output reg [7:0] rx_data,  // Byte dữ liệu đã nhận
    output reg       rx_done   // Xung HIGH 1 chu kỳ khi nhận xong 1 byte
);

    // Số chu kỳ cho mỗi bit = tần số / baud rate
    localparam BIT_COUNT      = CLK_FREQ / BAUD_RATE;
    localparam HALF_BIT_COUNT = BIT_COUNT / 2;  // Dùng để lấy mẫu ở giữa bit

    // -------------------------------------------------------------------------
    // Đồng bộ hóa tín hiệu RX vào miền xung nhịp hệ thống (2-FF)
    // -------------------------------------------------------------------------
    reg [1:0] rx_reg;
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) rx_reg <= 2'b11;        // Trạng thái idle UART là HIGH
        else        rx_reg <= {rx_reg[0], rx};
    end

    wire rx_sync = rx_reg[1];  // Tín hiệu RX đã đồng bộ

    // Phát hiện cạnh xuống (bit Start)
    reg rx_reg_prev;
    always @(posedge clk) rx_reg_prev <= rx_sync;
    wire start_bit = rx_reg_prev & ~rx_sync;  // HIGH 1 chu kỳ khi phát hiện Start

    // -------------------------------------------------------------------------
    // Máy trạng thái nhận UART
    // -------------------------------------------------------------------------
    localparam S_IDLE  = 4'd0;  // Chờ bit Start
    localparam S_START = 4'd1;  // Xác nhận bit Start (đợi nửa bit)
    localparam S_DATA  = 4'd2;  // Nhận 8 bit dữ liệu
    localparam S_STOP  = 4'd3;  // Đợi bit Stop

    reg [3:0]  state;
    reg [31:0] timer;    // Bộ đếm thời gian trong từng trạng thái
    reg [2:0]  bit_idx;  // Chỉ số bit hiện tại (0..7)

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state    <= S_IDLE;
            rx_done  <= 1'b0;
            timer    <= 32'd0;
            bit_idx  <= 3'd0;
            rx_data  <= 8'd0;
        end else begin
            rx_done <= 1'b0;  // Xóa cờ mặc định mỗi chu kỳ

            case (state)
                S_IDLE: begin
                    if (start_bit) begin
                        state <= S_START;
                        timer <= 32'd0;
                    end
                end

                S_START: begin
                    // Đợi nửa bit để căn giữa điểm lấy mẫu
                    if (timer == HALF_BIT_COUNT) begin
                        state   <= S_DATA;
                        timer   <= 32'd0;
                        bit_idx <= 3'd0;
                    end else begin
                        timer <= timer + 1'b1;
                    end
                end

                S_DATA: begin
                    // Lấy mẫu ở giữa mỗi bit (sau BIT_COUNT-1 chu kỳ, đồng bộ với uart_tx)
                    if (timer == BIT_COUNT - 1) begin
                        rx_data[bit_idx] <= rx_sync;  // Lưu bit vào buffer
                        timer <= 32'd0;
                        if (bit_idx == 3'd7)
                            state <= S_STOP;   // Đủ 8 bit → chờ Stop
                        else
                            bit_idx <= bit_idx + 1'b1;
                    end else begin
                        timer <= timer + 1'b1;
                    end
                end

                S_STOP: begin
                    // Đợi hết bit Stop rồi phát cờ hoàn thành
                    if (timer == BIT_COUNT - 1) begin
                        rx_done <= 1'b1;     // Nhận xong 1 byte
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
