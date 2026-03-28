# Gowin Build Script
set_device GW1N-UV1P5QN48XFC7/I6 -device_version C
add_file src/watchdog_top.v
add_file src/watchdog_core.v
add_file src/config_reg.v
add_file src/uart_rx.v
add_file src/uart_tx.v
add_file src/watchdog.cst

set_option -top_module watchdog_top
set_option -output_base_name watchdog_uart

run all
