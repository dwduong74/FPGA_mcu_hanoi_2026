
//cm3_apb2_demo
module cm3_apb2_demo
(
  sys_clk,
  reset_n,
  uart0_rxd,
  uart0_txd
);

input sys_clk;
input reset_n;

//UART0
input uart0_rxd;
output uart0_txd;


//APB2 master
wire        master_pclk;
wire        master_prst;
wire        master_penable;
wire [7:0]  master_paddr;
wire        master_pwrite;
wire [31:0] master_pwdata;
wire [3:0]  master_pstrb;
wire [2:0]  master_pprot;
wire        master_psel1;
wire [31:0] master_prdata1;
wire        master_pready1;

//Gowin_APB2_Multiple instantiation
Gowin_APB2_Multiple u_Gowin_APB2_Multiple
(
  .pclk(master_pclk),
  .presetn(master_prst),
  .psel(master_psel1),
  .penable(master_penable),
  .pwrite(master_pwrite),
  .paddr({4'b0000,master_paddr[7:2]}),
  .pwdata(master_pwdata),
  .prdata(master_prdata1),
  .pready(master_pready1)
);


wire mclk;    //mcu clock

//Gowin_PLLVR instantiation
Gowin_PLLVR u_Gowin_PLLVR
(
  .clkout(mclk),
  .clkin(sys_clk)
);


//Gowin_EMPU_Top instantiation
Gowin_EMPU_Top u_Gowin_EMPU_Top
(
  .sys_clk(mclk),
  .reset_n(reset_n),
  .uart0_rxd(uart0_rxd),
  .uart0_txd(uart0_txd),
  .master_pclk(master_pclk),
  .master_prst(master_prst),
  .master_penable(master_penable),
  .master_paddr(master_paddr),
  .master_pwrite(master_pwrite),
  .master_pwdata(master_pwdata),
  .master_pstrb(master_pstrb),
  .master_pprot(master_pprot),
  .master_psel1(master_psel1),
  .master_prdata1(master_prdata1),
  .master_pready1(master_pready1),
  .master_pslverr1(1'b0)
);

endmodule