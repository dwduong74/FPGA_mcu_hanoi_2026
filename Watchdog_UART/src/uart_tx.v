module uart_tx #(
    parameter CLK_FREQ = 27_000_000,
    parameter BAUD_RATE = 9600
)(
    input clk,
    input rst_n,
    input [7:0] tx_data,
    input tx_en,
    output reg tx,
    output reg tx_done,
    output reg tx_busy
);

    localparam BIT_COUNT = CLK_FREQ / BAUD_RATE;

    reg [3:0] state;
    reg [31:0] timer;
    reg [2:0] bit_idx;
    reg [7:0] data_reg;
    
    localparam IDLE  = 4'd0;
    localparam START = 4'd1;
    localparam DATA  = 4'd2;
    localparam STOP  = 4'd3;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state <= IDLE;
            tx <= 1'b1;
            tx_done <= 1'b0;
            tx_busy <= 1'b0;
            timer <= 0;
        end else begin
            tx_done <= 1'b0;
            case (state)
                IDLE: begin
                    if (tx_en) begin
                        state <= START;
                        data_reg <= tx_data;
                        tx_busy <= 1'b1;
                        tx <= 1'b0; // Start bit
                        timer <= 0;
                    end else begin
                        tx <= 1'b1;
                        tx_busy <= 1'b0;
                    end
                end
                START: begin
                    if (timer == BIT_COUNT - 1) begin
                        state <= DATA;
                        timer <= 0;
                        bit_idx <= 0;
                        tx <= data_reg[0];
                    end else timer <= timer + 1;
                end
                DATA: begin
                    if (timer == BIT_COUNT - 1) begin
                        timer <= 0;
                        if (bit_idx == 7) begin
                            state <= STOP;
                            tx <= 1'b1; // Stop bit
                        end else begin
                            bit_idx <= bit_idx + 1;
                            tx <= data_reg[bit_idx + 1];
                        end
                    end else timer <= timer + 1;
                end
                STOP: begin
                    if (timer == BIT_COUNT - 1) begin
                        tx_done <= 1'b1;
                        tx_busy <= 1'b0;
                        state <= IDLE;
                    end else timer <= timer + 1;
                end
            endcase
        end
    end
endmodule
