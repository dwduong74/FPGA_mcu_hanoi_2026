module gesture_bcnn (
    input clk,
    input rst_n,
    input [479:0] bnn_input,
    input window_valid,
    output reg [1:0] gesture_out
);

    // 1. Convolutional Weights
    wire [17:0] filter_0 = 18'h2AAAA; 
    wire [17:0] filter_1 = 18'h15555;
    wire [17:0] filter_2 = 18'h3F000;

    // 2. Dataflow registers
    reg [479:0] shift_reg;
    reg [2:0] state;
    reg [6:0] step_cnt;
    reg [1:0] filter_idx;
    reg [119:0] feature_map;
    
    localparam IDLE  = 3'd0;
    localparam LOAD  = 3'd1;
    localparam CONV  = 3'd2;
    localparam DENSE = 3'd3;
    localparam DONE  = 3'd4;

    wire [17:0] current_window = shift_reg[17:0];

    integer k, m;
    reg [4:0] score;
    reg [5:0] dense_pop_0, dense_pop_1, dense_pop_2;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state <= IDLE;
            step_cnt <= 0;
            filter_idx <= 0;
            feature_map <= 120'd0;
            gesture_out <= 2'b00;
            shift_reg <= 0;
        end else begin
            case (state)
                IDLE: begin
                    if (window_valid) begin
                        state <= LOAD;
                        filter_idx <= 0;
                    end
                end

                LOAD: begin
                    shift_reg <= bnn_input;
                    state <= CONV;
                    step_cnt <= 0;
                end

                CONV: begin
                    if (step_cnt[0] == 0) begin : calc_block
                        score = 0;
                        for(k=0; k<18; k=k+1) begin
                            score = score + ~(current_window[k] ^ (
                                (filter_idx == 0) ? filter_0[k] :
                                (filter_idx == 1) ? filter_1[k] : filter_2[k]
                            ));
                        end
                        feature_map[filter_idx*40 + (step_cnt >> 1)] <= (score > 10);
                    end

                    shift_reg <= {6'd0, shift_reg[479:6]};

                    if (step_cnt >= 77) begin
                        if (filter_idx == 2) state <= DENSE;
                        else begin
                            state <= LOAD;
                            filter_idx <= filter_idx + 1'b1;
                        end
                    end else begin
                        step_cnt <= step_cnt + 1'b1;
                    end
                end

                DENSE: begin
                    // FIX: Use Popcount for decision instead of raw vector comparison
                    dense_pop_0 = 0;
                    dense_pop_1 = 0;
                    dense_pop_2 = 0;
                    for (m = 0; m < 40; m = m + 1) begin
                        dense_pop_0 = dense_pop_0 + feature_map[m];      // Filter 0 area
                        dense_pop_1 = dense_pop_1 + feature_map[m + 40]; // Filter 1 area
                        dense_pop_2 = dense_pop_2 + feature_map[m + 80]; // Filter 2 area
                    end

                    if (dense_pop_0 > 20 && dense_pop_0 > dense_pop_1 && dense_pop_0 > dense_pop_2)
                        gesture_out <= 2'b01; // Wave
                    else if (dense_pop_1 > 20 && dense_pop_1 > dense_pop_0 && dense_pop_1 > dense_pop_2)
                        gesture_out <= 2'b10; // Shake
                    else if (dense_pop_2 > 20)
                        gesture_out <= 2'b11; // Circle
                    else
                        gesture_out <= 2'b00;
                    
                    state <= DONE;
                end

                DONE: begin
                    state <= IDLE;
                end
            endcase
        end
    end
endmodule
