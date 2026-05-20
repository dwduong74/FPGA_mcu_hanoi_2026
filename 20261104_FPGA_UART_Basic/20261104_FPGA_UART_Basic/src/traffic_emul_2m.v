module traffic_emul_2m #(
    parameter N_REQS = 16'd32
)(
    input  wire clk,
    input  wire rst_n,

    output reg  rep_valid,
    input  wire rep_ready,
    output reg  rep_pattern,
    output reg  [15:0] rep_cycles,
    output reg  [15:0] rep_conflicts,
    output reg  [15:0] rep_g0,
    output reg  [15:0] rep_g1,
    output reg  [15:0] rep_b0,
    output reg  [15:0] rep_b1,

    output wire bank0_we,
    output wire [6:0] bank0_addr,
    output wire [31:0] bank0_wdata,

    output wire bank1_we,
    output wire [6:0] bank1_addr,
    output wire [31:0] bank1_wdata,

    output wire running,
    output wire done
);

localparam ST_P0_INIT = 3'd0;
localparam ST_P0_RUN  = 3'd1;
localparam ST_P0_WAIT = 3'd2;
localparam ST_P1_INIT = 3'd3;
localparam ST_P1_RUN  = 3'd4;
localparam ST_P1_WAIT = 3'd5;
localparam ST_DONE    = 3'd6;

reg [2:0] state;

assign running = (state == ST_P0_RUN) || (state == ST_P1_RUN);
assign done    = (state == ST_DONE);

reg [15:0] m0_sent;
reg [15:0] m1_sent;

reg [15:0] cyc_ctr;
reg [15:0] conf_ctr;
reg [15:0] g0_ctr;
reg [15:0] g1_ctr;
reg [15:0] b0_ctr;
reg [15:0] b1_ctr;

reg rr_bank0;
reg rr_bank1;

wire pattern1 = (state == ST_P1_INIT) || (state == ST_P1_RUN) || (state == ST_P1_WAIT);

wire req0_valid = running && (m0_sent < N_REQS);
wire req1_valid = running && (m1_sent < N_REQS);

