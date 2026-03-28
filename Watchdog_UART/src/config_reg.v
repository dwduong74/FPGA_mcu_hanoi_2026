module config_reg (
    input clk,
    input rst_n,
    
    input [7:0] rx_data,
    input rx_done,
    
    output reg [31:0] twd_limit,
    output reg [31:0] trst_limit,
    output reg [31:0] arm_delay_limit,
    output reg en
);

    reg [2:0] byte_cnt;
    reg [7:0] cmd;
    reg [31:0] temp_data;
    
    // Default values (at 27 MHz)
    // tWD: 1s = 27,000,000
    // tRST: 100ms = 2,700,000
    // arm_delay: 2s = 54,000,000
    
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            twd_limit <= 32'd27_000_000;
            trst_limit <= 32'd2_700_000;
            arm_delay_limit <= 32'd54_000_000;
            en <= 1'b1;
            byte_cnt <= 0;
            temp_data <= 0;
        end else if (rx_done) begin
            if (byte_cnt == 0) begin
                cmd <= rx_data;
                byte_cnt <= 1;
                temp_data <= 0;
            end else begin
                temp_data <= {temp_data[23:0], rx_data};
                if (byte_cnt == 4) begin
                    case (cmd)
                        8'hA1: twd_limit <= {temp_data[23:0], rx_data};
                        8'hA2: trst_limit <= {temp_data[23:0], rx_data};
                        8'hA3: arm_delay_limit <= {temp_data[23:0], rx_data};
                        8'hA4: en <= rx_data[0];
                    endcase
                    byte_cnt <= 0;
                end else begin
                    byte_cnt <= byte_cnt + 1;
                end
            end
        end
    end

endmodule
