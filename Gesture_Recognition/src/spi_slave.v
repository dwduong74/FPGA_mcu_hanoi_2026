module spi_slave (
    input clk,          // 27MHz
    input rst_n,
    
    input sclk,
    input mosi,
    input cs_n,
    output miso,
    
    output reg [7:0] rx_byte,
    output reg rx_done
);

    reg [2:0] sclk_sync;
    reg [2:0] mosi_sync;
    reg [2:0] cs_n_sync;
    
    // Synchronize external signals
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            sclk_sync <= 3'b000;
            mosi_sync <= 3'b000;
            cs_n_sync <= 3'b111;
        end else begin
            sclk_sync <= {sclk_sync[1:0], sclk};
            mosi_sync <= {mosi_sync[1:0], mosi};
            cs_n_sync <= {cs_n_sync[1:0], cs_n};
        end
    end

    wire sclk_rising = (sclk_sync[2:1] == 2'b01);
    wire active      = ~cs_n_sync[1];

    reg [3:0] bit_cnt;
    reg [7:0] shift_reg;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            bit_cnt <= 4'd0;
            shift_reg <= 8'd0;
            rx_done <= 1'b0;
            rx_byte <= 8'd0;
        end else if (!active) begin
            bit_cnt <= 4'd0;
            rx_done <= 1'b0;
        end else begin
            // Default pulse state
            rx_done <= 1'b0;
            
            if (sclk_rising) begin
                if (bit_cnt < 8) begin
                    shift_reg <= {shift_reg[6:0], mosi_sync[1]};
                    bit_cnt <= bit_cnt + 1'b1;
                end
            end
            
            // Cycle 9: Latch data and trigger done pulse
            // This happens 1 clock cycle after bit_cnt reaches 8
            if (bit_cnt == 4'd8) begin
                rx_byte <= shift_reg;
                rx_done <= 1'b1;
                bit_cnt <= 4'd0;
            end
        end
    end

    assign miso = 1'bz;

endmodule