// 8-bit internal word address
// addr[0] = bank select
// addr[7:1] = row in bank
wire [7:0] req0_addr = {m0_sent[6:0], 1'b0}; // even word addresses

wire [7:0] req1_addr = pattern1
                     ? ({m1_sent[6:0], 1'b0} + 8'd1)   // odd words => bank 1
                     : (8'd128 + {m1_sent[6:0], 1'b0}); // even words => bank 0

wire [31:0] req0_data = {8'hA0, 7'd0, pattern1, m0_sent[15:0]};
wire [31:0] req1_data = {8'hB0, 7'd0, pattern1, m1_sent[15:0]};

wire req0_bank = req0_addr[0];
wire req1_bank = req1_addr[0];

wire [6:0] req0_row = req0_addr[7:1];
wire [6:0] req1_row = req1_addr[7:1];

wire same_bank_conflict = req0_valid && req1_valid && (req0_bank == req1_bank);

reg gnt0_r;
reg gnt1_r;

reg bank0_we_r;
reg [6:0] bank0_addr_r;
reg [31:0] bank0_wdata_r;

reg bank1_we_r;
reg [6:0] bank1_addr_r;
reg [31:0] bank1_wdata_r;

always @(*) begin
    gnt0_r       = 1'b0;
    gnt1_r       = 1'b0;

    bank0_we_r   = 1'b0;
    bank0_addr_r = 7'd0;
    bank0_wdata_r= 32'd0;

    bank1_we_r   = 1'b0;
    bank1_addr_r = 7'd0;
    bank1_wdata_r= 32'd0;

    if (req0_valid && req1_valid && (req0_bank == req1_bank)) begin
        if (req0_bank == 1'b0) begin
            if (rr_bank0 == 1'b0) begin
                gnt0_r        = 1'b1;
                bank0_we_r    = 1'b1;
                bank0_addr_r  = req0_row;
                bank0_wdata_r = req0_data;
            end else begin
                gnt1_r        = 1'b1;
                bank0_we_r    = 1'b1;
                bank0_addr_r  = req1_row;
                bank0_wdata_r = req1_data;
            end
        end else begin
            if (rr_bank1 == 1'b0) begin
                gnt0_r        = 1'b1;
                bank1_we_r    = 1'b1;
                bank1_addr_r  = req0_row;
                bank1_wdata_r = req0_data;
            end else begin
                gnt1_r        = 1'b1;
                bank1_we_r    = 1'b1;
                bank1_addr_r  = req1_row;
                bank1_wdata_r = req1_data;
            end
        end
    end else begin
        if (req0_valid) begin
            gnt0_r = 1'b1;
            if (req0_bank == 1'b0) begin
                bank0_we_r    = 1'b1;
                bank0_addr_r  = req0_row;
                bank0_wdata_r = req0_data;
            end else begin
                bank1_we_r    = 1'b1;
                bank1_addr_r  = req0_row;
                bank1_wdata_r = req0_data;
            end
        end

        if (req1_valid) begin
            gnt1_r = 1'b1;
            if (req1_bank == 1'b0) begin
                bank0_we_r    = 1'b1;
                bank0_addr_r  = req1_row;
                bank0_wdata_r = req1_data;
            end else begin
                bank1_we_r    = 1'b1;
                bank1_addr_r  = req1_row;
                bank1_wdata_r = req1_data;
            end
        end
    end
end

assign bank0_we    = bank0_we_r;
assign bank0_addr  = bank0_addr_r;
assign bank0_wdata = bank0_wdata_r;

assign bank1_we    = bank1_we_r;
assign bank1_addr  = bank1_addr_r;
assign bank1_wdata = bank1_wdata_r;

always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        state         <= ST_P0_INIT;

        m0_sent       <= 16'd0;
        m1_sent       <= 16'd0;

        cyc_ctr       <= 16'd0;
        conf_ctr      <= 16'd0;
        g0_ctr        <= 16'd0;
        g1_ctr        <= 16'd0;
        b0_ctr        <= 16'd0;
        b1_ctr        <= 16'd0;

        rr_bank0      <= 1'b0;
        rr_bank1      <= 1'b0;

        rep_valid     <= 1'b0;
        rep_pattern   <= 1'b0;
        rep_cycles    <= 16'd0;
        rep_conflicts <= 16'd0;
        rep_g0        <= 16'd0;
        rep_g1        <= 16'd0;
        rep_b0        <= 16'd0;
        rep_b1        <= 16'd0;
    end else begin
        case (state)
            ST_P0_INIT,
            ST_P1_INIT: begin
                m0_sent  <= 16'd0;
                m1_sent  <= 16'd0;

                cyc_ctr  <= 16'd0;
                conf_ctr <= 16'd0;
                g0_ctr   <= 16'd0;
                g1_ctr   <= 16'd0;
                b0_ctr   <= 16'd0;
                b1_ctr   <= 16'd0;

                rr_bank0 <= 1'b0;
                rr_bank1 <= 1'b0;

                rep_valid <= 1'b0;

                if (state == ST_P0_INIT)
                    state <= ST_P0_RUN;
                else
                    state <= ST_P1_RUN;
            end

            ST_P0_RUN,
            ST_P1_RUN: begin
                cyc_ctr <= cyc_ctr + 16'd1;

                if (same_bank_conflict)
                    conf_ctr <= conf_ctr + 16'd1;

                if (gnt0_r)
                    g0_ctr <= g0_ctr + 16'd1;

                if (gnt1_r)
                    g1_ctr <= g1_ctr + 16'd1;

                if (bank0_we_r)
                    b0_ctr <= b0_ctr + 16'd1;

                if (bank1_we_r)
                    b1_ctr <= b1_ctr + 16'd1;

                if (gnt0_r)
                    m0_sent <= m0_sent + 16'd1;

                if (gnt1_r)
                    m1_sent <= m1_sent + 16'd1;

                if (same_bank_conflict) begin
                    if (req0_bank == 1'b0)
                        rr_bank0 <= ~rr_bank0;
                    else
                        rr_bank1 <= ~rr_bank1;
                end

                if (((m0_sent + (gnt0_r ? 16'd1 : 16'd0)) == N_REQS) &&
                    ((m1_sent + (gnt1_r ? 16'd1 : 16'd0)) == N_REQS)) begin

                    rep_pattern   <= pattern1;
                    rep_cycles    <= cyc_ctr + 16'd1;
                    rep_conflicts <= conf_ctr + (same_bank_conflict ? 16'd1 : 16'd0);
                    rep_g0        <= g0_ctr + (gnt0_r ? 16'd1 : 16'd0);
                    rep_g1        <= g1_ctr + (gnt1_r ? 16'd1 : 16'd0);
                    rep_b0        <= b0_ctr + (bank0_we_r ? 16'd1 : 16'd0);
                    rep_b1        <= b1_ctr + (bank1_we_r ? 16'd1 : 16'd0);
                    rep_valid     <= 1'b1;

                    if (pattern1)
                        state <= ST_P1_WAIT;
                    else
                        state <= ST_P0_WAIT;
                end
            end

            ST_P0_WAIT: begin
                if (rep_ready) begin
                    rep_valid <= 1'b0;
                    state <= ST_P1_INIT;
                end
            end

            ST_P1_WAIT: begin
                if (rep_ready) begin
                    rep_valid <= 1'b0;
                    state <= ST_DONE;
                end
            end

            ST_DONE: begin
                rep_valid <= 1'b0;
            end

            default: begin
                state <= ST_P0_INIT;
            end
        endcase
    end
end

endmodule