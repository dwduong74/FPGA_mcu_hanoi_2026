
//cm3_i2c_demo
module cm3_i2c_demo
(
  sys_clk,
  reset_n,
  uart0_rxd,
  uart0_txd,
  scl,
  sda
);

input sys_clk;
input reset_n;

//UART0
input uart0_rxd;
output uart0_txd;

//I2C
inout scl;
inout sda;


wire mclk;      //mcu clock

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
  .scl(scl),
  .sda(sda)
);

endmodule