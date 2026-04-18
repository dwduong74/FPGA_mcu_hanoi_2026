// Timing Constraints — Kiwi 1P5 Board
// Clock: 27 MHz (period = 37.037 ns)
create_clock -name clk -period 37.037 -waveform {0 18.518} [get_ports {clk}]
