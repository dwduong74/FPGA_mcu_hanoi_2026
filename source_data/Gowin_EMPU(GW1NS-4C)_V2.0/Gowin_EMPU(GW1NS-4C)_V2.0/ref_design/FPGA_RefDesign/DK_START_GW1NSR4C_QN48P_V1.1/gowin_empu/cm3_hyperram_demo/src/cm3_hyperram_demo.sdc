//Copyright (C)2014-2024 GOWIN Semiconductor Corporation.
//All rights reserved.
//File Title: Timing Constraints file
//Tool Version: V1.9.9.01 (64-bit) 
//Created Time: 2024-02-19 10:52:10
create_clock -name hyperram_mem_clk -period 10 -waveform {0 5} [get_pins {u_Gowin_PLLVR/pllvr_inst/CLKOUT}]
set_clock_groups -exclusive -group [get_clocks {hyperram_mem_clk}]
