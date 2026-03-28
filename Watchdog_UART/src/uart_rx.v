module uart_rx #(
    parameter CLK_FREQ = 27_000_000,
    parameter BAUD_RATE = 9600
)(
    input clk,
    input rst_n,
    input rx,
    output reg [7:0] rx_data,
    output reg rx_done
);

    localparam BIT_COUNT = CLK_FREQ / BAUD_RATE;
    localparam HALF_BIT_COUNT = BIT_COUNT / 2;

    reg [1:0] rx_reg;
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) rx_reg <= 2'b11;
        else rx_reg <= {rx_reg[0], rx};
    end
    
    wire rx_sync = rx_reg[1];
    reg rx_reg_prev;
    always @(posedge clk) rx_reg_prev <= rx_sync;
    wire start_bit = rx_reg_prev & ~rx_sync;

    reg [3:0] state;
    reg [31:0] timer;
    reg [2:0] bit_idx;
    
    localparam IDLE  = 4'd0;
    localparam START = 4'd1;
    localparam DATA  = 4'd2;
    localparam STOP  = 4'd3;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state <= IDLE;
            rx_done <= 1'b0;
            timer <= 0;
        end else begin
            rx_done <= 1'b0;
            case (state)
                IDLE: begin
                    if (start_bit) begin
                        state <= START;
                        timer <= 0;
                    end
                end
                START: begin
                    if (timer == HALF_BIT_COUNT) begin
                        state <= DATA;
                        timer <= 0;
                        bit_idx <= 0;
                    end else timer <= timer + 1;
                end
                DATA: begin
                    if (timer == BIT_COUNT) begin
                        rx_data[bit_idx] <= rx_sync;
                        timer <= 0;
                        if (bit_idx == 7) state <= STOP;
                        else bit_idx <= bit_idx + 1;
                    end else timer <= timer + 1;
                end
                STOP: begin
                    if (timer == BIT_COUNT) begin
                        rx_done <= 1'b1;
                        state <= IDLE;
                    end else timer <= timer + 1;
                end
            endcase
        end
    end
endmodule
