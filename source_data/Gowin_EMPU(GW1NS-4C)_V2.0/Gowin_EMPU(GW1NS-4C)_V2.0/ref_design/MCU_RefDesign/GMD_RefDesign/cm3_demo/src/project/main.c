/*
 *********************************************************************************************
 * @file      main.c
 * @author    GowinSemicoductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Main function.
 *********************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "demo.h"


/* Definitions ---------------------------------------------------------------*/

//Application entry function
int main(void)
{
#if AHB2_MASTER_DEMO
	ahb2_master_demo();
#endif

#if APB2_MASTER_DEMO
	apb2_master_demo();
#endif

#if DMM_DEMO
	dmm_demo();
#endif

#if GPIO_IN_INTR_DEMO
	gpio_in_intr_demo();
#endif

#if HYPER_RAM_DEMO
	hyper_ram_demo();
#endif

#if I2C_MASTER_DEMO
	i2c_master_demo();
#endif

#if INTERRUPT_DEMO
	interrupt_demo();
#endif

#if KEYSCAN_DEMO
	keyscan_demo();
#endif

#if LED_DEMO
	led_demo();
#endif

#if PRINTF_DEMO
	printf_demo();
#endif

#if PSRAM_DEMO
	psram_demo();
#endif

#if RTC_DEMO
	rtc_demo();
#endif

#if SPIFLASH_DEMO
	spiflash_demo();
#endif

#if SPI_MASTER_DEMO
	spi_master_demo();
#endif

#if TIMER_DEMO
	timer_demo();
#endif

#if UART_RX_DEMO
	uart_rx_demo();
#endif

#if UART_RX_INTR_DEMO
	uart_rx_intr_demo();
#endif

#if WDOG_DEMO
	wdog_demo();
#endif

	return 0;
}
