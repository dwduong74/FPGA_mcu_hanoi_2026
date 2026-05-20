
//cm3_spiflash_demo
module cm3_spiflash_demo
(
  sys_clk,
  reset_n,
  uart0_rxd,
  uart0_txd,
  O_flash_cs_n,
  IO_flash_do,
  IO_flash_di,
  O_flash_ck
);

input sys_clk;
input reset_n;

//UART0
input uart0_rxd;
output uart0_txd;

//SPI-Flash
inout O_flash_cs_n;
inout IO_flash_do;
inout IO_flash_di;
inout O_flash_ck;


//APB2 master
wire        master_pclk ;
wire        master_pst ;
wire        master_penable ;
wire [7:0]  master_paddr ;
wire        master_pwrite ;
wire [31:0] master_pwdata ;
wire        master_psel1 ;
wire[31:0]  master_prdata1 ;
wire        master_pready1 ;
wire        master_pslverr1 ;

assign master_pslverr1  = 1'b0; //no error


//Gowin_EMPU_Top instantiation
Gowin_EMPU_Top  u_Gowin_EMPU_Top
(
  .sys_clk(sys_clk),
  .uart0_rxd(uart0_rxd),
  .uart0_txd(uart0_txd),
  .master_pclk(master_pclk),
  .master_prst(master_prst),
  .master_penable(master_penable),
  .master_paddr(master_paddr),
  .master_pwrite(master_pwrite),
  .master_pwdata(master_pwdata),
  .master_pstrb(),
  .master_pprot(),
  .master_psel1(master_psel1),
  .master_prdata1(master_prdata1),
  .master_pready1(master_pready1),
  .master_pslverr1(master_pslverr1),
  .reset_n(reset_n)
);


//Gowin_APB_SPI_Nor_Flash_gowin_top instantiation
Gowin_APB_SPI_Nor_Flash_gowin_top u_Gowin_APB_SPI_Nor_Flash_gowin_top
(
  .io_spi_csn   (O_flash_cs_n),
  .io_spi_miso  (IO_flash_do),
  .io_spi_mosi  (IO_flash_di),
  .io_spi_clk   (O_flash_ck),
  .pclk         (master_pclk),
  .presetn      (master_prst),
  .paddr        (master_paddr),
  .penable      (master_penable),
  .prdata       (master_prdata1),
  .pready       (master_pready1),
  .psel         (master_psel1),
  .pwdata       (master_pwdata),
  .pwrite       (master_pwrite)
); 

endmodule