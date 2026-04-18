set_device -name GW1N-1P5C GW1N-UV1P5QN48XFC7/I6
set_option -top_module watchdog_top
set_option -output_base_name watchdog_uart

add_file Watchdog_UART/src/watchdog_top.v
add_file Watchdog_UART/src/watchdog_core.v
add_file Watchdog_UART/src/config_reg.v
add_file Watchdog_UART/src/sync_debounce.v
add_file Watchdog_UART/src/uart_rx.v
add_file Watchdog_UART/src/uart_tx.v

add_file Watchdog_UART/src/watchdog.cst
add_file Watchdog_UART/src/watchdog.sdc

run all
