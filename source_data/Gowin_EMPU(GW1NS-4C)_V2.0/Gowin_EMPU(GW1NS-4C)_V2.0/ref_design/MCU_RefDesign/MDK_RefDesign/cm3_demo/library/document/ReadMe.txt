-------------------------------------------------------------------------------------------------------
Gowin_EMPU(GW1NS-4C) Software Programming Library Read Me File
-------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------
IDE
    tested version:
      . ARM Keil MDK V5.26
      . GMD V1.2
-------------------------------------------------------------------------------------------------------
library
    /document                                              //Documents
        /ReadMe.txt                                        //Read me file
        /ReleaseNote.txt                                   //Release note file
    /libraries                                             //Libraries
        /drivers                                           //Drivers
            /inc                                           //Drivers
                /gw1ns4c_gpio.h                            //GPIO driver
                /gw1ns4c_misc.h                            //Miscellaneous
                /gw1ns4c_timer.h                           //Timer driver
                /gw1ns4c_uart.h                            //UART driver
                /gw1ns4c_wdog.h                            //Watch Dog driver
                /gw1ns4c_rtc.h                             //RTC driver
                /gw1ns4c_syscon.h                          //System controller driver
                /gw1ns4c_i2c.h                             //I2C driver
                /gw1ns4c_spi.h                             //SPI driver
            /src                                           //Drivers
                /gw1ns4c_gpio.c                            //GPIO driver
                /gw1ns4c_misc.c                            //Miscellaneous
                /gw1ns4c_timer.c                           //Timer driver
                /gw1ns4c_uart.c                            //UART driver
                /gw1ns4c_wdog.c                            //Watch Dog driver
                /gw1ns4c_rtc.c                             //RTC driver
                /gw1ns4c_syscon.c                          //System controller driver
                /gw1ns4c_i2c.c                             //I2C driver
                /gw1ns4c_spi.c                             //SPI driver
        /cmsis                                             //Core and Device support
            /cm3                                           //Cortex-M3 support
                /core_support                              //Core support
                    /mdk                                   //MDK IDE
                    /gmd                                   //GMD IDE
                /device_support                            //Device support
                    /startup                               //Startup
                        /mdk                               //MDK IDE
                            /startup_gw1ns4c.s             //Startup
                        /gmd                               //GMD IDE
                            /linker                        //Flash linker
                                /gw1ns4c_flash.ld          //Flash linker
                            /startup_gw1ns4c.S             //Startup
                    /gw1ns4c.h                             //Registers and addresses definitions
                    /system_gw1ns4c.h                      //Initializes system
                    /system_gw1ns4c.c                      //Initializes system
                    /gw1ns4c_conf.h                        //Configurations
    /middlewares                                           //Middlewares
        /3rd_party                                         //The third-party application libraries
            /freertos                                      //RTOS FreeRTOS
            /rtthread_nano                                 //RTOS RT-Thread Nano version
            /ucos_iii                                      //RTOS uC/OS-III
        /delay                                             //Delay functions
            /delay.h                                       //Delay functions
            /delay.c                                       //Delay functions
        /hyper_ram                                         //HyperRAM functions
            /hyper_ram.h                                   //HyperRAM functions
            /hyper_ram.c                                   //HyperRAM functions
        /psram                                             //PSRAM functions
            /psram.h                                       //PSRAM functions
            /psram.c                                       //PSRAM functions
        /spi_flash                                         //SPI-Flash functions
            /spi_flash.h                                   //SPI-Flash functions
            /spi_flash.c                                   //SPI-Flash functions
        /dmm                                               //Dynamic memory management
            /malloc.h                                      //Dynamic memory management
            /malloc.c                                      //Dynamic memory management
        /gpio                                              //GPIO functions
            /gpio.h                                        //GPIO functions
            /gpio.c                                        //GPIO functions
        /uart                                              //UART functions
            /gmd                                           //GMD IDE
                /retarget.c                                //Printf
            /mdk                                           //MDK IDE
                /retarget.c                                //Printf
            /uart.h                                        //UART functions
            /uart.c                                        //UART functions
    /template                                              //Template
        /gw1ns4c_it.c                                      //Interrupt handler
        /gw1ns4c_it.h                                      //Interrupt handler
        /main.c                                            //Main functions
-------------------------------------------------------------------------------------------------------