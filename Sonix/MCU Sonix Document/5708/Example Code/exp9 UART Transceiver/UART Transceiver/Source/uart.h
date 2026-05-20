#ifndef _UART_
#define _UART_
#include "mcu.h"
#define    UART_BUFF_LENGTH     32
void uart0_config(void);
void uart_rx_time_dec();
uint8_t uart_rx_data_verify();
void uart_process();
#endif