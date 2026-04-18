// =============================================================================
// Đội thi    : Sunflower - PTIT
// Cuộc thi   : FPGA Extended Contest 2026 — Vòng sơ loại
// Nền tảng   : Kiwi 1P5 (Gowin GW1N-UV1P5QN48XFC7/I6)
// Tệp        : config_reg.v
// Module     : config_reg
// Mô tả      : Thanh ghi cấu hình kết hợp parser giao thức UART.
//              Nhận frame từ uart_rx, giải mã lệnh, ghi/đọc thanh ghi,
//              và gửi response về PC qua uart_tx.
//
// Giao thức frame (PC → FPGA):
//   [0x55][CMD][ADDR][LEN][DATA_0]...[DATA_N-1][CHK]
//   CHK = XOR(CMD, ADDR, LEN, DATA_0, ..., DATA_N-1)
//
// Bảng lệnh:
//   CMD 0x01 WRITE_REG : Ghi thanh ghi tại ADDR, response ACK 4 byte
//   CMD 0x02 READ_REG  : Đọc thanh ghi tại ADDR, response 8 byte
//   CMD 0x03 KICK      : Tạo 1 kick phần mềm,  response ACK 4 byte
//   CMD 0x04 GET_STATUS: Đọc nhanh STATUS,      response 7 byte
//   NACK 0xFE          : Checksum sai
//   NACK 0xFF          : CMD không hợp lệ
//
// Register map:
//   0x00 CTRL         bit0=EN_SW, bit2=CLR_FAULT (write-1-clear)
//   0x04 tWD_ms       Timeout watchdog (ms), mặc định 1600
//   0x08 tRST_ms      Thời gian giữ WDO thấp (ms), mặc định 200
//   0x0C arm_delay_us Delay bỏ qua WDI sau enable (µs), mặc định 150
//   0x10 STATUS       Chỉ đọc — xem STATUS bits bên dưới
//
// STATUS bits (reg_status[4:0]):
//   bit4 = last_kick_src  (0=button S1, 1=UART)
//   bit3 = wdo            (1=OK, 0=FAULT)
//   bit2 = enout          (1=đã arm)
//   bit1 = fault_active   (1=đang lỗi)
//   bit0 = en_effective   (1=đang enable)
// =============================================================================

