module uart_reporter #(
    parameter CLK_FREQ_HZ = 27000000,
    parameter BAUD        = 115200
)(
    input  wire clk,
    input  wire rst_n,

    input  wire rep_valid,
    output reg  rep_ready,

    input  wire rep_pattern,
    input  wire [15:0] rep_cycles,
    input  wire [15:0] rep_conflicts,
    input  wire [15:0] rep_g0,
    input  wire [15:0] rep_g1,
    input  wire [15:0] rep_b0,
    input  wire [15:0] rep_b1,

    output wire uart_tx,
    output wire busy
);

localparam S_IDLE = 2'd0;
localparam S_SEND = 2'd1;
localparam S_WAIT = 2'd2;
localparam S_ACK  = 2'd3;

localparam MSG_LEN = 7'd50;

reg [1:0] state;
reg [6:0] idx;

reg pat_l;
reg [15:0] cyc_l;
reg [15:0] conf_l;
reg [15:0] g0_l;
reg [15:0] g1_l;
reg [15:0] b0_l;
reg [15:0] b1_l;

reg tx_start;
reg [7:0] tx_data;
wire tx_busy;
wire tx_done;

assign busy = (state != S_IDLE);

function [7:0] hexchar;
    input [3:0] nib;
    begin
        case (nib)
            4'h0: hexchar = "0";
            4'h1: hexchar = "1";
            4'h2: hexchar = "2";
            4'h3: hexchar = "3";
            4'h4: hexchar = "4";
            4'h5: hexchar = "5";
            4'h6: hexchar = "6";
            4'h7: hexchar = "7";
            4'h8: hexchar = "8";
            4'h9: hexchar = "9";
            4'hA: hexchar = "A";
            4'hB: hexchar = "B";
            4'hC: hexchar = "C";
            4'hD: hexchar = "D";
            4'hE: hexchar = "E";
            4'hF: hexchar = "F";
        endcase
    end
endfunction

reg [7:0] next_byte;

always @(*) begin
    case (idx)
         0: next_byte = "P";
         1: next_byte = pat_l ? "1" : "0";
         2: next_byte = " ";
         3: next_byte = "C";
         4: next_byte = "=";
         5: next_byte = hexchar(cyc_l[15:12]);
         6: next_byte = hexchar(cyc_l[11:8]);
         7: next_byte = hexchar(cyc_l[7:4]);
         8: next_byte = hexchar(cyc_l[3:0]);
         9: next_byte = " ";
        10: next_byte = "F";
        11: next_byte = "=";
        12: next_byte = hexchar(conf_l[15:12]);
        13: next_byte = hexchar(conf_l[11:8]);
        14: next_byte = hexchar(conf_l[7:4]);
        15: next_byte = hexchar(conf_l[3:0]);
        16: next_byte = " ";
        17: next_byte = "G";
        18: next_byte = "0";
        19: next_byte = "=";
        20: next_byte = hexchar(g0_l[15:12]);
        21: next_byte = hexchar(g0_l[11:8]);
        22: next_byte = hexchar(g0_l[7:4]);
        23: next_byte = hexchar(g0_l[3:0]);
        24: next_byte = " ";
        25: next_byte = "G";
        26: next_byte = "1";
        27: next_byte = "=";
        28: next_byte = hexchar(g1_l[15:12]);
        29: next_byte = hexchar(g1_l[11:8]);
        30: next_byte = hexchar(g1_l[7:4]);
        31: next_byte = hexchar(g1_l[3:0]);
        32: next_byte = " ";
        33: next_byte = "B";
        34: next_byte = "0";
        35: next_byte = "=";
        36: next_byte = hexchar(b0_l[15:12]);
        37: next_byte = hexchar(b0_l[11:8]);
        38: next_byte = hexchar(b0_l[7:4]);
        39: next_byte = hexchar(b0_l[3:0]);
        40: next_byte = " ";
        41: next_byte = "B";
        42: next_byte = "1";
        43: next_byte = "=";
        44: next_byte = hexchar(b1_l[15:12]);
        45: next_byte = hexchar(b1_l[11:8]);
        46: next_byte = hexchar(b1_l[7:4]);
        47: next_byte = hexchar(b1_l[3:0]);
        48: next_byte = 8'h0D;
        49: next_byte = 8'h0A;
        default: next_byte = 8'h20;
    endcase
end

uart_tx #(
    .CLKS_PER_BIT(CLK_FREQ_HZ / BAUD)
) u_tx (
    .clk      (clk),
    .rst_n    (rst_n),
    .tx_start (tx_start),
    .tx_data  (tx_data),
    .tx       (uart_tx),
    .tx_busy  (tx_busy),
    .tx_done  (tx_done)
);

always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        state     <= S_IDLE;
        idx       <= 7'd0;
        tx_start  <= 1'b0;
        tx_data   <= 8'h00;
        rep_ready <= 1'b0;

        pat_l     <= 1'b0;
        cyc_l     <= 16'd0;
        conf_l    <= 16'd0;
        g0_l      <= 16'd0;
        g1_l      <= 16'd0;
        b0_l      <= 16'd0;
        b1_l      <= 16'd0;
    end else begin
        tx_start  <= 1'b0;
        rep_ready <= 1'b0;

        case (state)
            S_IDLE: begin
                if (rep_valid) begin
                    pat_l  <= rep_pattern;
                    cyc_l  <= rep_cycles;
                    conf_l <= rep_conflicts;
                    g0_l   <= rep_g0;
                    g1_l   <= rep_g1;
                    b0_l   <= rep_b0;
                    b1_l   <= rep_b1;
                    idx    <= 7'd0;
                    state  <= S_SEND;
                end
            end

            S_SEND: begin
                if (!tx_busy) begin
                    tx_data  <= next_byte;
                    tx_start <= 1'b1;
                    state    <= S_WAIT;
                end
            end

            S_WAIT: begin
                if (tx_done) begin
                    if (idx == MSG_LEN - 1)
                        state <= S_ACK;
                    else begin
                        idx   <= idx + 7'd1;
                        state <= S_SEND;
                    end
                end
            end

            S_ACK: begin
                rep_ready <= 1'b1;
                state     <= S_IDLE;
            end

            default: begin
                state <= S_IDLE;
            end
        endcase
    end
end

endmodule