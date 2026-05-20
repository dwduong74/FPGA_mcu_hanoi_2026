/*
 **********************************************************************************************
 * @file      demo.h
 * @author    GowinSemicoductor
 * @device    Gowin_EMPU(GW1NS-4C)
 * @brief     Macro definitions
 **********************************************************************************************
 */

#ifndef __DEMO_H_
#define __DEMO_H_

/* Definitions ---------------------------------------------------------------*/

#define AHB2_MASTER_DEMO	0	//ahb2_master
#define APB2_MASTER_DEMO	0	//apb2_master
#define	DMM_DEMO			0	//dmm
#define GPIO_IN_INTR_DEMO	0	//gpio_in_intr
#define HYPER_RAM_DEMO		0	//hyper ram
#define I2C_MASTER_DEMO		0	//i2c_master
#define INTERRUPT_DEMO		0	//interrupt
#define KEYSCAN_DEMO		0	//keyscan
#define LED_DEMO			0	//led
#define PRINTF_DEMO			0	//printf
#define PSRAM_DEMO			0	//psram
#define RTC_DEMO			0	//rtc
#define SPIFLASH_DEMO		0	//spi_flash
#define	SPI_MASTER_DEMO		0	//spi_master
#define	TIMER_DEMO			1	//timer
#define UART_RX_DEMO		0	//uart_rx
#define	UART_RX_INTR_DEMO	0	//uart_rx_intr
#define WDOG_DEMO			0	//watchdog


#if AHB2_MASTER_DEMO
#define CM3_100MHZ_FREQ		//100MHz
int ahb2_master_demo(void);
#endif

#if APB2_MASTER_DEMO
#define CM3_100MHZ_FREQ		//100MHz
int apb2_master_demo(void);
#endif

#if DMM_DEMO
int dmm_demo(void);
#endif

#if GPIO_IN_INTR_DEMO
int gpio_in_intr_demo(void);
#endif

#if HYPER_RAM_DEMO
#define CM3_100MHZ_FREQ		//100MHz
int hyper_ram_demo(void);
#endif

#if I2C_MASTER_DEMO
#define CM3_100MHZ_FREQ		//100MHz
int i2c_master_demo(void);
#endif

#if INTERRUPT_DEMO
int interrupt_demo(void);
#endif

#if KEYSCAN_DEMO
int keyscan_demo(void);
#endif

#if LED_DEMO
int led_demo(void);
#endif

#if PRINTF_DEMO
int printf_demo(void);
#endif

#if PSRAM_DEMO
#define CM3_100MHZ_FREQ		//100MHz
int psram_demo(void);
#endif

#if RTC_DEMO
int rtc_demo(void);
#endif

#if SPIFLASH_DEMO
#define CM3_50MHZ_FREQ		//50MHz
int spiflash_demo(void);
#endif

#if SPI_MASTER_DEMO
#define CM3_100MHZ_FREQ		//100MHz
int spi_master_demo(void);
#endif

#if TIMER_DEMO
int timer_demo(void);
#endif

#if UART_RX_DEMO
int uart_rx_demo(void);
#endif

#if UART_RX_INTR_DEMO
int uart_rx_intr_demo(void);
#endif

#if WDOG_DEMO
int wdog_demo(void);
#endif

#endif
