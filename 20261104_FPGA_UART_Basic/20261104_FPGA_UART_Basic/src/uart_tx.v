module uart_tx #(
    parameter CLKS_PER_BIT = 234
)(
    input  wire clk,
    input  wire rst_n,
    input  wire tx_start,
    input  wire [7:0] tx_data,
    output reg  tx,
    output reg  tx_busy,
    output reg  tx_done
);

localparam S_IDLE  = 2'd0;
localparam S_START = 2'd1;
localparam S_DATA  = 2'd2;
localparam S_STOP  = 2'd3;

reg [1:0] state;
reg [7:0] data_r;
reg [2:0] bit_idx;
reg [15:0] clk_cnt;

always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        state   <= S_IDLE;
        tx      <= 1'b1;
        tx_busy <= 1'b0;
        tx_done <= 1'b0;
        data_r  <= 8'd0;
        bit_idx <= 3'd0;
        clk_cnt <= 16'd0;
    end else begin
        tx_done <= 1'b0;

        case (state)
            S_IDLE: begin
                tx      <= 1'b1;
                tx_busy <= 1'b0;
                clk_cnt <= 16'd0;
                bit_idx <= 3'd0;

                if (tx_start) begin
                    tx_busy <= 1'b1;
                    data_r  <= tx_data;
                    state   <= S_START;
                end
            end

            S_START: begin
                tx <= 1'b0;
                if (clk_cnt == CLKS_PER_BIT - 1) begin
                    clk_cnt <= 16'd0;
                    state   <= S_DATA;
                end else begin
                    clk_cnt <= clk_cnt + 16'd1;
                end
            end

            S_DATA: begin
                tx <= data_r[bit_idx];
                if (clk_cnt == CLKS_PER_BIT - 1) begin
                    clk_cnt <= 16'd0;
                    if (bit_idx == 3'd7) begin
                        bit_idx <= 3'd0;
                        state   <= S_STOP;
                    end else begin
                        bit_idx <= bit_idx + 3'd1;
                    end
                end else begin
                    clk_cnt <= clk_cnt + 16'd1;
                end
            end

            S_STOP: begin
                tx <= 1'b1;
                if (clk_cnt == CLKS_PER_BIT - 1) begin
                    clk_cnt <= 16'd0;
                    tx_done <= 1'b1;
                    state   <= S_IDLE;
                end else begin
                    clk_cnt <= clk_cnt + 16'd1;
                end
            end

            default: begin
                state <= S_IDLE;
            end
        endcase
    end
end

endmodule