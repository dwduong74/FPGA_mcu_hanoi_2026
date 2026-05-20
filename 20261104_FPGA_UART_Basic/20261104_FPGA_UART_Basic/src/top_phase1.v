module top_phase1(
    input  wire sys_clk,   // 27 MHz, pin 4
    output wire uart_tx,   // inferred: pin 34 on KIWI 1P5 standalone board
    output wire led1,      // pin 27, active-low
    output wire led2       // pin 28, active-low
);

reg [15:0] por_cnt = 16'd0;
wire rst_n = &por_cnt;

always @(posedge sys_clk) begin
    if (!rst_n)
        por_cnt <= por_cnt + 1'b1;
end

wire rep_valid;
wire rep_ready;
wire rep_pattern;
wire [15:0] rep_cycles;
wire [15:0] rep_conflicts;
wire [15:0] rep_g0;
wire [15:0] rep_g1;
wire [15:0] rep_b0;
wire [15:0] rep_b1;

wire bank0_we;
wire [6:0] bank0_addr;
wire [31:0] bank0_wdata;

wire bank1_we;
wire [6:0] bank1_addr;
wire [31:0] bank1_wdata;

wire emu_running;
wire emu_done;
wire uart_busy;

wire [31:0] unused_rdata0;
wire [31:0] unused_rdata1;

traffic_emul_2m #(
    .N_REQS(16'd32)
) u_emul (
    .clk           (sys_clk),
    .rst_n         (rst_n),

    .rep_valid     (rep_valid),
    .rep_ready     (rep_ready),
    .rep_pattern   (rep_pattern),
    .rep_cycles    (rep_cycles),
    .rep_conflicts (rep_conflicts),
    .rep_g0        (rep_g0),
    .rep_g1        (rep_g1),
    .rep_b0        (rep_b0),
    .rep_b1        (rep_b1),

    .bank0_we      (bank0_we),
    .bank0_addr    (bank0_addr),
    .bank0_wdata   (bank0_wdata),

    .bank1_we      (bank1_we),
    .bank1_addr    (bank1_addr),
    .bank1_wdata   (bank1_wdata),

    .running       (emu_running),
    .done          (emu_done)
);

bram_bank_simple #(
    .AW(7)
) u_bank0 (
    .clk   (sys_clk),
    .we    (bank0_we),
    .waddr (bank0_addr),
    .wdata (bank0_wdata),
    .raddr (7'd0),
    .rdata (unused_rdata0)
);

bram_bank_simple #(
    .AW(7)
) u_bank1 (
    .clk   (sys_clk),
    .we    (bank1_we),
    .waddr (bank1_addr),
    .wdata (bank1_wdata),
    .raddr (7'd0),
    .rdata (unused_rdata1)
);

uart_reporter #(
    .CLK_FREQ_HZ(27000000),
    .BAUD       (115200)
) u_report (
    .clk           (sys_clk),
    .rst_n         (rst_n),
    .rep_valid     (rep_valid),
    .rep_ready     (rep_ready),
    .rep_pattern   (rep_pattern),
    .rep_cycles    (rep_cycles),
    .rep_conflicts (rep_conflicts),
    .rep_g0        (rep_g0),
    .rep_g1        (rep_g1),
    .rep_b0        (rep_b0),
    .rep_b1        (rep_b1),
    .uart_tx       (uart_tx),
    .busy          (uart_busy)
);

assign led1 = ~(emu_running | uart_busy); // on while test/logging
assign led2 = ~emu_done;                  // on when all done

endmodule