-------------------------------------------------------------------------------------------------------------------------------------------------
Gowin_EMPU(GW1NS-4C) SDK Read Me File
-------------------------------------------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------------------------------------------
1. Software
   Version tested:
      Gowin_V1.9.9.01 (64-bit)
      ARM Keil MDK V5.26
      GMD V1.2
-------------------------------------------------------------------------------------------------------------------------------------------------
2. Hardware Targets
   DK-START-GW1NSR4C-QN48G V1.1
   DK-START-GW1NSR4C-QN48P V1.1
   DK-START-GW1NSR4C-MG64P V1.1
-------------------------------------------------------------------------------------------------------------------------------------------------
3. Files
|-- doc                                     --> Documents
|   |-- ReadMe.txt                          --> Read me file
|   |-- ReleaseNote.txt                     --> Release note file
|-- ref_design                              --> Reference design
|   |-- FPGA_RefDesign                      --> Hardware reference design
|   |   |-- DK_START_GW1NSR4C_QN48G_V1.1    --> Hardware reference design on DK-START-GW1NSR4C-QN48G V1.1 board
|   |   |   |-- gowin_empu                  --> Hardware reference design
|   |   |   |   |-- cm3_i2c_demo            --> I2C hardware reference design
|   |   |   |   |-- cm3_spiflash_demo       --> SPI-Flash hardware reference design
|   |   |   |-- ReadMe.txt                  --> Read me file
|   |   |-- DK_START_GW1NSR4C_QN48P_V1.1    --> Hardware reference design on DK-START-GW1NSR4C-QN48P V1.1 board
|   |   |   |-- gowin_empu                  --> Hardware reference design
|   |   |   |   |-- cm3_ahb2_demo           --> AHB2 master hardware reference design
|   |   |   |   |-- cm3_apb2_demo           --> APB2 master hardware reference design
|   |   |   |   |-- cm3_demo                --> Hardware reference design
|   |   |   |   |-- cm3_hyperram_demo       --> HyperRAM hardware reference design
|   |   |   |-- ReadMe.txt                  --> Read me file
|   |   |-- DK_START_GW1NSR4C_MG64P_V1.1    --> Hardware reference design on DK-START-GW1NSR4C-MG64P V1.1 board
|   |   |   |-- gowin_empu                  --> Hardware reference design
|   |   |   |   |-- cm3_psram_demo          --> PSRAM hardware reference design
|   |   |   |-- ReadMe.txt                  --> Read me file
|   |   |--	ReadMe.txt                      --> Hardware reference design read me file
|   |-- MCU_RefDesign                       --> Software reference design
|   |   |-- GMD_RefDesign                   --> Software reference design in GMD IDE
|   |   |   |-- cm3_demo                    --> Software reference design
|   |   |   |-- cm3_freertos                --> Software RTOS FreeRTOS reference design
|   |   |   |-- cm3_rtthread_nano           --> Software RTOS RT-Thread Nano version reference design
|   |   |   |-- cm3_ucos_iii                --> Software RTOS uC/OS-III reference design
|   |   |   |-- ReadMe.txt                  --> Read me file
|   |   |-- MDK_RefDesign                   --> Software reference design in MDK IDE
|   |   |   |-- cm3_demo                    --> Software reference design
|   |   |   |-- cm3_freertos                --> Software RTOS FreeRTOS reference design
|   |   |   |-- cm3_rtthread_nano           --> Software RTOS RT-Thread Nano version reference design
|   |   |   |-- cm3_ucos_iii                --> Software RTOS uC/OS-III reference design
|   |   |   |-- ReadMe.txt                  --> Read me file
|   |   |-- ReadMe.txt                      --> Software reference design read me file
|-- library                                 --> Software programming library
|-- solution                                --> Solution
|   |-- RTOS                                --> RTOS solution
|   |   |- ref_design                       --> Reference design
|   |   |   |- FPGA_RefDesign               --> Hardware reference design
|   |   |   |   |-- gowin_empu              --> Hardware reference design
|   |   |   |   |-- ReadMe.txt              --> Read me file
|   |   |   |- MCU_RefDesign                --> Software reference design
|   |   |   |   |-- GMD_RefDesign           --> Software reference design in GMD IDE
|   |   |   |   |   |-- cm3_freertos        --> Software RTOS FreeRTOS reference design
|   |   |   |   |   |-- cm3_rtthread_nano   --> Software RTOS RT-Thread Nano version reference design
|   |   |   |   |   |-- cm3_ucos_iii        --> Software RTOS uC/OS-III reference design
|   |   |   |   |   |-- ReadMe.txt          --> Read me file
|   |   |   |   |-- MDK_RefDesign           --> Software reference design in MDK IDE
|   |   |   |   |   |-- cm3_freertos        --> Software RTOS FreeRTOS reference design
|   |   |   |   |   |-- cm3_rtthread_nano   --> Software RTOS RT-Thread Nano version reference design
|   |   |   |   |   |-- cm3_ucos_iii        --> Software RTOS uC/OS-III reference design
|   |   |   |   |   |-- ReadMe.txt          --> Read me file
|   |   |   |   |-- ReadMe.txt              --> Software reference design read me file
|   |-- RunInSRAM_FromEmbFlash              --> Running code in SRAM from embedded userflash
|   |   |- ref_design                       --> Reference design
|   |   |   |- FPGA_RefDesign               --> Hardware reference design
|   |   |   |   |-- gowin_empu              --> Hardware reference design
|   |   |   |   |-- ReadMe.txt              --> Read me file
|   |   |   |- MCU_RefDesign                --> Software reference design
|   |   |   |   |-- cm3_demo                --> Software reference design
|   |   |   |   |-- ReadMe.txt              --> Read me file
|   |-- RunInSRAM_FromSIPFlash              --> Running code in SRAM from SIP SPI-Flash
|   |   |- ref_design                       --> Reference design
|   |   |   |- FPGA_RefDesign               --> Hardware reference design
|   |   |   |   |-- gowin_empu              --> Hardware reference design
|   |   |   |   |-- ReadMe.txt              --> Read me file
|   |   |   |- MCU_RefDesign                --> Software reference design
|   |   |   |   |-- cm3_demo                --> Software reference design
|   |   |   |   |-- ReadMe.txt              --> Read me file
|   |   |- tool                             --> Tool
|   |   |   |- bin2hex                      --> Tool bin2hex
-------------------------------------------------------------------------------------------------------------------------------------------------
4. Manuals
   * Download from http://www.gowinsemi.com.cn/
       - Gowin_EMPU(GW1NS-4C)快速设计参考手册.pdf
       - Gowin_EMPU(GW1NS-4C)软件编程参考手册.pdf
       - Gowin_EMPU(GW1NS-4C)硬件设计参考手册.pdf
       - Gowin_EMPU(GW1NS-4C) IDE软件参考手册.pdf
       - Gowin_EMPU(GW1NS-4C)串口调试参考手册.pdf
       - Gowin_EMPU(GW1NS-4C)解决方案参考手册.pdf
   * Download from https://www.gowinsemi.com/en/
       - Gowin_EMPU(GW1NS-4C) Quick Design Reference Manual.pdf
       - Gowin_EMPU(GW1NS-4C) Software Programming Reference Manual.pdf
       - Gowin_EMPU(GW1NS-4C) Hardware Design Reference Manual.pdf
       - Gowin_EMPU(GW1NS-4C) IDE Software Reference Manual.pdf
       - Gowin_EMPU(GW1NS-4C) Serial Debugging Reference Manual.pdf
       - Gowin_EMPU(GW1NS-4C) Solution Reference Manual.pdf
-------------------------------------------------------------------------------------------------------------------------------------------------