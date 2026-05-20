module bram_bank_simple #(
    parameter AW = 7
)(
    input  wire              clk,
    input  wire              we,
    input  wire [AW-1:0]     waddr,
    input  wire [31:0]       wdata,
    input  wire [AW-1:0]     raddr,
    output reg  [31:0]       rdata
);

reg [31:0] mem [0:(1<<AW)-1];
integer i;

initial begin
    for (i = 0; i < (1<<AW); i = i + 1)
        mem[i] = 32'd0;
end

always @(posedge clk) begin
    if (we)
        mem[waddr] <= wdata;
    rdata <= mem[raddr];
end

endmodule