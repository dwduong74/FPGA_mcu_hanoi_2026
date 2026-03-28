module imu_buffer (
    input clk,
    input rst_n,
    
    input [7:0] byte_in,
    input byte_ready,
    
    output reg [479:0] bnn_window, 
    output reg window_ready // Pulses high for 1 cycle when a full 6-axis sample is added
);

    reg [2:0] byte_cnt;
    reg [5:0] current_sample_bits;
    reg sample_ready;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            byte_cnt <= 3'd0;
            current_sample_bits <= 6'd0;
            sample_ready <= 1'b0;
        end else if (byte_ready) begin
            current_sample_bits <= {current_sample_bits[4:0], byte_in[7]};
            if (byte_cnt == 3'd5) begin
                byte_cnt <= 3'd0;
                sample_ready <= 1'b1;
            end else begin
                byte_cnt <= byte_cnt + 1'b1;
                sample_ready <= 1'b0;
            end
        end else begin
            sample_ready <= 1'b0;
        end
    end

    reg [6:0] fill_cnt;
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            bnn_window <= 480'd0;
            fill_cnt <= 7'd0;
            window_ready <= 1'b0;
        end else if (sample_ready) begin
            bnn_window <= {bnn_window[473:0], current_sample_bits};
            if (fill_cnt < 80) begin
                fill_cnt <= fill_cnt + 1'b1;
                window_ready <= 1'b0;
            end else begin
                window_ready <= 1'b1; // Pulse high for 1 cycle to trigger CNN
            end
        end else begin
            window_ready <= 1'b0;
        end
    end
endmodule