module config_reg #(
    parameter CLK_FREQ = 27_000_000   // Tần số clock mặc định 27 MHz
)(
    input  clk,           // Xung nhịp hệ thống
    input  rst_n,         // Reset bất đồng bộ, tích cực thấp

    // Giao tiếp với uart_rx / uart_tx
    input  [7:0] rx_data, // Byte dữ liệu vừa nhận từ PC
    input        rx_done, // Xung 1 chu kỳ: nhận xong 1 byte
    output reg [7:0] tx_data, // Byte dữ liệu cần phát về PC
    output reg       tx_en,   // Xung 1 chu kỳ: kích hoạt phát
    input            tx_busy, // HIGH khi uart_tx đang phát

    // Các giới hạn thời gian (quy đổi từ ms/µs → số chu kỳ clock)
    output [31:0] twd_limit,        // tWD * (CLK_FREQ/1000)
    output [31:0] trst_limit,       // tRST * (CLK_FREQ/1000)
    output [31:0] arm_delay_limit,  // arm_delay * (CLK_FREQ/1000000)

    // Điều khiển
    output        en_sw,      // EN_SW từ CTRL[0] → enable watchdog qua UART
    output reg    clr_fault,  // Xung xóa lỗi (từ CTRL[2], write-1-clear)
    output reg    uart_kick,  // Xung kick phần mềm (CMD 0x03)

    // Trạng thái thời gian thực từ watchdog_core (dùng cho STATUS)
    input fault_active,       // Watchdog đang ở trạng thái FAULT
    input enout,              // Watchdog đã arm và đang chạy
    input wdo,                // Đầu ra WDO hiện tại (1=OK, 0=lỗi)
    input en_effective,       // Enable tổng hợp (CTRL[0] OR nút S2)
    input last_kick_src       // Nguồn kick gần nhất (0=button, 1=UART)
);

    // =========================================================================
    // Các thanh ghi cấu hình — giá trị mặc định theo TPS3431
    // =========================================================================
    reg [31:0] reg_ctrl    = 32'd0;     // CTRL: bit0=EN_SW, bit2=CLR_FAULT
    reg [31:0] reg_twd_ms  = 32'd1600;  // tWD timeout watchdog (ms)
    reg [31:0] reg_trst_ms = 32'd200;   // tRST giữ WDO thấp khi lỗi (ms)
    reg [15:0] reg_arm_us  = 16'd150;   // arm_delay bỏ qua WDI sau enable (µs)

    // =========================================================================
    // Thanh ghi trạng thái (chỉ đọc, wire — phản ánh realtime)
    // =========================================================================
    // Ghép các bit trạng thái từ watchdog_core và sticky bit thành 1 word 32 bit
    wire [31:0] reg_status = {27'd0, last_kick_src, wdo, enout, fault_active, en_effective};

    // =========================================================================
    // Quy đổi tham số thời gian → số chu kỳ clock
    // =========================================================================
    assign twd_limit       = reg_twd_ms  * (CLK_FREQ / 1000);     // ms → cycles
    assign trst_limit      = reg_trst_ms * (CLK_FREQ / 1000);     // ms → cycles
    assign arm_delay_limit = reg_arm_us  * (CLK_FREQ / 1000000);  // µs → cycles
    assign en_sw           = reg_ctrl[0];  // Bit enable từ CTRL

    // =========================================================================
    // Mux đọc thanh ghi — combinatorial (dùng cho CMD 0x02 READ_REG)
    // -------------------------------------------------------------------------
    // Chọn giá trị 32-bit để trả về dựa theo f_addr nhận được trong frame.
    // Phải là combinatorial để tính checksum ngay trong cùng clock cycle.
    // =========================================================================
    reg [31:0] rd_mux;
    always @(*) begin
        case (f_addr)
            8'h00:   rd_mux = reg_ctrl;                    // CTRL
            8'h04:   rd_mux = reg_twd_ms;                  // tWD_ms
            8'h08:   rd_mux = reg_trst_ms;                 // tRST_ms
            8'h0C:   rd_mux = {16'h0000, reg_arm_us};      // arm_delay_us (zero-extend)
            8'h10:   rd_mux = reg_status;                  // STATUS (read-only)
            default: rd_mux = 32'h0;                       // Địa chỉ không hợp lệ
        endcase
    end

    // =========================================================================
    // Định nghĩa các trạng thái của parser UART
    // =========================================================================
    localparam PS_HDR  = 3'd0;  // Chờ byte header 0x55
    localparam PS_CMD  = 3'd1;  // Nhận byte lệnh CMD
    localparam PS_ADDR = 3'd2;  // Nhận địa chỉ thanh ghi ADDR
    localparam PS_LEN  = 3'd3;  // Nhận độ dài dữ liệu LEN
    localparam PS_DATA = 3'd4;  // Nhận LEN byte dữ liệu
    localparam PS_CHK  = 3'd5;  // Nhận checksum và xử lý lệnh

    // Biến trạng thái parser
    reg [2:0] ps;               // Trạng thái hiện tại của parser
    reg [7:0] f_cmd;            // CMD byte của frame hiện tại
    reg [7:0] f_addr;           // ADDR byte của frame hiện tại
    reg [7:0] f_len;            // LEN byte (số byte data)
    reg [7:0] f_chk;            // Checksum đang tính dần (XOR từng byte)
    reg [7:0] f_dcnt;           // Đếm số byte data đã nhận
    reg [7:0] f_data [0:3];     // Buffer chứa tối đa 4 byte data

    // Biến quản lý TX response
    reg [7:0] resp [0:7];       // Buffer response tối đa 8 byte
    reg [3:0] r_len;            // Số byte cần gửi trong response
    reg [3:0] r_idx;            // Chỉ số byte đang gửi
    reg       r_pending;        // 1 = đang có response chờ được gửi

    // =========================================================================
    // Logic chính: TX sequencer + UART frame parser
    // =========================================================================
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            // Khởi tạo: về trạng thái chờ header, xóa response buffer
            ps          <= PS_HDR;
            r_pending   <= 1'b0;
            r_idx       <= 4'd0;
            // Giá trị mặc định các thanh ghi
            reg_ctrl    <= 32'd0;
            reg_twd_ms  <= 32'd1600;
            reg_trst_ms <= 32'd200;
            reg_arm_us  <= 16'd150;
        end else begin
            // Xóa các xung 1 chu kỳ mỗi clk (default = 0)
            clr_fault <= 1'b0;
            uart_kick <= 1'b0;
            tx_en     <= 1'b0;

            // =================================================================
            // TX Sequencer: gửi lần lượt từng byte trong buffer resp[]
            // Điều kiện: có response pending, uart_tx không bận, chưa kích tx
            // =================================================================
            if (r_pending && !tx_busy && !tx_en) begin
                tx_data <= resp[r_idx];   // Nạp byte tiếp theo vào uart_tx
                tx_en   <= 1'b1;          // Kích uart_tx phát byte này
                if (r_idx == r_len - 4'd1) begin
                    // Đây là byte cuối → xóa cờ pending
                    r_pending <= 1'b0;
                    r_idx     <= 4'd0;
                end else begin
                    r_idx <= r_idx + 4'd1;
                end
            end

            // =================================================================
            // UART Frame Parser: xử lý từng byte nhận được từ uart_rx
            // =================================================================
            if (rx_done) begin
                case (ps)

                    // ---------------------------------------------------------
                    // PS_HDR: Chờ byte header 0x55
                    // ---------------------------------------------------------
                    PS_HDR: begin
                        if (rx_data == 8'h55)
                            ps <= PS_CMD;   // Header đúng → nhận CMD tiếp theo
                        // Bất kỳ byte nào khác đều bị bỏ qua (tự đồng bộ lại)
                    end

                    // ---------------------------------------------------------
                    // PS_CMD: Lưu CMD, bắt đầu tính checksum
                    // ---------------------------------------------------------
                    PS_CMD: begin
                        f_cmd <= rx_data;
                        f_chk <= rx_data;   // CHK = CMD ^ ADDR ^ LEN ^ DATA...
                        ps    <= PS_ADDR;
                    end

                    // ---------------------------------------------------------
                    // PS_ADDR: Lưu địa chỉ thanh ghi, XOR vào checksum
                    // ---------------------------------------------------------
                    PS_ADDR: begin
                        f_addr <= rx_data;
                        f_chk  <= f_chk ^ rx_data;
                        ps     <= PS_LEN;
                    end

                    // ---------------------------------------------------------
                    // PS_LEN: Lưu số byte data, nếu LEN=0 thì bỏ qua PS_DATA
                    // ---------------------------------------------------------
                    PS_LEN: begin
                        f_len  <= rx_data;
                        f_chk  <= f_chk ^ rx_data;
                        f_dcnt <= 8'd0;
                        // LEN=0 (lệnh không có data) → nhảy thẳng đến checksum
                        ps <= (rx_data == 8'h00) ? PS_CHK : PS_DATA;
                    end

                    // ---------------------------------------------------------
                    // PS_DATA: Nhận từng byte data, XOR vào checksum
                    // ---------------------------------------------------------
                    PS_DATA: begin
                        f_data[f_dcnt] <= rx_data;
                        f_chk          <= f_chk ^ rx_data;
                        if (f_dcnt == f_len - 8'd1)
                            ps <= PS_CHK;              // Đủ LEN byte → kiểm tra CHK
                        else
                            f_dcnt <= f_dcnt + 8'd1;
                    end

                    // ---------------------------------------------------------
                    // PS_CHK: Nhận checksum, so sánh và thực thi lệnh
                    // ---------------------------------------------------------
                    PS_CHK: begin
                        ps <= PS_HDR;   // Luôn về HDR sau khi xử lý xong frame

                        if (rx_data == f_chk) begin
                            // Checksum khớp → thực thi lệnh
                            case (f_cmd)

                                // ─────────────────────────────────────────────
                                // CMD 0x01 — WRITE_REG
                                // Ghi giá trị f_data[3:0] (little-endian) vào
                                // thanh ghi tại địa chỉ f_addr.
                                // Response: [AA][01][00][01] (4 byte ACK)
                                // ─────────────────────────────────────────────
                                8'h01: begin
                                    case (f_addr)
                                        8'h00: begin
                                            // Ghi CTRL
                                            reg_ctrl <= {f_data[3], f_data[2],
                                                         f_data[1], f_data[0]};
                                            // bit2=CLR_FAULT: write-1-to-clear
                                            if (f_data[0][2]) clr_fault <= 1'b1;
                                        end
                                        8'h04: reg_twd_ms  <= {f_data[3], f_data[2],
                                                                f_data[1], f_data[0]};
                                        8'h08: reg_trst_ms <= {f_data[3], f_data[2],
                                                                f_data[1], f_data[0]};
                                        8'h0C: reg_arm_us  <= {f_data[1], f_data[0]};
                                        // Địa chỉ khác: bỏ qua (STATUS read-only)
                                    endcase
                                    resp[0] <= 8'hAA; resp[1] <= 8'h01;
                                    resp[2] <= 8'h00; resp[3] <= 8'h01;
                                    r_len <= 4'd4; r_pending <= 1'b1;
                                end

                                // ─────────────────────────────────────────────
                                // CMD 0x02 — READ_REG
                                // Đọc thanh ghi tại f_addr qua rd_mux (comb).
                                // Response: [AA][02][00][D0][D1][D2][D3][CHK]
                                // CHK = 0x02 ^ 0x00 ^ D0 ^ D1 ^ D2 ^ D3
                                // ─────────────────────────────────────────────
                                8'h02: begin
                                    resp[0] <= 8'hAA;
                                    resp[1] <= 8'h02;
                                    resp[2] <= 8'h00;
                                    resp[3] <= rd_mux[7:0];    // D0: byte thấp nhất
                                    resp[4] <= rd_mux[15:8];   // D1
                                    resp[5] <= rd_mux[23:16];  // D2
                                    resp[6] <= rd_mux[31:24];  // D3: byte cao nhất
                                    resp[7] <= 8'h02 ^ 8'h00
                                              ^ rd_mux[7:0]   ^ rd_mux[15:8]
                                              ^ rd_mux[23:16] ^ rd_mux[31:24];
                                    r_len <= 4'd8; r_pending <= 1'b1;
                                end

                                // ─────────────────────────────────────────────
                                // CMD 0x03 — KICK
                                // Tạo 1 xung uart_kick → watchdog_top tạo
                                // xung WDI giả 20 chu kỳ → watchdog_core
                                // nhận cạnh xuống → reset bộ đếm tWD.
                                // Response: [AA][03][00][03] (4 byte ACK)
                                // ─────────────────────────────────────────────
                                8'h03: begin
                                    uart_kick <= 1'b1;   // Xung 1 chu kỳ
                                    resp[0] <= 8'hAA; resp[1] <= 8'h03;
                                    resp[2] <= 8'h00; resp[3] <= 8'h03;
                                    r_len <= 4'd4; r_pending <= 1'b1;
                                end

                                // ─────────────────────────────────────────────
                                // CMD 0x04 — GET_STATUS
                                // Đọc nhanh reg_status (wire realtime).
                                // Response: [AA][04][00][D0][D1][D2][CHK]
                                // CHK = 0x04 ^ 0x00 ^ D0 ^ D1 ^ D2
                                // ─────────────────────────────────────────────
                                8'h04: begin
                                    resp[0] <= 8'hAA;
                                    resp[1] <= 8'h04;
                                    resp[2] <= 8'h00;
                                    resp[3] <= reg_status[7:0];   // D0: chứa các bit trạng thái
                                    resp[4] <= reg_status[15:8];  // D1: luôn = 0
                                    resp[5] <= reg_status[23:16]; // D2: luôn = 0
                                    resp[6] <= 8'h04 ^ 8'h00
                                              ^ reg_status[7:0]
                                              ^ reg_status[15:8]
                                              ^ reg_status[23:16];
                                    r_len <= 4'd7; r_pending <= 1'b1;
                                end

                                // ─────────────────────────────────────────────
                                // CMD không hợp lệ → NACK 0xFF
                                // ─────────────────────────────────────────────
                                default: begin
                                    resp[0] <= 8'hAA; resp[1] <= 8'hFF;
                                    resp[2] <= 8'h00; resp[3] <= 8'hFF;
                                    r_len <= 4'd4; r_pending <= 1'b1;
                                end

                            endcase

                        end else begin
                            // Checksum không khớp → NACK 0xFE
                            resp[0] <= 8'hAA; resp[1] <= 8'hFE;
                            resp[2] <= 8'h00; resp[3] <= 8'hFE;
                            r_len <= 4'd4; r_pending <= 1'b1;
                        end
                    end

                endcase
            end // if (rx_done)
        end
    end

endmodule
