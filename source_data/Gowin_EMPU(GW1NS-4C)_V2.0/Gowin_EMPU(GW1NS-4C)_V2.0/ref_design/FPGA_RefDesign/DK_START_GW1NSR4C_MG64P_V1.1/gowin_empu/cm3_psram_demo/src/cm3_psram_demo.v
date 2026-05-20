
//cm3_psram_demo
module cm3_psram_demo
(
  sys_clk,
  uart0_rxd,
  uart0_txd,
  reset_n,
  init_calib,   //Initialized flag
  O_psram_ck,
  O_psram_ck_n,
  O_psram_cs_n,
  O_psram_reset_n,
  IO_psram_dq,
  IO_psram_rwds
);

input sys_clk;
input reset_n;

//UART0
input uart0_rxd;
output uart0_txd;

//PSRAM
output init_calib;
output [1:0] O_psram_ck;
output [1:0] O_psram_ck_n;
output [1:0] O_psram_cs_n;
output [1:0] O_psram_reset_n;
inout [15:0] IO_psram_dq;
inout [1:0] IO_psram_rwds;


//AHB2 master
wire master_hclk;
wire master_hrst;
wire master_hsel;
wire [31:0] master_haddr;
wire [1:0] master_htrans;
wire master_hwrite;
wire [2:0] master_hsize;
wire [2:0] master_hburst;
wire [3:0] master_hprot;
wire [1:0] master_hmemattr;
wire master_hexreq;
wire [3:0] master_hmaster;
wire [31:0] master_hwdata;
wire master_hmastlock;
wire master_hreadymux;
wire master_hauser;
wire [3:0] master_hwuser;
wire [31:0] master_hrdata;
wire master_hreadyout;
wire master_hresp;

wire clk_100m;
wire mclk;              //mcu clock = 50MHz
wire psram_memory_clk;  //psram memory clock = 100MHz
wire psram_base_clk;    //psram base clock = 50MHz

//Gowin_PLLVR instantiation
Gowin_PLLVR u_Gowin_PLLVR
(
  .clkout(clk_100m),
  .clkin(sys_clk)
);

assign psram_memory_clk = clk_100m;
assign psram_base_clk = sys_clk;
assign mclk = clk_100m;


//Gowin_AHB_PSRAM_Top instantiation
Gowin_AHB_PSRAM_Top u_Gowin_AHB_PSRAM_Top
(
  .AHB_HRDATA(master_hrdata),
  .AHB_HREADY(master_hreadyout),
  .AHB_HRESP(master_hresp),
  .AHB_HTRANS(master_htrans),
  .AHB_HBURST(master_hburst),
  .AHB_HPROT(master_hprot),
  .AHB_HSIZE(master_hsize),
  .AHB_HWRITE(master_hwrite),
  .AHB_HMASTLOCK(master_hmastlock),
  .AHB_HMASTER(master_hmaster),
  .AHB_HADDR(master_haddr),
  .AHB_HWDATA(master_hwdata),
  .AHB_HSEL(master_hsel),
  .AHB_HCLK(master_hclk),
  .AHB_HRESETn(master_hrst),
  .psram_base_clk(psram_base_clk),     //50MHz
  .psram_memory_clk(psram_memory_clk), //100MHz
  .led_init(init_calib),
  .O_psram_ck(O_psram_ck),
  .O_psram_ck_n(O_psram_ck_n),
  .O_psram_cs_n(O_psram_cs_n),
  .O_psram_reset_n(O_psram_reset_n),
  .IO_psram_dq(IO_psram_dq),
  .IO_psram_rwds(IO_psram_rwds)
);


//Gowin_EMPU_Top instantiation
Gowin_EMPU_Top u_Gowin_EMPU_Top
(
  .sys_clk(mclk),
  .reset_n(reset_n),
  .uart0_rxd(uart0_rxd),
  .uart0_txd(uart0_txd),
  .master_hclk(master_hclk),
  .master_hrst(master_hrst),
  .master_hsel(master_hsel),
  .master_haddr(master_haddr),
  .master_htrans(master_htrans),
  .master_hwrite(master_hwrite),
  .master_hsize(master_hsize),
  .master_hburst(master_hburst),
  .master_hprot(master_hprot),
  .master_hmemattr(master_hmemattr),
  .master_hexreq(master_hexreq),
  .master_hmaster(master_hmaster),
  .master_hwdata(master_hwdata),
  .master_hmastlock(master_hmastlock),
  .master_hreadymux(master_hreadymux),
  .master_hauser(master_hauser),
  .master_hwuser(master_hwuser),
  .master_hrdata(master_hrdata),
  .master_hreadyout(master_hreadyout),
  .master_hresp(master_hresp),
  .master_hexresp(1'b0),
  .master_hruser(3'b000)
);

endmodule