
//cm3_demo
module cm3_demo
(
  sys_clk,
  reset_n,
  led,
  key,
  uart0_rxd,
  uart0_txd
);

input sys_clk;
input reset_n;

//LED
inout led;

//KEY
inout key;

//UART0
input uart0_rxd;
output uart0_txd;


wire mclk;      //mcu clock
wire rtc_clk;   //rtc clock

//Gowin_PLLVR instantiation
Gowin_PLLVR u_Gowin_PLLVR
(
  .clkout(mclk),
  .clkoutd(rtc_clk),
  .clkin(sys_clk)
);


//Gowin_EMPU_Top instantiation
Gowin_EMPU_Top u_Gowin_EMPU_Top
(
  .sys_clk(mclk),
  .reset_n(reset_n),
  .rtc_src_clk(rtc_clk),
  .gpio({key, led}),
  .uart0_rxd(uart0_rxd),
  .uart0_txd(uart0_txd)
);

endmodule